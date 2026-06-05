/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    mdf.c
  * @brief   This file provides code for the configuration
  *          of the MDF instances.
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
/* Includes ------------------------------------------------------------------*/
#include "mdf.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

MDF_HandleTypeDef MdfHandle0;
MDF_FilterConfigTypeDef MdfFilterConfig0;
MDF_HandleTypeDef MdfHandle1;
MDF_FilterConfigTypeDef MdfFilterConfig1;
MDF_OldConfigTypeDef mdfOldConfig1;
DMA_HandleTypeDef handle_GPDMA1_Channel0;

/* MDF1 init function */
void MX_MDF1_Init(void)
{

  /* USER CODE BEGIN MDF1_Init 0 */

  /* USER CODE END MDF1_Init 0 */

  /* USER CODE BEGIN MDF1_Init 1 */

  /* USER CODE END MDF1_Init 1 */

  /**
    MdfHandle0 structure initialization and HAL_MDF_Init function call
  */
  MdfHandle0.Instance = MDF1_Filter0;
  MdfHandle0.Init.CommonParam.InterleavedFilters = 0;
  MdfHandle0.Init.CommonParam.ProcClockDivider = 1;
  MdfHandle0.Init.CommonParam.OutputClock.Activation = ENABLE;
  MdfHandle0.Init.CommonParam.OutputClock.Pins = MDF_OUTPUT_CLOCK_0;
  MdfHandle0.Init.CommonParam.OutputClock.Divider = 5;
  MdfHandle0.Init.CommonParam.OutputClock.Trigger.Activation = ENABLE;
  MdfHandle0.Init.CommonParam.OutputClock.Trigger.Source = MDF_CLOCK_TRIG_TIM1_TRGO;
  MdfHandle0.Init.CommonParam.OutputClock.Trigger.Edge = MDF_CLOCK_TRIG_RISING_EDGE;
  MdfHandle0.Init.SerialInterface.Activation = ENABLE;
  MdfHandle0.Init.SerialInterface.Mode = MDF_SITF_NORMAL_SPI_MODE;
  MdfHandle0.Init.SerialInterface.ClockSource = MDF_SITF_CCK0_SOURCE;
  MdfHandle0.Init.SerialInterface.Threshold = 31;
  MdfHandle0.Init.FilterBistream = MDF_BITSTREAM0_RISING;
  if (HAL_MDF_Init(&MdfHandle0) != HAL_OK)
  {
    Error_Handler();
  }

  /**
    MdfFilterConfig0, MdfOldConfig0 and/or MdfScdConfig0 structures initialization

    WARNING : only structures are filled, no specific init function call for filter
  */
  MdfFilterConfig0.DataSource = MDF_DATA_SOURCE_BSMX;
  MdfFilterConfig0.Delay = 0;
  MdfFilterConfig0.CicMode = MDF_ONE_FILTER_SINC5;
  MdfFilterConfig0.DecimationRatio = 16;
  MdfFilterConfig0.Offset = 0;
  MdfFilterConfig0.Gain = 1;
  MdfFilterConfig0.ReshapeFilter.Activation = ENABLE;
  MdfFilterConfig0.ReshapeFilter.DecimationRatio = MDF_RSF_DECIMATION_RATIO_4;
  MdfFilterConfig0.HighPassFilter.Activation = ENABLE;
  MdfFilterConfig0.HighPassFilter.CutOffFrequency = MDF_HPF_CUTOFF_0_000625FPCM;
  MdfFilterConfig0.Integrator.Activation = ENABLE;
  MdfFilterConfig0.Integrator.Value = 2;
  MdfFilterConfig0.Integrator.OutputDivision = MDF_INTEGRATOR_OUTPUT_DIV_128;
  MdfFilterConfig0.SoundActivity.Activation = DISABLE;
  MdfFilterConfig0.AcquisitionMode = MDF_MODE_SYNC_CONT;
  MdfFilterConfig0.FifoThreshold = MDF_FIFO_THRESHOLD_NOT_EMPTY;
  MdfFilterConfig0.DiscardSamples = 255;
  MdfFilterConfig0.Trigger.Source = MDF_CLOCK_TRIG_TRGO;
  MdfFilterConfig0.Trigger.Edge = MDF_FILTER_TRIG_RISING_EDGE;

  /**
    MdfHandle1 structure initialization and HAL_MDF_Init function call
  */
  MdfHandle1.Instance = MDF1_Filter1;
  MdfHandle1.Init.CommonParam.InterleavedFilters = 0;
  MdfHandle1.Init.CommonParam.ProcClockDivider = 1;
  MdfHandle1.Init.CommonParam.OutputClock.Activation = ENABLE;
  MdfHandle1.Init.CommonParam.OutputClock.Pins = MDF_OUTPUT_CLOCK_0;
  MdfHandle1.Init.CommonParam.OutputClock.Divider = 5;
  MdfHandle1.Init.CommonParam.OutputClock.Trigger.Activation = ENABLE;
  MdfHandle1.Init.CommonParam.OutputClock.Trigger.Source = MDF_CLOCK_TRIG_TIM1_TRGO;
  MdfHandle1.Init.CommonParam.OutputClock.Trigger.Edge = MDF_CLOCK_TRIG_RISING_EDGE;
  MdfHandle1.Init.SerialInterface.Activation = DISABLE;
  if (HAL_MDF_Init(&MdfHandle1) != HAL_OK)
  {
    Error_Handler();
  }

  /**
    MdfFilterConfig1, MdfOldConfig1 and/or MdfScdConfig1 structures initialization

    WARNING : only structures are filled, no specific init function call for filter
  */
  MdfFilterConfig1.DataSource = MDF_DATA_SOURCE_BSMX;
  MdfFilterConfig1.Delay = 0;
  MdfFilterConfig1.CicMode = MDF_TWO_FILTERS_MCIC_SINC3;
  MdfFilterConfig1.DecimationRatio = 32;
  MdfFilterConfig1.Offset = 0;
  MdfFilterConfig1.Gain = 0;
  MdfFilterConfig1.ReshapeFilter.Activation = DISABLE;
  MdfFilterConfig1.HighPassFilter.Activation = DISABLE;
  MdfFilterConfig1.Integrator.Activation = ENABLE;
  MdfFilterConfig1.Integrator.Value = 2;
  MdfFilterConfig1.Integrator.OutputDivision = MDF_INTEGRATOR_OUTPUT_DIV_32;
  MdfFilterConfig1.AcquisitionMode = MDF_MODE_ASYNC_CONT;
  MdfFilterConfig1.FifoThreshold = MDF_FIFO_THRESHOLD_NOT_EMPTY;
  MdfFilterConfig1.DiscardSamples = 0;
  mdfOldConfig1.OldCicMode = MDF_OLD_FILTER_FASTSINC;
  mdfOldConfig1.OldDecimationRatio = 32;
  mdfOldConfig1.HighThreshold = 3355443;
  mdfOldConfig1.LowThreshold = -3355443;
  mdfOldConfig1.OldEventConfig = MDF_OLD_SIGNAL_OUTSIDE_THRESHOLDS;
  mdfOldConfig1.BreakSignal = MDF_NO_BREAK_SIGNAL;

  /* USER CODE BEGIN MDF1_Init 2 */

  /* USER CODE END MDF1_Init 2 */

}

void HAL_MDF_MspInit(MDF_HandleTypeDef* mdfHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(mdfHandle->Instance == MDF1_Filter0)
  {
  /* USER CODE BEGIN MDF1_MspInit 0 */

  /* USER CODE END MDF1_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_MDF1;
    PeriphClkInit.Mdf1ClockSelection = RCC_MDF1CLKSOURCE_PLL3;
    PeriphClkInit.PLL3.PLL3Source = RCC_PLLSOURCE_HSE;
    PeriphClkInit.PLL3.PLL3M = 3;
    PeriphClkInit.PLL3.PLL3N = 48;
    PeriphClkInit.PLL3.PLL3P = 2;
    PeriphClkInit.PLL3.PLL3Q = 25;
    PeriphClkInit.PLL3.PLL3R = 2;
    PeriphClkInit.PLL3.PLL3RGE = RCC_PLLVCIRANGE_1;
    PeriphClkInit.PLL3.PLL3FRACN = 0;
    PeriphClkInit.PLL3.PLL3ClockOut = RCC_PLL3_DIVP;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

    /* MDF1 clock enable */
    __HAL_RCC_MDF1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**MDF1 GPIO Configuration
    PB1     ------> MDF1_SDI0
    PB8     ------> MDF1_CCK0
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF6_MDF1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_MDF1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    __HAL_SYSCFG_FASTMODEPLUS_ENABLE(SYSCFG_FASTMODEPLUS_PB8);

    /* MDF1 DMA Init */
    /* GPDMA1_REQUEST_MDF1_FLT0 Init */
    handle_GPDMA1_Channel0.Instance = GPDMA1_Channel0;
    handle_GPDMA1_Channel0.Init.Request = GPDMA1_REQUEST_MDF1_FLT0;
    handle_GPDMA1_Channel0.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    handle_GPDMA1_Channel0.Init.Direction = DMA_PERIPH_TO_MEMORY;
    handle_GPDMA1_Channel0.Init.SrcInc = DMA_SINC_FIXED;
    handle_GPDMA1_Channel0.Init.DestInc = DMA_DINC_FIXED;
    handle_GPDMA1_Channel0.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_WORD;
    handle_GPDMA1_Channel0.Init.DestDataWidth = DMA_DEST_DATAWIDTH_WORD;
    handle_GPDMA1_Channel0.Init.Priority = DMA_LOW_PRIORITY_LOW_WEIGHT;
    handle_GPDMA1_Channel0.Init.SrcBurstLength = 1;
    handle_GPDMA1_Channel0.Init.DestBurstLength = 1;
    handle_GPDMA1_Channel0.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0|DMA_DEST_ALLOCATED_PORT0;
    handle_GPDMA1_Channel0.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    handle_GPDMA1_Channel0.Init.Mode = DMA_NORMAL;
    if (HAL_DMA_Init(&handle_GPDMA1_Channel0) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(mdfHandle, hdma, handle_GPDMA1_Channel0);

    if (HAL_DMA_ConfigChannelAttributes(&handle_GPDMA1_Channel0, DMA_CHANNEL_NPRIV) != HAL_OK)
    {
      Error_Handler();
    }

  }

    /* MDF1 interrupt Init */
    HAL_NVIC_SetPriority(MDF1_FLT0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(MDF1_FLT0_IRQn);
    HAL_NVIC_SetPriority(MDF1_FLT1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(MDF1_FLT1_IRQn);
  /* USER CODE BEGIN MDF1_MspInit 1 */

  /* USER CODE END MDF1_MspInit 1 */
}

void HAL_MDF_MspDeInit(MDF_HandleTypeDef* mdfHandle)
{

  if(IS_MDF_INSTANCE(mdfHandle->Instance))
  {
  /* USER CODE BEGIN MDF1_MspDeInit 0 */

  /* USER CODE END MDF1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_MDF1_CLK_DISABLE();

    /**MDF1 GPIO Configuration
    PB1     ------> MDF1_SDI0
    PB8     ------> MDF1_CCK0
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_1|GPIO_PIN_8);

    /* MDF1 DMA DeInit */
    HAL_DMA_DeInit(mdfHandle->hdma);

    /* MDF1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(MDF1_FLT0_IRQn);
    HAL_NVIC_DisableIRQ(MDF1_FLT1_IRQn);
  /* USER CODE BEGIN MDF1_MspDeInit 1 */

  /* USER CODE END MDF1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

