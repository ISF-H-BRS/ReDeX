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

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ID2_0_Pin GPIO_PIN_14
#define ID2_0_GPIO_Port GPIOC
#define ID2_1_Pin GPIO_PIN_15
#define ID2_1_GPIO_Port GPIOC
#define BUS_VOLTAGE_Pin GPIO_PIN_0
#define BUS_VOLTAGE_GPIO_Port GPIOA
#define BUS_CURRENT_Pin GPIO_PIN_1
#define BUS_CURRENT_GPIO_Port GPIOA
#define BUS_VOLTAGE_AMPLIFIED_Pin GPIO_PIN_3
#define BUS_VOLTAGE_AMPLIFIED_GPIO_Port GPIOA
#define ID1_0_Pin GPIO_PIN_4
#define ID1_0_GPIO_Port GPIOA
#define ID1_1_Pin GPIO_PIN_5
#define ID1_1_GPIO_Port GPIOA
#define SCL1_Pin GPIO_PIN_7
#define SCL1_GPIO_Port GPIOA
#define VARIANT0_Pin GPIO_PIN_0
#define VARIANT0_GPIO_Port GPIOB
#define VARIANT1_Pin GPIO_PIN_1
#define VARIANT1_GPIO_Port GPIOB
#define STATUS_GOOD_Pin GPIO_PIN_15
#define STATUS_GOOD_GPIO_Port GPIOA
#define STATUS_BAD_Pin GPIO_PIN_3
#define STATUS_BAD_GPIO_Port GPIOB
#define SDA1_Pin GPIO_PIN_4
#define SDA1_GPIO_Port GPIOB
#define SCL2_Pin GPIO_PIN_6
#define SCL2_GPIO_Port GPIOB
#define SDA2_Pin GPIO_PIN_7
#define SDA2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
