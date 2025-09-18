/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "dma.h"
#include "iwdg.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
    
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
//  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */
  
  delay_init(72);                     
  
/*-------------------------------------FAN TEST PROGRAM----------------------------------------------*/

//  HAL_TIM_PWM_Start (&htim3, TIM_CHANNEL_1);
//  HAL_GPIO_WritePin (GPIOB,GPIO_PIN_3,GPIO_PIN_RESET);
  Fan_Init();
  Fan_Start();
  Filter_Init_All();
//  Filter_Test();
/*-------------------------------------FAN TEST PROGRAM----------------------------------------------*/
  
/*-------------------------------------PWM TEST PROGRAM----------------------------------------------*/

//  HAL_TIM_PWM_Start (&htim1, TIM_CHANNEL_2);
//  HAL_TIM_PWM_Start (&htim4, TIM_CHANNEL_4);

/*-------------------------------------PWM TEST PROGRAM----------------------------------------------*/


/*-------------------------------------MAX6675 TEST PROGRAM------------------------------------------*/
//  MAX6675_Setup();
//  Temperature_Monitor_Task();
/*-------------------------------------MAX6675 TEST PROGRAM------------------------------------------*/
  
  
/*-------------------------------------ds18b20 test program------------------------------------------*/
//  u8 error = 0; 
//  OneWire_Init(&OneWire_2,DS18B20_2_GPIO_Port, DS18B20_2_Pin); 
//  error=OneWire_Reset(&OneWire_2);

//  if(error==0)
//  {
//      printf("ok!!!\n");
//  }else
//  {
//      printf("error!!!\n");
//  }
/*-------------------------------------ds18b20 test program-------------------------------------------*/


/*-------------------------------------TPS02R test program----------------------------------------------*/
//  u8 error = 0;                                                             
//  tps02r_iic_init(&g_stTps02r_IICManger);                                        
//  error=tps02r_cfg_init();
//  if(error==0)
//  {
//      printf("ok!!!\n");
//  }else
//  {
//      printf("error!!!\n");
//  }
/*-------------------------------------TPS02R test program----------------------------------------------*/
  
  
/*-------------------------------------FM24CXX test program----------------------------------------------*/
//  FM24CXX_iic_init(&fm24cxx_iicmanager);
//  printf("test\r\n");
//  if(FM_Check()==0)
//  {
//      printf("ok!!!\n");
//  }
//  char result = FM_ReadOneByte(EE_TYPE);
//  printf("result = %d ",result);
/*-------------------------------------FM24CXX test program----------------------------------------------*/

  
/*-------------------------------------SSD1305 test program----------------------------------------------*/
  SSD1305_init();                                       
  clearscreen();

// /* 起始界面 */
//  DispString(40, 16, (unsigned char*)"MH5030",   false);
//  DispString(28, 40, (unsigned char*)"Ver 1.0.0", false);

    /*  */
    DispString(0, 0, (unsigned char*)"温度",   false);
//  draw_rect(10, 8, 50, 30, false);
//  draw_hline(0,23,32,false);
//  Show_Word_U(0, 0, 12311, 5, 2, false);   // 显示为 123.11
//  Disp_Char(0, 10, 'A', false);                         
//  DispString(0, 0, (unsigned char*)"前枪管", false);    
/*-------------------------------------SSD1305 test program----------------------------------------------*/
  /* USER CODE END 2 */

//  /* Call init function for freertos objects (in cmsis_os2.c) */
//  MX_FREERTOS_Init();

//  /* Start scheduler */
//  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint32_t num = 0;
  while (1)
  {
//    num++;
//    if(num >1000)
//    {
//        num = 0;
//    }
//    Show_Word_U(0, 0, num, 5, 0, true);   
//    delay_ms(200);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM2 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */
    
  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM2)
  {
    HAL_IncTick();
  }

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
      
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

