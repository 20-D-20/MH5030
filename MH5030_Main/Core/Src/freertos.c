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

// FreeRTOS����
QueueHandle_t UI_Queue = NULL;
SemaphoreHandle_t Data_Mutex = NULL;

// ������
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
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* ��ʼ��ҳ������ */
   init_page_data();
   Init_PID_Manager();  // ��ʼ��PID������
   Key_Manager_Init();  // ��ʼ������������
   /* ������Ϣ���� */
   UI_Queue = xQueueCreate(10, sizeof(UIMessage_t));
   
   /* ���������� */
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

//  /* Create the thread(s) */
//  /* definition and creation of HeaterCtrl_Task */
//  osThreadDef(HeaterCtrl_Task, heaterctrl_task, osPriorityAboveNormal, 0, 128);
//  HeaterCtrl_TaskHandle = osThreadCreate(osThread(HeaterCtrl_Task), NULL);

//  /* definition and creation of Fan_Task */
//  osThreadDef(Fan_Task, fan_task, osPriorityNormal, 0, 48);
//  Fan_TaskHandle = osThreadCreate(osThread(Fan_Task), NULL);

//  /* definition and creation of TCouple_Task */
//  osThreadDef(TCouple_Task, tcouple_task, osPriorityNormal, 0, 64);
//  TCouple_TaskHandle = osThreadCreate(osThread(TCouple_Task), NULL);

//  /* definition and creation of DS18B20_Task */
//  osThreadDef(DS18B20_Task, ds18b20_task, osPriorityNormal, 0, 64);
//  DS18B20_TaskHandle = osThreadCreate(osThread(DS18B20_Task), NULL);

  /* definition and creation of Wdg_Task */
  osThreadDef(Wdg_Task, wdg_task, osPriorityRealtime , 0, 32);
  Wdg_TaskHandle = osThreadCreate(osThread(Wdg_Task), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  
   /* ����Key���� */
  osThreadDef(Key_Task, key_task, osPriorityHigh, 0, 64);
  Key_TaskHandle = osThreadCreate(osThread(Key_Task), NULL);
  
  /* ����UI���� */
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
    float pt100_front = 0;                                              /* ��¼ǰǹ���¶� */
    float pt100_rear = 0;                                               /* ��¼ǻ���¶� */
    uint8_t init_flag = 0;                                              /* ��ʼ����־ */
    
    /* Ӳ����ʼ�� */
    tps02r_iic_init(&g_stTps02r_IICManger);                            /* ��ʼ��tps02r */
    FM24CXX_iic_init(&fm24cxx_iicmanager);                             /* ��ʼ��EEPROM */
    
    /* PIDϵͳ��ʼ�� */
    pid_front_init(&g_stPidFront, &g_stPidFrontAuto, &g_stFilterFront);/* ��ʼ��ǰǹ��PID���� */
    pid_rear_init(&g_stPidRear, &g_stPidRearAuto, &g_stFilterRear);    /* ��ʼ��ǻ��PID���� */
    
    /* ������������ʼ�� */
    Init_PID_Manager();
    
    /* ���EEPROM */
    if (FM_Check() != 0)
    {
        g_system_status.error_code = 1;                                /* EEPROM���� */
    }
    
    /* Infinite loop */
    for(;;)
    {
        /* ��ȡǰǹ���¶� */
        if (tps02r_get_temp(TPS02R_CHAN1, &pt100_front) == TPS02R_FUN_OK)          
        {
            /* �˲����� */
            g_stPidFront.Pv = combined_filter(&g_stFilterFront, pt100_front);
            g_system_status.front_temp_pv = g_stPidFront.Pv;
        }
        else
        {
            g_system_status.temp_error |= 0x01;                        /* ǰǹ���¶ȶ�ȡ���� */
        }
        
        /* ��ȡǻ���¶� */
        if (tps02r_get_temp(TPS02R_CHAN2, &pt100_rear) == TPS02R_FUN_OK)          
        {
            /* �˲����� */
            g_stPidRear.Pv = combined_filter(&g_stFilterRear, pt100_rear);
            g_system_status.rear_temp_pv = g_stPidRear.Pv;
        }
        else
        {
            g_system_status.temp_error |= 0x02;                        /* ǻ���¶ȶ�ȡ���� */
        }
        
        /* �����趨ֵ */
        g_stPidFront.Sv = g_system_status.front_temp_sv;
        g_stPidRear.Sv = g_system_status.rear_temp_sv;
        
        /* ����Ƿ���Ҫ�л������� */
        if (g_system_status.mode != MODE_AUTOTUNE)
        {
            Check_And_Switch_Group();
        }
        
        /* �������������� */
        if (g_system_status.autotune_request)
        {
            /* ����������ģʽ */
            g_stPidFrontAuto.tuneEnable = 1;
            g_stPidRearAuto.tuneEnable = 1;
            g_system_status.autotune_request = 0;
        }
        
        /* ��������ģʽִ�в�ͬ�߼� */
        if (g_system_status.mode == MODE_AUTOTUNE)
        {
            /* ������ģʽ����·ͬʱ���� */
            RelayFeedbackAutoTuning(&g_stPidFront, &g_stPidFrontAuto);
            RelayFeedbackAutoTuning(&g_stPidRear, &g_stPidRearAuto);
            
            /* ����������Ƿ���� */
            Check_Autotune_Complete();
        }
        else if (g_system_status.mode == MODE_RUN)
        {
            /* ����ģʽ��ִ��PID���� */
            PID_Calc(&g_stPidFront, &g_stPidFrontAuto);
            PID_Calc(&g_stPidRear, &g_stPidRearAuto);
        }
        else
        {
            /* ֹͣģʽ��������� */
            g_stPidFront.OUT = 0;
            g_stPidRear.OUT = 0;
        }
        
        /* ����PWM��� */
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
 * @brief  Key���� - ��������ϵͳ�߼�
 */
void key_task(void const * argument)
{
    uint8_t key = 0;
    uint32_t startup_timer = 0;
    
    /* ��ʼ������������ */
    Key_Manager_Init();
    
    /* ����ҳ����ʾ��ʱ */
    startup_timer = osKernelSysTick();
    
    for(;;)
    {
        /* �Զ��л�����ҳ�� */
        if(g_current_page_id == PAGE_STARTUP && 
           (osKernelSysTick() - startup_timer) > 2000)
        {
            xSemaphoreTake(Data_Mutex, portMAX_DELAY);
            g_current_page_id = PAGE_TEMP_DISPLAY;
            xSemaphoreGive(Data_Mutex);
            
            Send_UI_Message(MSG_PAGE_CHANGE, PAGE_TEMP_DISPLAY, 0, 1);
        }
        
        /* ����ɨ�� */
        key = key_scan(0);
        
        if(key != 0)
        {
            xSemaphoreTake(Data_Mutex, portMAX_DELAY);
            
            if(g_current_mode == MODE_BROWSE)
            {
                Process_Browse_Mode_Key(key);
            }
            else if(g_current_mode == MODE_EDIT)
            {
                Process_Edit_Mode_Key(key);
            }
            
            xSemaphoreGive(Data_Mutex);
        }
        
        osDelay(20);
    }
}

/**
 * @brief  UI���� - ������ʾ����
 */
void ui_task(void const * argument)
{
    UIMessage_t msg;
    uint32_t realtime_counter = 0;
    
    /* ��ʼ��OLED */
    SSD1305_init();
    clearscreen();
    
    /* ��ʾ����ҳ�� */
    Display_Startup_Page();
    
    for(;;)
    {
        /* �����Ϣ���� */
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
                    if(msg.page_id == PAGE_GUN_SETTING || 
                       msg.page_id == PAGE_CAVITY_SETTING)
                    {
                        bool highlight = (g_current_mode == MODE_EDIT);
                        Update_Value_Display(msg.page_id, 
                                          g_pages[msg.page_id].current_value, 
                                          highlight);
                    }
                    break;
            }
        }
        
        /* ʵʱ���ݸ��� */
        realtime_counter++;
        if(realtime_counter >= 25)  // 500ms
        {
            realtime_counter = 0;
            
            xSemaphoreTake(Data_Mutex, portMAX_DELAY);
            Update_Realtime_Data();
            xSemaphoreGive(Data_Mutex);
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
    Filter_Init_All();                                           /* ��ʼ���˲��� */
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


