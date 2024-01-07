/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "stm32l5xx_hal.h"

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

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define CIN_FILTERED_Pin GPIO_PIN_2
#define CIN_FILTERED_GPIO_Port GPIOA
#define VZERO_Pin GPIO_PIN_4
#define VZERO_GPIO_Port GPIOA
#define DAC_OUT_Pin GPIO_PIN_5
#define DAC_OUT_GPIO_Port GPIOA
#define VOUT_Pin GPIO_PIN_6
#define VOUT_GPIO_Port GPIOA
#define VOUT_FILTERED_Pin GPIO_PIN_4
#define VOUT_FILTERED_GPIO_Port GPIOC
#define VOUT_BUFFERED_Pin GPIO_PIN_0
#define VOUT_BUFFERED_GPIO_Port GPIOB
#define COMM_SCK_Pin GPIO_PIN_10
#define COMM_SCK_GPIO_Port GPIOB
#define COMM_CS_Pin GPIO_PIN_12
#define COMM_CS_GPIO_Port GPIOB
#define COMM_CS_EXTI_IRQn EXTI12_IRQn
#define COMM_DRDY_Pin GPIO_PIN_13
#define COMM_DRDY_GPIO_Port GPIOB
#define COMM_MISO_Pin GPIO_PIN_14
#define COMM_MISO_GPIO_Port GPIOB
#define COMM_MOSI_Pin GPIO_PIN_15
#define COMM_MOSI_GPIO_Port GPIOB
#define COMM_CON_Pin GPIO_PIN_6
#define COMM_CON_GPIO_Port GPIOC
#define VERSION_0_Pin GPIO_PIN_15
#define VERSION_0_GPIO_Port GPIOA
#define VERSION_1_Pin GPIO_PIN_10
#define VERSION_1_GPIO_Port GPIOC
#define STATUS_GOOD_Pin GPIO_PIN_11
#define STATUS_GOOD_GPIO_Port GPIOC
#define STATUS_BAD_Pin GPIO_PIN_12
#define STATUS_BAD_GPIO_Port GPIOC
#define MUX_EN_Pin GPIO_PIN_2
#define MUX_EN_GPIO_Port GPIOD
#define MUX_A0_Pin GPIO_PIN_3
#define MUX_A0_GPIO_Port GPIOB
#define MUX_A1_Pin GPIO_PIN_4
#define MUX_A1_GPIO_Port GPIOB
#define TESTOUT_Pin GPIO_PIN_8
#define TESTOUT_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
