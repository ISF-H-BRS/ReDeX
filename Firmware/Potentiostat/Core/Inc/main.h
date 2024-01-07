/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

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
void SamplePeriodElapsedCallback();
void SignalPeriodElapsedCallback();
void PowerPeriodElapsedCallback();
void BlinkPeriodElapsedCallback();
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PWR_ENABLE_Pin GPIO_PIN_14
#define PWR_ENABLE_GPIO_Port GPIOC
#define ADC_CONVST_Pin GPIO_PIN_2
#define ADC_CONVST_GPIO_Port GPIOA
#define ADC_CS1_Pin GPIO_PIN_3
#define ADC_CS1_GPIO_Port GPIOA
#define ADC_CS2_Pin GPIO_PIN_4
#define ADC_CS2_GPIO_Port GPIOA
#define ADC_SCK_Pin GPIO_PIN_5
#define ADC_SCK_GPIO_Port GPIOA
#define ADC_MISO_Pin GPIO_PIN_6
#define ADC_MISO_GPIO_Port GPIOA
#define DAC_AUX_Pin GPIO_PIN_7
#define DAC_AUX_GPIO_Port GPIOA
#define STATUS_GOOD_Pin GPIO_PIN_0
#define STATUS_GOOD_GPIO_Port GPIOB
#define STATUS_BAD_Pin GPIO_PIN_1
#define STATUS_BAD_GPIO_Port GPIOB
#define OUTPUT_ENABLE_Pin GPIO_PIN_8
#define OUTPUT_ENABLE_GPIO_Port GPIOA
#define MUX_A0_Pin GPIO_PIN_3
#define MUX_A0_GPIO_Port GPIOB
#define MUX_A1_Pin GPIO_PIN_4
#define MUX_A1_GPIO_Port GPIOB
#define MUX_EN_Pin GPIO_PIN_5
#define MUX_EN_GPIO_Port GPIOB
#define SCL_Pin GPIO_PIN_6
#define SCL_GPIO_Port GPIOB
#define SDA_Pin GPIO_PIN_7
#define SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
