/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
#define bool _Bool
#define true 1
#define false 0
    
typedef unsigned char cchar;   
typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef int32_t s32;  
typedef uint32_t u32;   
typedef uint16_t u16;   
typedef uint8_t u8;   
typedef uint16_t uc16;   
typedef unsigned char uc8;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define FM24CL16_WP_Pin GPIO_PIN_13
#define FM24CL16_WP_GPIO_Port GPIOC
#define FM24CL16_SCL_Pin GPIO_PIN_14
#define FM24CL16_SCL_GPIO_Port GPIOC
#define FM24CL16_SDA_Pin GPIO_PIN_15
#define FM24CL16_SDA_GPIO_Port GPIOC
#define NTC_Pin GPIO_PIN_0
#define NTC_GPIO_Port GPIOA
#define SSD1305_CS_Pin GPIO_PIN_1
#define SSD1305_CS_GPIO_Port GPIOA
#define SSD1305_RST_Pin GPIO_PIN_2
#define SSD1305_RST_GPIO_Port GPIOA
#define SSD1305_DC_Pin GPIO_PIN_3
#define SSD1305_DC_GPIO_Port GPIOA
#define SSD1305_CLK_Pin GPIO_PIN_4
#define SSD1305_CLK_GPIO_Port GPIOA
#define SSD1305_DI_Pin GPIO_PIN_5
#define SSD1305_DI_GPIO_Port GPIOA
#define KEY4_Pin GPIO_PIN_6
#define KEY4_GPIO_Port GPIOA
#define KEY3_Pin GPIO_PIN_7
#define KEY3_GPIO_Port GPIOA
#define KEY2_Pin GPIO_PIN_0
#define KEY2_GPIO_Port GPIOB
#define KEY1_Pin GPIO_PIN_1
#define KEY1_GPIO_Port GPIOB
#define TPS02RAH_SCL_Pin GPIO_PIN_10
#define TPS02RAH_SCL_GPIO_Port GPIOB
#define TPS02RAH_SDA_Pin GPIO_PIN_11
#define TPS02RAH_SDA_GPIO_Port GPIOB
#define BELL_Pin GPIO_PIN_12
#define BELL_GPIO_Port GPIOB
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOB
#define DS18B20_2_Pin GPIO_PIN_15
#define DS18B20_2_GPIO_Port GPIOB
#define MAX6675_SO_B_Pin GPIO_PIN_8
#define MAX6675_SO_B_GPIO_Port GPIOA
#define HEAT2_Pin GPIO_PIN_9
#define HEAT2_GPIO_Port GPIOA
#define MAX6675_SO_A_Pin GPIO_PIN_10
#define MAX6675_SO_A_GPIO_Port GPIOA
#define MAX6675__CS_Pin GPIO_PIN_11
#define MAX6675__CS_GPIO_Port GPIOA
#define MAX6675_SCK_Pin GPIO_PIN_12
#define MAX6675_SCK_GPIO_Port GPIOA
#define FFG_Pin GPIO_PIN_15
#define FFG_GPIO_Port GPIOA
#define FCTR_Pin GPIO_PIN_3
#define FCTR_GPIO_Port GPIOB
#define FPWM_Pin GPIO_PIN_4
#define FPWM_GPIO_Port GPIOB
#define DS18B20_1_Pin GPIO_PIN_8
#define DS18B20_1_GPIO_Port GPIOB
#define HEAT1_Pin GPIO_PIN_9
#define HEAT1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */


/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
