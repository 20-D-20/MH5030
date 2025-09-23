/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "delay.h"
#include "I2c.h"
#include "tps02r.h"
#include <stdio.h>
#include "ssd1305.h"
#include "FM24CXX.h"
#include "onewire.h"
#include "ds18b20.h"
#include "MAX6675.h"
#include "ntc.h"
#include "pid.h"
#include "fan_control.h"
#include "iwdg.h"
#include "key.h"
#include "queue.h"
#include "semphr.h"
#include <stdlib.h>
#include "pid_manager.h"
#include "ui_manager.h"
#include "key_manager.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

// FreeRTOS对象
QueueHandle_t UI_Queue = NULL;
SemaphoreHandle_t Data_Mutex = NULL;

// 任务句柄
osThreadId Key_TaskHandle;
osThreadId UI_TaskHandle;

/* USER CODE END Variables */
osThreadId HeaterCtrl_TaskHandle;
osThreadId Fan_TaskHandle;
osThreadId TCouple_TaskHandle;
osThreadId DS18B20_TaskHandle;
osThreadId Wdg_TaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void heaterctrl_task(void const * argument);
void fan_task(void const * argument);
void tcouple_task(void const * argument);
void ds18b20_task(void const * argument);
void wdg_task(void const * argument);
void key_task(void const * argument);
void ui_task(void const * argument);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */


/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) 
{
  /* USER CODE BEGIN Init */

   /* 创建消息队列 */
   UI_Queue = xQueueCreate(10, sizeof(UIMessage_t));
   
   /* 创建互斥量 */
   Data_Mutex = xSemaphoreCreateMutex();

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of HeaterCtrl_Task */
  osThreadDef(HeaterCtrl_Task, heaterctrl_task, osPriorityAboveNormal, 0, 256);
  HeaterCtrl_TaskHandle = osThreadCreate(osThread(HeaterCtrl_Task), NULL);

  /* definition and creation of Fan_Task */
  osThreadDef(Fan_Task, fan_task, osPriorityNormal, 0, 48);
  Fan_TaskHandle = osThreadCreate(osThread(Fan_Task), NULL);

//  /* definition and creation of TCouple_Task */
//  osThreadDef(TCouple_Task, tcouple_task, osPriorityNormal, 0, 64);
//  TCouple_TaskHandle = osThreadCreate(osThread(TCouple_Task), NULL);

//  /* definition and creation of DS18B20_Task */
//  osThreadDef(DS18B20_Task, ds18b20_task, osPriorityNormal, 0, 64);
//  DS18B20_TaskHandle = osThreadCreate(osThread(DS18B20_Task), NULL);

//  /* definition and creation of Wdg_Task */
//  osThreadDef(Wdg_Task, wdg_task, osPriorityRealtime , 0, 32);
//  Wdg_TaskHandle = osThreadCreate(osThread(Wdg_Task), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  
   /* 创建Key任务 */
  osThreadDef(Key_Task, key_task, osPriorityHigh, 0, 64);
  Key_TaskHandle = osThreadCreate(osThread(Key_Task), NULL);
  
  /* 创建UI任务 */
  osThreadDef(UI_Task, ui_task, osPriorityNormal, 0, 256);
  UI_TaskHandle = osThreadCreate(osThread(UI_Task), NULL);
  
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void heaterctrl_task(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
    float pt100_front = 0;                                              /* 记录前枪管温度 */
    float pt100_rear = 0;                                               /* 记录腔体温度 */
    
    /* 硬件初始化 */
    tps02r_iic_init(&g_stTps02r_IICManger);                            /* 初始化tps02r */
    tps02r_cfg_init();
    FM24CXX_iic_init(&fm24cxx_iicmanager);                             /* 初始化EEPROM */
    
    /* PID系统初始化 */
    pid_front_init(&g_stPidFront, &g_stPidFrontAuto, &g_stFilterFront);/* 初始化前枪管PID参数 */
    pid_rear_init(&g_stPidRear, &g_stPidRearAuto, &g_stFilterRear);    /* 初始化腔体PID参数 */
    
    /* 参数管理器初始化 */
    Init_PID_Manager();
    
    /* 检查EEPROM */
    if (FM_Check() != 0)
    {
        g_system_status.error_code = 1;                                /* EEPROM错误 */
    }
    
    /* Infinite loop */
    for(;;)
    {
        /* 读取前枪管温度 */
        taskENTER_CRITICAL() ;
        if (tps02r_get_temp(TPS02R_CHAN1, &pt100_front) == TPS02R_FUN_OK)          
        {
            /* 滤波处理 */
            g_stPidFront.Pv = combined_filter(&g_stFilterFront, pt100_front);
            g_system_status.front_temp_pv = g_stPidFront.Pv;
        }
        else
        {
            g_system_status.temp_error |= 0x01;                        /* 前枪管温度读取错误 */
        }
        
        /* 读取腔体温度 */
        if (tps02r_get_temp(TPS02R_CHAN2, &pt100_rear) == TPS02R_FUN_OK)          
        {
            /* 滤波处理 */
            g_stPidRear.Pv = combined_filter(&g_stFilterRear, pt100_rear);
            g_system_status.rear_temp_pv = g_stPidRear.Pv;
        }
        
        else
        {
            g_system_status.temp_error |= 0x02;                        /* 腔体温度读取错误 */
        }
        taskEXIT_CRITICAL();
        /* 更新设定值 */
        g_stPidFront.Sv = g_system_status.front_temp_sv;
        g_stPidRear.Sv = g_system_status.rear_temp_sv;
        
        /* 检查是否需要切换参数组 */
        if (g_system_status.mode != PID_MODE_AUTOTUNE)
        {
            Check_And_Switch_Group();
        }
        
        /* 处理自整定请求 */
        if (g_system_status.autotune_request)
        {
            /* 设置自整定模式 */
            g_stPidFrontAuto.tuneEnable = 1;
            g_stPidRearAuto.tuneEnable = 1;
            g_system_status.autotune_request = 0;
        }
        
        /* 根据运行模式执行不同逻辑 */
        if (g_system_status.mode == PID_MODE_AUTOTUNE)
        {
            /* 自整定模式：两路同时进行 */
            RelayFeedbackAutoTuning(&g_stPidFront, &g_stPidFrontAuto);
            RelayFeedbackAutoTuning(&g_stPidRear, &g_stPidRearAuto);
            
            /* 检查自整定是否完成 */
            Check_Autotune_Complete();
        }
        else if (g_system_status.mode == PID_MODE_RUN)
        {
            /* 运行模式：执行PID控制 */
            PID_Calc(&g_stPidFront, &g_stPidFrontAuto);
            PID_Calc(&g_stPidRear, &g_stPidRearAuto);
        }
        else
        {
            /* 停止模式：输出置零 */
            g_stPidFront.OUT = 0;
            g_stPidRear.OUT = 0;
        }
        
        /* 更新PWM输出 */
        if (g_stPidFront.OUT > 0)
        {
            TIM4->CCR4 = g_stPidFront.OUT - 1;
        }
        else
        {
            TIM4->CCR4 = 0;
        }
        
        if (g_stPidRear.OUT > 0)
        {
            TIM1->CCR2 = g_stPidRear.OUT - 1;
        }
        else
        {
            TIM1->CCR2 = 0;
        }
        
        osDelay(200);
    }
  /* USER CODE END StartDefaultTask */
}

/**
 * @brief  Key任务 - 处理按键和系统逻辑
 */
void key_task(void const * argument)
{
    uint8_t key = 0;
    uint32_t startup_timer = osKernelSysTick();
    
    /* 初始化按键管理器 */
    Key_Manager_Init();
    
    for(;;)
    {
        /* 自动切换启动页面 */
        if(g_current_page_id == PAGE_STARTUP && 
           (osKernelSysTick() - startup_timer) > 2000)
        {
            xSemaphoreTake(Data_Mutex, portMAX_DELAY);
            g_current_page_id = PAGE_TEMP_DISPLAY;
            xSemaphoreGive(Data_Mutex);
            
            Send_UI_Message(MSG_PAGE_CHANGE, PAGE_TEMP_DISPLAY, 0, 1);
        }
        
        /* 按键扫描 */
        key = key_scan(0);
        
        if(key != 0)
        {
            xSemaphoreTake(Data_Mutex, portMAX_DELAY);
            
            /* 根据当前模式处理按键 */
            switch (g_current_mode) 
           {
            case MODE_BROWSE:
                Process_Browse_Mode_Key(key);
                break;
            
            case MODE_EDIT:
                Process_Edit_Mode_Key(key);
                break;
            
            case MODE_AUTOTUNE:
                Process_Autotune_Key(key);
                break;
            
            case MODE_SELECT:
                Process_Gun_Select_Mode_Key(key);
                break;

            default:
                /* 未知模式：可选择忽略、记录日志或做安全处理 */
                break;
            }

            
            xSemaphoreGive(Data_Mutex);
        }
        
        /* 自整定进度更新 */
        if(g_current_mode == MODE_AUTOTUNE)
        {
            Update_System_Status();
            
            /* 检查是否完成 */
            if(g_system_status.autotune_complete)
            {
                g_current_mode = MODE_BROWSE;
                g_key_counter.autotune_triggered = 0;
                Reset_Key_Counter();
                Send_UI_Message(MSG_PAGE_CHANGE, PAGE_SMART_CONTROL, 0, 1);
            }
            else
            {
                /* 发送进度更新消息 */
                Send_UI_Message(MSG_AUTOTUNE_PROGRESS, PAGE_SMART_CONTROL, 0, 0);
            }
        }
        
        osDelay(20);
    }
}

/* UI任务 */
void ui_task(void const * argument)
{
    UIMessage_t msg;
    uint32_t realtime_counter = 0;
    bool need_temp_update = false;
    
    /* 初始化UI管理器 */
    UI_Manager_Init();
    
    /* 显示启动页面 */
    Display_Startup_Page();
    
    for(;;)
    {
        /* 检查消息队列 */
        if(xQueueReceive(UI_Queue, &msg, 0) == pdTRUE)
        {
            switch(msg.msg_type)
            {
                case MSG_PAGE_CHANGE:
                    Display_Page(msg.page_id);
                    break;
                    
                case MSG_VALUE_UPDATE:
                    Update_Value_Display(msg.page_id, msg.new_value, 
                                       g_current_mode == MODE_EDIT);
                    break;
                    
                case MSG_MODE_CHANGE:
                    if(msg.page_id == PAGE_SMART_CONTROL ||
                       msg.page_id == PAGE_GUN_SELECT)
                    {
                        Display_Page(msg.page_id);
                    }
                    else
                    {
                        Update_Value_Display(msg.page_id, 
                            g_pages[msg.page_id].current_value,
                            g_current_mode == MODE_EDIT);
                    }
                    break;
                    
                case MSG_AUTOTUNE_PROGRESS:
                    if(g_current_page_id == PAGE_SMART_CONTROL && 
                       g_current_mode == MODE_AUTOTUNE)
                    {
                        Display_Autotune_Progress_Page();
                    }
                    break;
            }
        }
        
        /* 实时数据更新 */
        realtime_counter++;
        if(realtime_counter >= 25)  /* 500ms */
        {
            realtime_counter = 0;
            
            xSemaphoreTake(Data_Mutex, portMAX_DELAY);
            
            /* 根据不同页面更新数据 */
            if(g_current_page_id == PAGE_TEMP_DISPLAY && 
               g_current_mode == MODE_BROWSE)
            {
//                /* 用于测试的温度获取 */
//                get_test_temperatures(&g_gun_temp_measured, &g_cavity_temp_measured);
                
                /* 温度显示页面 */
                need_temp_update = true;
            }
            else if(g_current_page_id == PAGE_DIOXIN_DISPLAY && 
                    g_current_mode == MODE_BROWSE)
            {
                /* 二噁英温度页面 */
                get_dioxin_temperatures(&g_dioxin_temp1, &g_dioxin_temp2);
                need_temp_update = true;
            }
            
            xSemaphoreGive(Data_Mutex);
            
            /* 刷新显示 */
            if(need_temp_update)
            {
                if(g_current_page_id == PAGE_TEMP_DISPLAY)
                {
                    /* 用于测试的温度显示*/
//                    Show_Word_U_16x32(8,24,g_gun_temp_measured,3,0,false);
//                    Show_Word_U_16x32(70,24,g_cavity_temp_measured,3,0,false);

                    /* 实际测量的温度显示 */
                    Show_Word_U_16x32(8,24,(u16)g_system_status.front_temp_pv,3,0,false);
                    Show_Word_U_16x32(70,24,(u16)g_system_status.rear_temp_pv,3,0,false);
                }
                else if(g_current_page_id == PAGE_DIOXIN_DISPLAY)
                {
                    Show_Word_U(14, 32, g_dioxin_temp1, 3, 0, false);
                    Show_Word_U(78, 32, g_dioxin_temp2, 3, 0, false);
                }
                need_temp_update = false;
            }
        }
        
        osDelay(20);
    }
}


/* USER CODE BEGIN Header_fan_task */
/**
* @brief Function implementing the Fan_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_fan_task */
void fan_task(void const * argument)
{
  /* USER CODE BEGIN fan_task */
    Fan_Init();                                                  /* Fan initialization */
    Fan_Start();                                                 /* Start fan, default duty cycle 85% */
    Filter_Init_All();                                           /* 初始化滤波器 */
  /* Infinite loop */
    for(;;)
    {
        Fan_Update();                                            /* Update fan status */
        if(g_stFanStatus.fault_consec >= 10)                     /* Continuous fault count >= 10? */
        { 
            HAL_GPIO_WritePin(GPIOB, BELL_Pin, GPIO_PIN_SET);    /* Trigger alarm */
            g_stFanStatus.fault = 1;                             /* Set fault flag */
        }else
        {
             HAL_GPIO_WritePin(GPIOB, BELL_Pin, GPIO_PIN_RESET);
             g_stFanStatus.fault = 0; 
        }
        osDelay(200);
    }
  /* USER CODE END fan_task */
}

/* USER CODE BEGIN Header_tcouple_task */
/**
* @brief Function implementing the TCouple_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_tcouple_task */
void tcouple_task(void const * argument)
{
  /* USER CODE BEGIN tcouple_task */
  MAX6675_Setup();                                               /* Initialize Max6675 */
  float temp_max6675;                                            /* Record temperature value */
  MAX6675_Error_e result ;                                       /* Record read status */
  /* Infinite loop */                  
  for(;;)
  {
     /* Check if thermocouple channel is ready */
    if(MAX6675_IsChannelReady(&g_stMax6675, MAX6675_CH1)) 
    {
         /* Read thermocouple temperature */
        result = MAX6675_ReadTemperatureFiltered(&g_stMax6675, MAX6675_CH1, &temp_max6675);
    }
    
    if(result != MAX6675_OK)
    {
        if(g_stMax6675.total_errors >= 3)                        /* Continuous read errors >= 3 */
        {
            //Error handling
        }
    }
    else 
    {
        g_stMax6675.total_errors = 0 ;                           /* Reset error count */
    }
    osDelay(300);
  }
  /* USER CODE END tcouple_task */
}

/* USER CODE BEGIN Header_ds18b20_task */
/**
* @brief Function implementing the DS18B20_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ds18b20_task */
void ds18b20_task(void const * argument)
{
  /* USER CODE BEGIN ds18b20_task */
  OneWire_Init(&g_stOneWire_1,DS18B20_1_GPIO_Port, DS18B20_1_Pin); 
  bool ok = true;
  /* Infinite loop */
  for(;;)
  {
    ok = Ds18b20_ManualConvert(&g_stDs18b20_1,&g_stOneWire_1);
    if(ok != true)
    {
        if(g_stDs18b20_1.error_count >= 3)
        {
            //Error handling
        }
    }
    else 
    {
        g_stDs18b20_1.error_count = 0;                 /* Reset error count */
        g_stDs18b20_1.DataIsValid = true;              /* Data valid */
    }
    osDelay(200);
  }
  /* USER CODE END ds18b20_task */
}

/* USER CODE BEGIN Header_wdg_task */
/**
* @brief Function implementing the Wdg_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_wdg_task */
void wdg_task(void const * argument)
{
  /* USER CODE BEGIN wdg_task */
    MX_IWDG_Init();                                    /* Initialize watchdog */
  /* Infinite loop */
  for(;;)
  {
                                                       /* Hardware watchdog + LED indicator */
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
  	HAL_IWDG_Refresh(&hiwdg);
    osDelay(500);
  }
  /* USER CODE END wdg_task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */


/* USER CODE END Application */


