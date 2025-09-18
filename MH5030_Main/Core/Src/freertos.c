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
#include "tps02r.h"
#include "FM24CXX.h"
#include "onewire.h"
#include "ds18b20.h"
#include "MAX6675.h"
#include "ntc.h"
#include "pid.h"
#include "fan_control.h"
#include "iwdg.h"
#include "key.h"
#include "temp_monitor.h"
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
  osThreadDef(HeaterCtrl_Task, heaterctrl_task, osPriorityAboveNormal, 0, 128);
  HeaterCtrl_TaskHandle = osThreadCreate(osThread(HeaterCtrl_Task), NULL);

  /* definition and creation of Fan_Task */
  osThreadDef(Fan_Task, fan_task, osPriorityNormal, 0, 128);
  Fan_TaskHandle = osThreadCreate(osThread(Fan_Task), NULL);

  /* definition and creation of TCouple_Task */
  osThreadDef(TCouple_Task, tcouple_task, osPriorityIdle, 0, 128);
  TCouple_TaskHandle = osThreadCreate(osThread(TCouple_Task), NULL);

  /* definition and creation of DS18B20_Task */
  osThreadDef(DS18B20_Task, ds18b20_task, osPriorityIdle, 0, 128);
  DS18B20_TaskHandle = osThreadCreate(osThread(DS18B20_Task), NULL);

  /* definition and creation of Wdg_Task */
  osThreadDef(Wdg_Task, wdg_task, osPriorityIdle, 0, 64);
  Wdg_TaskHandle = osThreadCreate(osThread(Wdg_Task), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
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
    tps02r_iic_init(&g_stTps02r_IICManger);                                     /* Initialize tps02r */
    pid_front_init(&g_stPidFront,&g_stPidFrontAuto,&g_stFilterFront);           /* Initialize front PID controller */
    pid_rear_init(&g_stPidRear,&g_stPidRearAuto, &g_stFilterRear);              /* Initialize rear PID controller */
    float pt100_front = 0;                                                      /* Record front temperature */
    float pt100_rear = 0;                                                       /* Record rear temperature */
  /* Infinite loop */
  for(;;)
  {
          
     if(tps02r_get_temp(TPS02R_CHAN2,&pt100_front) == TPS02R_FUN_OK)          
     {
           g_stPidFront.Pv = combined_filter(&g_stFilterFront,pt100_front);     
           #ifdef Test
                RelayFeedbackAutoTuning(&g_stPidFront, &g_stPidFrontAuto);
           #endif
         
           #ifdef Run    
                RUN(&g_stPidFront, &g_stPidFrontAuto);
           #endif
    
            PID_Calc(&g_stPidFront, &g_stPidFrontAuto);                    
            TIM4->CCR4 = g_stPidFront.OUT - 1;        
     }
     
     if(tps02r_get_temp(TPS02R_CHAN1,&pt100_rear) == TPS02R_FUN_OK)          
     {
           g_stPidRear.Pv = combined_filter(&g_stFilterFront,pt100_rear);     
           #ifdef Test
                RelayFeedbackAutoTuning(&g_stPidRear, &g_stPidRearAuto);
           #endif
         
           #ifdef Run    
                RUN(&g_stPidRear, &g_stPidRearAuto);
           #endif
    
            PID_Calc(&g_stPidRear, &g_stPidRearAuto);                    
            TIM1->CCR2 = g_stPidRear.OUT - 1;        
     }

    osDelay(200);
  }
  /* USER CODE END StartDefaultTask */
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
  /* Infinite loop */
    for(;;)
    {
        Fan_Update();                                            /* Update fan status */
        if(g_stFanStatus.fault_consec >= 10)                     /* Continuous fault count >= 10? */
        { 
            HAL_GPIO_WritePin(GPIOB, BELL_Pin, GPIO_PIN_SET);    /* Trigger alarm */
            g_stFanStatus.fault = 1;                             /* Set fault flag */
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


