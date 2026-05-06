/* USER CODE BEGIN Header */
/**
  **********************************************************************************************************************
  * @file    lpbam_i2c_spec.h
  * @author  MCD Application Team
  * @brief   Header for LPBAM I2C_Spec application
  **********************************************************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  **********************************************************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -----------------------------------------------------------------------------*/
#ifndef LPBAM_I2C_SPEC_H
#define LPBAM_I2C_SPEC_H

/* Includes ----------------------------------------------------------------------------------------------------------*/
#include "main.h"
#include "stm32_lpbam.h"

/* Exported functions ------------------------------------------------------------------------------------------------*/
/* I2C_Spec application initialization */
void MX_I2C_Spec_Init(void);

/* I2C_Spec application I2C_RX scenario initialization */
void MX_I2C_Spec_I2C_RX_Init(void);

/* I2C_Spec application I2C_RX scenario de-initialization */
void MX_I2C_Spec_I2C_RX_DeInit(void);

/* I2C_Spec application I2C_RX scenario build */
void MX_I2C_Spec_I2C_RX_Build(void);

/* I2C_Spec application I2C_RX scenario link */
void MX_I2C_Spec_I2C_RX_Link(DMA_HandleTypeDef *hdma);

/* I2C_Spec application I2C_RX scenario unlink */
void MX_I2C_Spec_I2C_RX_UnLink(DMA_HandleTypeDef *hdma);

/* I2C_Spec application I2C_RX scenario start */
void MX_I2C_Spec_I2C_RX_Start(DMA_HandleTypeDef *hdma);

/* I2C_Spec application I2C_RX scenario stop */
void MX_I2C_Spec_I2C_RX_Stop(DMA_HandleTypeDef *hdma);

#endif /* LPBAM_I2C_SPEC_H */
