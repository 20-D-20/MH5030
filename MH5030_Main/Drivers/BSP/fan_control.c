/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fan_control.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct {
    uint32_t pulse_count;            // �������
    uint32_t last_pulse_count;       // �ϴ��������
    uint16_t rpm;                    // ת�� (RPM)
//  uint16_t target_rpm;             // Ŀ��ת��
    uint8_t  duty_cycle;             // PWMռ�ձ� (0-100)
    uint8_t  enable;                 // ����ʹ��״̬
    uint8_t  fault;                  // ���ϱ�־
    uint32_t last_update_tick;       // �ϴθ���ʱ��
} FanControl_t;

/* Private define ------------------------------------------------------------*/
#define FAN_PWM_FREQ        10000    // PWMƵ�� 10kHz
#define FAN_PWM_PERIOD      1000     // PWM����ֵ (72MHz/10kHz)
#define FAN_PULSE_PER_REV   2        // ÿת������
#define FAN_UPDATE_PERIOD   1000     // ת�ٸ������� (ms)
#define FAN_MIN_DUTY        0        // ��Сռ�ձ� (%)
#define FAN_MAX_DUTY        100      // ���ռ�ձ� (%)
#define FAN_FAULT_THRESHOLD 100      // ���ϼ����ֵ (RPM)
                                     
/* Private variables ---------------------------------------------------------*/
static FanControl_t fan = {0};

/* Private function prototypes -----------------------------------------------*/
static void Fan_PWM_Init(void);
static void Fan_Capture_Init(void);
static uint16_t Fan_CalculateRPM(uint32_t pulse_count, uint32_t period_ms);

/* Public variables ---------------------------------------------------------*/
FanStatus_t g_stFanStatus = {0};   	 /* ��¼���ȵ�����״̬ */

/* Public functions ----------------------------------------------------------*/
/**
  * @brief  ��ʼ�����ȿ���
  * @param  None
  * @retval None
  */
void Fan_Init(void)
{
    // ��ʼ��PWM���
    Fan_PWM_Init();
    
    // ��ʼ��ת�ٲ���
    Fan_Capture_Init();
    
    // ����Ĭ�ϲ���
    fan.duty_cycle = 85;      // Ĭ��85%ռ�ձ�
    fan.enable = 0;           // Ĭ�Ϲر�
    
    // �رշ���
    Fan_Stop();
}


/**
  * @brief  PWM��ʼ�� (TIM3_CH1 - PB4)
  * @param  None
  * @retval None
  */
static void Fan_PWM_Init(void)
{
    // ����PWM
    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
}

/**
  * @brief  ת�ٲ����ʼ�� 
  * @param  None
  * @retval None
  */
static void Fan_Capture_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // ʹ��ʱ��
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    // ����PA15Ϊ���� (FFG�ź�)
    GPIO_InitStruct.Pin = FFG_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;  // �������ж�
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(FFG_GPIO_Port, &GPIO_InitStruct);
    
    // �����ⲿ�ж�
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/**
  * @brief  ��������
  * @param  None
  * @retval None
  */
void Fan_Start(void)
{
    if (!fan.enable) 
    {
        // �򿪷��ȵ�Դ (FCTR = 0)
        HAL_GPIO_WritePin(FCTR_GPIO_Port, FCTR_Pin, GPIO_PIN_RESET);
      
        // ����Ŀ��ռ�ձ�
        Fan_SetDutyCycle(fan.duty_cycle);
        fan.enable = 1;
        fan.pulse_count = 0;
        fan.last_pulse_count = 0;
        fan.last_update_tick = HAL_GetTick();
    }
}

/**
  * @brief  ֹͣ����
  * @param  None
  * @retval None
  */
void Fan_Stop(void)
{
    // �رշ��ȵ�Դ (FCTR = 1)
    HAL_GPIO_WritePin(FCTR_GPIO_Port, FCTR_Pin, GPIO_PIN_SET);
    
    // PWM���0
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
    
    fan.enable = 0;
    fan.rpm = 0;
    fan.pulse_count = 0;
}

/**
  * @brief  ����PWMռ�ձ�
  * @param  duty: ռ�ձ� (0-100)
  * @retval None
  */
void Fan_SetDutyCycle(uint8_t duty)
{
    if (duty > FAN_MAX_DUTY) 
    {
        duty = FAN_MAX_DUTY;
    }
    
    fan.duty_cycle = duty;
    
    // ����Ƚ�ֵ
     uint32_t compare = (FAN_PWM_PERIOD * duty) / 100;
    
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, compare);
}

///**
//  * @brief  ����Ŀ��ת��
//  * @param  rpm: Ŀ��ת��
//  * @retval None
//  */
//void Fan_SetTargetRPM(uint16_t rpm)
//{
//    fan.target_rpm = rpm;
//}

/**
  * @brief  ��ȡ��ǰת��
  * @param  None
  * @retval ת��(RPM)
  */
uint16_t Fan_GetRPM(void)
{
    return fan.rpm;
}

/**
  * @brief  ��ȡռ�ձ�
  * @param  None
  * @retval ռ�ձ�(0-100)
  */
uint8_t Fan_GetDutyCycle(void)
{
    return fan.duty_cycle;
}

/**
  * @brief  ��ȡ����״̬
  * @param  None
  * @retval 0:����, 1:����
  */
uint8_t Fan_GetFaultStatus(void)
{
    return fan.fault;
}

/**
  * @brief  ����ת��
  * @param  pulse_count: ������
  * @param  period_ms: ʱ������(ms)
  * @retval ת��(RPM)
  */
static uint16_t Fan_CalculateRPM(uint32_t pulse_count, uint32_t period_ms)
{
    if (period_ms == 0) return 0;
    
    // RPM = (������ * 60000) / (ÿת������ * ʱ������ms)
    uint32_t rpm = (pulse_count * 60000) / (FAN_PULSE_PER_REV * period_ms);
    
    return (uint16_t)rpm;
}

/**
  * @brief  ���·���״̬ (����ѭ���е���)
  * @param  None
  * @retval None
  */
void Fan_Update(void)
{
    uint32_t current_tick = HAL_GetTick();
    
    // ÿ�����һ��ת��
    if (current_tick - fan.last_update_tick >= FAN_UPDATE_PERIOD) 
    {
        uint32_t period = current_tick - fan.last_update_tick;
        uint32_t pulses = fan.pulse_count - fan.last_pulse_count;
        
        // ��ֵ�˲����ת��
        fan.rpm = Filter_Median(Fan_CalculateRPM(pulses, period));

        // ���ϼ��
        if (fan.enable && fan.rpm < FAN_FAULT_THRESHOLD) 
        {
            fan.fault = 1;
			g_stFanStatus.fault_consec ++;							/* ���ƴ���ͳ�� */
        } 
        else 
        {
            fan.fault = 0;
			g_stFanStatus.fault_consec = 0;							/* �������� */
        }       
        // ���¼���
        fan.last_pulse_count = fan.pulse_count;
        fan.last_update_tick = current_tick;
    }
}

/* Interrupt Handlers --------------------------------------------------------*/

/**
  * @brief  �ⲿ�жϻص� (ת������)
  * @param  GPIO_Pin: ���ź�
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == FFG_Pin) 
    {
        // FFG�������
        fan.pulse_count++;
    }
}

/**
  * @brief  EXTI1�жϷ�����
  * @param  None
  * @retval None
  */
void EXTI15_10_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(FFG_Pin);
}

