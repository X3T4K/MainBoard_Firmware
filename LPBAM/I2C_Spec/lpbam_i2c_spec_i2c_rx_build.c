/* USER CODE BEGIN Header */
/**
  **********************************************************************************************************************
  * @file   lpbam_i2c_spec_i2c_rx_build.c
  * @author MCD Application Team
  * @brief  Provides LPBAM I2C_Spec application I2C_RX scenario build services
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
/* Includes ----------------------------------------------------------------------------------------------------------*/
#include "lpbam_i2c_spec.h"

/* Private variables -------------------------------------------------------------------------------------------------*/
/* LPBAM variables declaration */
/* USER CODE BEGIN I2C_Spec_I2C_RX_Descs 0 */

/* USER CODE END I2C_Spec_I2C_RX_Descs 0 */

/* USER CODE BEGIN I2CAcq_Q_Master_Transmit_Data_1_Desc[1] */

/* USER CODE END I2CAcq_Q_Master_Transmit_Data_1_Desc[1] */
static LPBAM_I2C_MasterTxDataDesc_t I2CAcq_Q_Master_Transmit_Data_1_Desc[1];

/* USER CODE BEGIN I2CAcq_Q_Master_Receive_Data_1_Desc[1] */

/* USER CODE END I2CAcq_Q_Master_Receive_Data_1_Desc[1] */
static LPBAM_I2C_MasterRxDataDesc_t I2CAcq_Q_Master_Receive_Data_1_Desc[1];

/* USER CODE BEGIN I2C_Spec_I2C_RX_Descs 1 */

/* USER CODE END I2C_Spec_I2C_RX_Descs 1 */

/* Exported variables ------------------------------------------------------------------------------------------------*/
/* LPBAM queues declaration */
DMA_QListTypeDef I2CAcq_Q;

/* External variables ------------------------------------------------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Private function prototypes ---------------------------------------------------------------------------------------*/
static void MX_I2CAcq_Q_Build(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Exported functions ------------------------------------------------------------------------------------------------*/
/**
  * @brief I2C_Spec application I2C_RX scenario build
  * @param None
  * @retval None
  */
void MX_I2C_Spec_I2C_RX_Build(void)
{
  /* USER CODE BEGIN I2C_Spec_I2C_RX_Build 0 */

  /* USER CODE END I2C_Spec_I2C_RX_Build 0 */

  /* LPBAM build I2CAcq queue */
  MX_I2CAcq_Q_Build();

  /* USER CODE BEGIN I2C_Spec_I2C_RX_Build 1 */

  /* USER CODE END I2C_Spec_I2C_RX_Build 1 */
}

/* Private functions -------------------------------------------------------------------------------------------------*/

/**
  * @brief  I2C_Spec application I2C_RX scenario I2CAcq queue build
  * @param  None
  * @retval None
  */
static void MX_I2CAcq_Q_Build(void)
{
  /* LPBAM build variable */
  LPBAM_DMAListInfo_t pDMAListInfo_I2C = {0};
  LPBAM_I2C_DataAdvConf_t pTxData_I2C = {0};
  LPBAM_COMMON_TrigAdvConf_t pTrigConfig_I2C = {0};
  LPBAM_I2C_DataAdvConf_t pRxData_I2C = {0};
  uint32_t data_size = 0;
  uint32_t tmp_data_size = 0;
  uint32_t transfer_idx = 0;

  /**
    * I2CAcq queue Master_Transmit_Data_1 build
    */
   pDMAListInfo_I2C.QueueType= LPBAM_LINEAR_ADDRESSING_Q;
   pDMAListInfo_I2C.pInstance= LPDMA1;
  pTxData_I2C.AutoModeConf.TriggerState = LPBAM_I2C_AUTO_MODE_DISABLE;
   pTxData_I2C.AddressingMode = LPBAM_I2C_ADDRESSINGMODE_7BIT;
   pTxData_I2C.SequenceNumber = 1;
   pTxData_I2C.pData = (uint8_t*)&AS7341_start_register[DataBufferOffset];
   pTxData_I2C.DevAddress = 0x95;
   pTxData_I2C.Size = 1;
  /* Set transfer parameters */
  data_size = pTxData_I2C.Size;
  tmp_data_size = pTxData_I2C.Size;
  transfer_idx = 0;

  /* Repeat inserting I2C master Tx data queue until completing all data */
  while (data_size != 0U)
  {
    if (ADV_LPBAM_I2C_MasterTx_SetDataQ(I2C3, &pDMAListInfo_I2C, &pTxData_I2C, &I2CAcq_Q_Master_Transmit_Data_1_Desc[transfer_idx], &I2CAcq_Q) != LPBAM_OK)
    {
      Error_Handler();
    }

    transfer_idx++;

    if (data_size > LPBAM_I2C_MAX_DATA_SIZE)
    {
      data_size -= LPBAM_I2C_MAX_DATA_SIZE;
    }
    else
    {
      data_size = 0U;
    }

    pTxData_I2C.Size = data_size;
  }
  pTxData_I2C.Size = tmp_data_size;
  pTrigConfig_I2C.TriggerConfig.TriggerMode = LPBAM_DMA_TRIGM_BLOCK_TRANSFER;
  pTrigConfig_I2C.TriggerConfig.TriggerPolarity = LPBAM_DMA_TRIG_POLARITY_RISING;
  pTrigConfig_I2C.TriggerConfig.TriggerSelection = LPBAM_LPDMA1_TRIGGER_LPTIM1_CH1;
  if (ADV_LPBAM_Q_SetTriggerConfig (&pTrigConfig_I2C, LPBAM_I2C_MASTERTX_DATAQ_CONFIG_NODE, &I2CAcq_Q_Master_Transmit_Data_1_Desc) != LPBAM_OK)
  {
    Error_Handler();
  }

  /**
    * I2CAcq queue Master_Receive_Data_1 build
    */
  pRxData_I2C.AutoModeConf.TriggerState = LPBAM_I2C_AUTO_MODE_DISABLE;
   pRxData_I2C.AddressingMode = LPBAM_I2C_ADDRESSINGMODE_7BIT;
   pRxData_I2C.SequenceNumber = 1;
   pRxData_I2C.pData = (uint8_t*)&AS7341_Rx_Buffer[DataBufferOffset];
   pRxData_I2C.DevAddress = 0x39;
   pRxData_I2C.Size = 12;
  /* Set transfer parameters */
  data_size = pRxData_I2C.Size;
  tmp_data_size = pRxData_I2C.Size;
  transfer_idx = 0;

  /* Repeat inserting I2C master Tx data queue until completing all data */
  while (data_size != 0U)
  {
    if (ADV_LPBAM_I2C_MasterRx_SetDataQ(I2C3, &pDMAListInfo_I2C, &pRxData_I2C, &I2CAcq_Q_Master_Receive_Data_1_Desc[transfer_idx], &I2CAcq_Q) != LPBAM_OK)
    {
      Error_Handler();
    }

    transfer_idx++;

    if (data_size > LPBAM_I2C_MAX_DATA_SIZE)
    {
      data_size -= LPBAM_I2C_MAX_DATA_SIZE;
    }
    else
    {
      data_size = 0U;
    }

    pRxData_I2C.Size = data_size;
  }
  pRxData_I2C.Size = tmp_data_size;
  if (ADV_LPBAM_Q_SetTriggerConfig (&pTrigConfig_I2C, LPBAM_I2C_MASTERRX_DATAQ_CONFIG_NODE, &I2CAcq_Q_Master_Receive_Data_1_Desc) != LPBAM_OK)
  {
    Error_Handler();
  }

  /**
    * Set circular mode
    */
  if (ADV_LPBAM_Q_SetCircularMode(&I2CAcq_Q_Master_Transmit_Data_1_Desc, LPBAM_I2C_MASTERTX_DATAQ_CONFIG_NODE, &I2CAcq_Q) != LPBAM_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN I2C_Spec_I2C_RX_Build */

/* USER CODE END I2C_Spec_I2C_RX_Build */
