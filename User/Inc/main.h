/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>

  /* Private includes ----------------------------------------------------------*/
  /* USER CODE BEGIN Includes */

  /* USER CODE END Includes */

  /* Exported types ------------------------------------------------------------*/
  /* USER CODE BEGIN ET */

  /* USER CODE END ET */

  /* Exported constants --------------------------------------------------------*/
  /* USER CODE BEGIN EC */
  extern __IO int sdio_int_num;
  extern __IO int dma_int_num;
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
/* System error output */
#define SYS_ERR(x)                                        \
  do                                                      \
  {                                                       \
    printf("Error:%s\n\t%s:%d\n", x, __FILE__, __LINE__); \
  } while (0)
  
#define SYS_LOG(x, y)         \
  do                          \
  {                           \
    printf("%s: %s\n", x, y); \
  } while (0)

#define HAL_CHECK(x)        \
  do                        \
  {                         \
    if (x != HAL_OK)        \
    {                       \
      SYS_ERR("HAL != OK"); \
    }                       \
  } while (0)

  /* USER CODE END EM */

  /* Exported functions prototypes ---------------------------------------------*/

  /* USER CODE BEGIN EFP */
  void Error_Handler(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED0_Pin GPIO_PIN_9
#define LED0_GPIO_Port GPIOF
#define LED1_Pin GPIO_PIN_10
#define LED1_GPIO_Port GPIOF
#define LCD_BL_Pin GPIO_PIN_15
#define LCD_BL_GPIO_Port GPIOB
#define TOUCH_RST_Pin GPIO_PIN_13
#define TOUCH_RST_GPIO_Port GPIOC
#define TOUCH_INT_Pin GPIO_PIN_1
#define TOUCH_INT_GPIO_Port GPIOB
#define TOUCH_IIC_CLK_Pin GPIO_PIN_0
#define TOUCH_IIC_CLK_GPIO_Port GPIOB
#define TOUCH_IIC_SDA_Pin GPIO_PIN_11
#define TOUCH_IIC_SDA_GPIO_Port GPIOF

  /* USER CODE BEGIN Private defines */

  /* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
