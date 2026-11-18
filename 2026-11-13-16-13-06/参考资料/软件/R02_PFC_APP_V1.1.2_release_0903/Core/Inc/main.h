/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32g4xx_hal.h"

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_out_Pin GPIO_PIN_13
#define LED_out_GPIO_Port GPIOC
#define Debug_Key_Pin GPIO_PIN_14
#define Debug_Key_GPIO_Port GPIOC
#define RELAY_CTRL_Pin GPIO_PIN_15
#define RELAY_CTRL_GPIO_Port GPIOC
#define NRST_Pin GPIO_PIN_10
#define NRST_GPIO_Port GPIOG
#define VBUS_adc_Pin GPIO_PIN_0
#define VBUS_adc_GPIO_Port GPIOA
#define ILin_Sense_adc_Pin GPIO_PIN_1
#define ILin_Sense_adc_GPIO_Port GPIOA
#define ILin_Ref_adc_Pin GPIO_PIN_2
#define ILin_Ref_adc_GPIO_Port GPIOA
#define ACin_adc_Pin GPIO_PIN_3
#define ACin_adc_GPIO_Port GPIOA
#define P12V_adc_Pin GPIO_PIN_4
#define P12V_adc_GPIO_Port GPIOA
#define VREF1V5_adc_Pin GPIO_PIN_5
#define VREF1V5_adc_GPIO_Port GPIOA
#define TEMP_adc_Pin GPIO_PIN_6
#define TEMP_adc_GPIO_Port GPIOA
#define Vbus_OVP_Pin GPIO_PIN_7
#define Vbus_OVP_GPIO_Port GPIOA
#define iL_ocpH_Pin GPIO_PIN_0
#define iL_ocpH_GPIO_Port GPIOB
#define iL_ocpL_Pin GPIO_PIN_1
#define iL_ocpL_GPIO_Port GPIOB
#define TEMP2_adc_Pin GPIO_PIN_2
#define TEMP2_adc_GPIO_Port GPIOB
#define TEMP3_adc_Pin GPIO_PIN_11
#define TEMP3_adc_GPIO_Port GPIOB
#define FastMOS_B_Pin GPIO_PIN_13
#define FastMOS_B_GPIO_Port GPIOB
#define SlowMos_B_Pin GPIO_PIN_14
#define SlowMos_B_GPIO_Port GPIOB
#define SlowMos_A_Pin GPIO_PIN_15
#define SlowMos_A_GPIO_Port GPIOB
#define FastMOS_A_Pin GPIO_PIN_8
#define FastMOS_A_GPIO_Port GPIOA
#define LLC_OK_Pin GPIO_PIN_4
#define LLC_OK_GPIO_Port GPIOB
#define PFC_OK_Pin GPIO_PIN_5
#define PFC_OK_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_adc2;

extern COMP_HandleTypeDef hcomp1;
extern COMP_HandleTypeDef hcomp2;
extern COMP_HandleTypeDef hcomp4;

extern DAC_HandleTypeDef hdac1;
extern DAC_HandleTypeDef hdac3;

//extern IWDG_HandleTypeDef hiwdg;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
