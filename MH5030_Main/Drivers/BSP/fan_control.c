/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fan_control.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct 
{
    uint32_t pulse_count;            /* 脉冲计数 */
    uint32_t last_pulse_count;       /* 上次脉冲计数 */
    uint16_t rpm;                    /* 转速 (RPM) */
    uint8_t  duty_cycle;             /* PWM占空比 (0-100) */
    uint8_t  enable;                 /* 风扇使能状态 */
    uint8_t  fault;                  /* 故障标志 */
    uint32_t last_update_tick;       /* 上次更新时间戳 */
} FanControl_t;

/* Private define ------------------------------------------------------------*/
#define FAN_PWM_FREQ        10000    /* PWM频率 10kHz */
#define FAN_PWM_PERIOD      1000     /* PWM周期值 (72MHz/10kHz) */
#define FAN_PULSE_PER_REV   2        /* 每转脉冲数 */
#define FAN_UPDATE_PERIOD   1000     /* 转速更新周期 (ms) */
#define FAN_MIN_DUTY        0        /* 最小占空比 (%) */
#define FAN_MAX_DUTY        100      /* 最大占空比 (%) */
#define FAN_FAULT_THRESHOLD 100      /* 故障检测阈值 (RPM) */

/* Private variables ---------------------------------------------------------*/
static FanControl_t fan = {0};

/* Private function prototypes -----------------------------------------------*/
static void Fan_PWM_Init(void);
static void Fan_Capture_Init(void);
static uint16_t Fan_CalculateRPM(uint32_t pulse_count, uint32_t period_ms);

/* Public variables ---------------------------------------------------------*/
FanStatus_t g_stFanStatus = {0};     /* 记录风扇运行状态 */

/* Public functions ----------------------------------------------------------*/
/**
  * @brief  初始化风扇控制器
  * @param  None
  * @retval None
  */
void Fan_Init(void)
{
    /* 初始化PWM输出 */                                                          /* TIM3_CH1 - PB4 */
    Fan_PWM_Init();
    
    /* 初始化转速检测 */                                                         /* FFG信号输入 */
    Fan_Capture_Init();
    
    /* 设置默认参数 */                                                           /* 默认配置 */
    fan.duty_cycle = 95;                                                         /* 默认85%占空比 */
    fan.enable = 0;                                                              /* 默认关闭 */
    
    /* 关闭风扇 */                                                               /* 初始状态 */
    Fan_Stop();
}

/**
  * @brief  PWM初始化 (TIM3_CH1 - PB4)
  * @param  None
  * @retval None
  */
static void Fan_PWM_Init(void)
{
    /* 启动PWM输出 */                                                            /* TIM3通道1 */
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}

/**
  * @brief  转速检测初始化
  * @param  None
  * @retval None
  */
static void Fan_Capture_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* 使能时钟 */                                                               /* GPIOA时钟 */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    /* 配置PA15为输入 (FFG信号) */                                               /* 上升沿触发 */
    GPIO_InitStruct.Pin = FFG_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(FFG_GPIO_Port, &GPIO_InitStruct);
    
    /* 配置外部中断 */                                                           /* 优先级设置 */
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/**
  * @brief  启动风扇
  * @param  None
  * @retval None
  */
void Fan_Start(void)
{
    if (!fan.enable) 
    {
        /* 开启风扇电源 (FCTR = 0) */                                              /* 控制信号低电平有效 */
        HAL_GPIO_WritePin(FCTR_GPIO_Port, FCTR_Pin, GPIO_PIN_RESET);
      
        /* 设置目标占空比 */                                                        /* 应用设定值 */
        Fan_SetDutyCycle(fan.duty_cycle);
        fan.enable = 1;
        fan.pulse_count = 0;
        fan.last_pulse_count = 0;
        fan.last_update_tick = HAL_GetTick();
    }
}

/**
  * @brief  停止风扇
  * @param  None
  * @retval None
  */
void Fan_Stop(void)
{
    /* 关闭风扇电源 (FCTR = 1) */                                                  /* 控制信号高电平关闭 */
    HAL_GPIO_WritePin(FCTR_GPIO_Port, FCTR_Pin, GPIO_PIN_SET);
    
    /* PWM输出置零 */                                                            /* 停止输出 */
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
    
    fan.enable = 0;
    fan.rpm = 0;
    fan.pulse_count = 0;
}

/**
  * @brief  设置PWM占空比
  * @param  duty: 占空比 (0-100)
  * @retval None
  */
void Fan_SetDutyCycle(uint8_t duty)
{
    if (duty > FAN_MAX_DUTY) 
    {
        duty = FAN_MAX_DUTY;
    }
    
    fan.duty_cycle = duty;
    
    /* 计算比较值 */                                                              /* 根据占空比计算 */
    uint32_t compare = (FAN_PWM_PERIOD * duty) / 100;
    
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, compare);
}

/**
  * @brief  获取当前转速
  * @param  None
  * @retval 转速(RPM)
  */
uint16_t Fan_GetRPM(void)
{
    return fan.rpm;
}

/**
  * @brief  获取占空比
  * @param  None
  * @retval 占空比(0-100)
  */
uint8_t Fan_GetDutyCycle(void)
{
    return fan.duty_cycle;
}

/**
  * @brief  获取风扇状态
  * @param  None
  * @retval 0:正常, 1:故障
  */
uint8_t Fan_GetFaultStatus(void)
{
    return fan.fault;
}

/**
  * @brief  计算转速
  * @param  pulse_count: 脉冲计数
  * @param  period_ms: 时间周期(ms)
  * @retval 转速(RPM)
  */
static uint16_t Fan_CalculateRPM(uint32_t pulse_count, uint32_t period_ms)
{
    if (period_ms == 0) return 0;
    
    /* RPM = (脉冲数 * 60000) / (每转脉冲数 * 时间周期ms) */                      /* 转速计算公式 */
    uint32_t rpm = (pulse_count * 60000) / (FAN_PULSE_PER_REV * period_ms);
    
    return (uint16_t)rpm;
}

/**
  * @brief  更新风扇状态 (需要在循环中调用)
  * @param  None
  * @retval None
  */
void Fan_Update(void)
{
    uint32_t current_tick = HAL_GetTick();
    
    /* 每隔固定时间更新转速 */                                                     /* 周期更新 */
    if (current_tick - fan.last_update_tick >= FAN_UPDATE_PERIOD) 
    {
        uint32_t period = current_tick - fan.last_update_tick;
        uint32_t pulses = fan.pulse_count - fan.last_pulse_count;
        
        /* 计算并滤波转速值 */                                                    /* 中值滤波 */
        fan.rpm = Filter_Median(Fan_CalculateRPM(pulses, period));

        /* 故障检测 */                                                            /* 转速过低判断 */
        if (fan.enable && fan.rpm < FAN_FAULT_THRESHOLD) 
        {
            fan.fault = 1;
            g_stFanStatus.fault_consec++;                                         /* 连续故障计数 */
        } 
        else 
        {
            fan.fault = 0;
            g_stFanStatus.fault_consec = 0;                                       /* 清零计数 */
        }       
        
        /* 更新计数 */                                                            /* 准备下次计算 */
        fan.last_pulse_count = fan.pulse_count;
        fan.last_update_tick = current_tick;
    }
}

/* Interrupt Handlers --------------------------------------------------------*/

/**
  * @brief  外部中断回调函数 (转速脉冲计数)
  * @param  GPIO_Pin: 引脚编号
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == FFG_Pin) 
    {
        /* FFG脉冲计数 */                                                         /* 每次上升沿计数 */
        fan.pulse_count++;
    }
}

/**
  * @brief  EXTI1中断服务函数
  * @param  None
  * @retval None
  */
void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(FFG_Pin);
}

