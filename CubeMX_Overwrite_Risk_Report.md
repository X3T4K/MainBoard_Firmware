# 🚨 Analisi Rischio Sovrascrittura CubeMX & LPBAM

Questo documento elenca in dettaglio tutte le modifiche apportate nelle cartelle `Core` e `LPBAM` che **rischiano di essere sovrascritte e perse** se viene rigenerato il codice tramite STM32CubeMX o l'LPBAM Utility.

## 📊 Tabella Riassuntiva dei Rischi

| File | Tipo di File / Stato | Rischio Sovrascrittura | Azione Richiesta |

| :--- | :--- | :--- | :--- |

| `Core/Inc/rtc.h` | Modificato FUORI da USER CODE | 🔴 **CRITICO** | Spostare in USER CODE o allineare `.ioc` |

| `Core/Inc/stm32u5xx_it.h` | Modificato FUORI da USER CODE | 🔴 **CRITICO** | Spostare in USER CODE o allineare `.ioc` |

| `Core/Src/lpdma.c` | Modificato FUORI da USER CODE | 🔴 **CRITICO** | Spostare in USER CODE o allineare `.ioc` |

| `Core/Src/lptim.c` | Modificato FUORI da USER CODE | 🔴 **CRITICO** | Spostare in USER CODE o allineare `.ioc` |

| `Core/Src/main.c` | Modificato FUORI da USER CODE | 🔴 **CRITICO** | Spostare in USER CODE o allineare `.ioc` |

| `Core/Src/rtc.c` | Modificato FUORI da USER CODE | 🔴 **CRITICO** | Spostare in USER CODE o allineare `.ioc` |

| `Core/Src/stm32u5xx_it.c` | Modificato FUORI da USER CODE | 🔴 **CRITICO** | Spostare in USER CODE o allineare `.ioc` |

| `LPBAM/I2C_Spec/lpbam_i2c_spec_config.c` | Generato da LPBAM Utility | 🔴 **CRITICO** | Fare backup e riapplicare dopo la generazione |

| `LPBAM/I2C_Spec/lpbam_i2c_spec_i2c_rx_build.c` | Generato da LPBAM Utility | 🔴 **CRITICO** | Fare backup e riapplicare dopo la generazione |

| `LPBAM/I2C_Spec/lpbam_i2c_spec_i2c_rx_config.c` | Generato da LPBAM Utility | 🔴 **CRITICO** | Fare backup e riapplicare dopo la generazione |

| `Core/Inc/SPI.h` | Modificato solo DENTRO USER CODE | 🟢 Nessuno (Sicuro) | Nessuna azione |

| `Core/Inc/main.h` | Modificato solo DENTRO USER CODE | 🟢 Nessuno (Sicuro) | Nessuna azione |

| `Core/Inc/Memory_operations.h` | File Custom (Non gestito da CubeMX) | 🟢 Nessuno (Sicuro) | Nessuna azione |

| `Core/Inc/callback_LPDMA.h` | File Custom (Non gestito da CubeMX) | 🟢 Nessuno (Sicuro) | Nessuna azione |

| `Core/Inc/stm32u5xx_hal_conf.h` | File Custom (Non gestito da CubeMX) | 🟢 Nessuno (Sicuro) | Nessuna azione |

| `Core/Src/Memory_operations.c` | File Custom (Non gestito da CubeMX) | 🟢 Nessuno (Sicuro) | Nessuna azione |

| `Core/Src/SPI_NAND.c` | File Custom (Non gestito da CubeMX) | 🟢 Nessuno (Sicuro) | Nessuna azione |

| `Core/Src/Spec_AS7341.c` | File Custom (Non gestito da CubeMX) | 🟢 Nessuno (Sicuro) | Nessuna azione |

| `Core/Src/callback_LPDMA.c` | File Custom (Non gestito da CubeMX) | 🟢 Nessuno (Sicuro) | Nessuna azione |


---

## 🔴 1. Dettaglio Modifiche Critiche Fuori USER CODE (Core)

Queste modifiche sono state fatte direttamente nel codice boilerplate generato da CubeMX. **CubeMX le eliminerà completamente** alla prossima generazione perché non sono protette dai tag `/* USER CODE BEGIN */`.


### `Core/Inc/rtc.h`

*Nuovo file generato da CubeMX (es. RTC). Le sue impostazioni dipendono dal file .ioc.*


> *Nuovo file generato da CubeMX. Assicurarsi che le impostazioni hardware nel file `.ioc` corrispondano.* 


### `Core/Inc/stm32u5xx_it.h`

*Modifiche manuali apportate alle parti di codice generato (boilerplate) gestite da CubeMX. Queste andranno perse alla prossima rigenerazione!*


```diff
@@ -46,6 +46,7 @@
 void OTG_FS_IRQHandler(void);
 void SPI3_IRQHandler(void);
 void MDF1_FLT0_IRQHandler(void);
+void LPDMA1_Channel0_IRQHandler(void);
 /* USER CODE BEGIN EFP */
 /* USER CODE END EFP */
 
```


### `Core/Src/lpdma.c`

*Modifiche manuali apportate alle parti di codice generato (boilerplate) gestite da CubeMX. Queste andranno perse alla prossima rigenerazione!*


```diff
@@ -17,6 +17,10 @@
 
   /* Peripheral clock enable */
   __HAL_RCC_LPDMA1_CLK_ENABLE();
+
+  /* LPDMA1 interrupt Init */
+    HAL_NVIC_SetPriority(LPDMA1_Channel0_IRQn, 1, 0);
+    HAL_NVIC_EnableIRQ(LPDMA1_Channel0_IRQn);
 
   /* USER CODE BEGIN LPDMA1_Init 1 */
   /* USER CODE END LPDMA1_Init 1 */
```


### `Core/Src/lptim.c`

*Modifiche manuali apportate alle parti di codice generato (boilerplate) gestite da CubeMX. Queste andranno perse alla prossima rigenerazione!*


```diff
@@ -21,9 +21,9 @@
   /* USER CODE END LPTIM1_Init 1 */
   hlptim1.Instance = LPTIM1;
   hlptim1.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
-  hlptim1.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV32;
+  hlptim1.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;
   hlptim1.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
-  hlptim1.Init.Period = 2047;
+  hlptim1.Init.Period = 31999;
   hlptim1.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
   hlptim1.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
   hlptim1.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
@@ -56,7 +56,7 @@
   /** Initializes the peripherals clock
   */
     PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPTIM1;
-    PeriphClkInit.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_LSE;
+    PeriphClkInit.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_LSI;
     if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
     {
       Error_Handler();
```


### `Core/Src/main.c`

*Modifiche manuali apportate alle parti di codice generato (boilerplate) gestite da CubeMX. Queste andranno perse alla prossima rigenerazione!*


```diff
@@ -7,6 +7,7 @@
 #include "lpdma.h"
 #include "lptim.h"
 #include "mdf.h"
+#include "rtc.h"
 #include "spi.h"
 #include "tim.h"
 #include "usart.h"
@@ -30,10 +31,6 @@
 /* USER CODE END PM */
 
 /* Private variables ---------------------------------------------------------*/
-
-I2C_HandleTypeDef hi2c3;
-DMA_HandleTypeDef handle_LPDMA1_Channel0;
-LPTIM_HandleTypeDef hlptim1;
 
 /* USER CODE BEGIN PV */
 /* USER CODE END PV */
@@ -82,6 +79,7 @@
   MX_LPTIM1_Init();
   MX_ICACHE_Init();
   MX_MDF1_Init();
+  MX_RTC_Init();
   MX_SPI2_Init();
   MX_SPI3_Init();
   MX_TIM2_Init();
@@ -114,16 +112,14 @@
     Error_Handler();
   }
 
-  /** Configure LSE Drive Capability
-  */
-  HAL_PWR_EnableBkUpAccess();
-  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
-
   /** Initializes the CPU, AHB and APB buses clocks
   */
-  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
+  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_HSI;
   RCC_OscInitStruct.HSEState = RCC_HSE_ON;
-  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
+  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
+  RCC_OscInitStruct.LSIDiv = RCC_LSI_DIV1;
+  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
+  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
   RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
   RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
   RCC_OscInitStruct.PLL.PLLMBOOST = RCC_PLLMBOOST_DIV2;
@@ -138,6 +134,9 @@
   {
     Error_Handler();
   }
+
+  /* Enable HSI in Stop mode (HSIKERON) so LPBAM can autonomously request it during Stop 2 sleep! */
+  __HAL_RCC_HSISTOP_ENABLE();
 
   /** Initializes the CPU, AHB and APB buses clocks
   */
```


### `Core/Src/rtc.c`

*Nuovo file generato da CubeMX (es. RTC). Le sue impostazioni dipendono dal file .ioc.*


> *Nuovo file generato da CubeMX. Assicurarsi che le impostazioni hardware nel file `.ioc` corrispondano.* 


### `Core/Src/stm32u5xx_it.c`

*Modifiche manuali apportate alle parti di codice generato (boilerplate) gestite da CubeMX. Queste andranno perse alla prossima rigenerazione!*


```diff
@@ -33,6 +33,7 @@
 /* USER CODE END 0 */
 
 /* External variables --------------------------------------------------------*/
+extern DMA_HandleTypeDef handle_LPDMA1_Channel0;
 extern MDF_HandleTypeDef MdfHandle0;
 extern SPI_HandleTypeDef hspi2;
 extern SPI_HandleTypeDef hspi3;
@@ -296,5 +297,17 @@
   /* USER CODE END MDF1_FLT0_IRQn 1 */
 }
 
+/**
+  * @brief This function handles LPDMA1 SmartRun Channel 0 global interrupt.
+  */
+void LPDMA1_Channel0_IRQHandler(void)
+{
+  /* USER CODE BEGIN LPDMA1_Channel0_IRQn 0 */
+  /* USER CODE END LPDMA1_Channel0_IRQn 0 */
+  HAL_DMA_IRQHandler(&handle_LPDMA1_Channel0);
+  /* USER CODE BEGIN LPDMA1_Channel0_IRQn 1 */
+  /* USER CODE END LPDMA1_Channel0_IRQn 1 */
+}
+
 /* USER CODE BEGIN 1 */
 /* USER CODE END 1 */
```



---

## 🔴 2. Dettaglio File e Modifiche LPBAM (Intera Cartella a Rischio)

La cartella `LPBAM` viene ricreata dall'LPBAM Utility di STM32CubeMX. Qualsiasi modifica manuale apportata a questi file (inclusi i file di build dei descrittori DMA) **verrà persa completamente** poiché lo strumento non supporta blocchi USER CODE affidabili in tutte le parti dei file.


### `LPBAM/I2C_Spec/lpbam_i2c_spec_config.c`

*File LPBAM modificato. Lo strumento LPBAM di CubeMX rigenera completamente questi sorgenti da zero, perdendo ogni modifica manuale.*


```diff
@@ -59,16 +59,10 @@
   }
   }
 
-  /** Configure LSE Drive Capability
-  */
-  HAL_PWR_EnableBkUpAccess();
-  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
-
   /** Initializes the CPU, AHB and APB buses clocks
   */
   RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI
-                              |RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
-  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
+                              |RCC_OSCILLATORTYPE_MSI;
   RCC_OscInitStruct.HSIState = RCC_HSI_ON;
   RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
   RCC_OscInitStruct.LSIState = RCC_LSI_ON;
```


### `LPBAM/I2C_Spec/lpbam_i2c_spec_i2c_rx_build.c`

*File LPBAM modificato. Lo strumento LPBAM di CubeMX rigenera completamente questi sorgenti da zero, perdendo ogni modifica manuale.*


```diff
@@ -8,27 +8,99 @@
 /* USER CODE BEGIN I2C_Spec_I2C_RX_Descs 0 */
 /* USER CODE END I2C_Spec_I2C_RX_Descs 0 */
 
-/* USER CODE BEGIN I2CAcq_Q_Master_Transmit_Data_1_Desc[1] */
-/* USER CODE END I2CAcq_Q_Master_Transmit_Data_1_Desc[1] */
-static LPBAM_I2C_MasterTxDataDesc_t I2CAcq_Q_Master_Transmit_Data_1_Desc[1];
-
-/* USER CODE BEGIN I2CAcq_Q_Master_Receive_Data_1_Desc[1] */
-/* USER CODE END I2CAcq_Q_Master_Receive_Data_1_Desc[1] */
-static LPBAM_I2C_MasterRxDataDesc_t I2CAcq_Q_Master_Receive_Data_1_Desc[1];
+/* USER CODE BEGIN Blue_Flick_Acq_Q_Master_Transmit_CH_REG1_Desc[1] */
+/* USER CODE END Blue_Flick_Acq_Q_Master_Transmit_CH_REG1_Desc[1] */
+static LPBAM_I2C_MasterTxDataDesc_t Blue_Flick_Acq_Q_Master_Transmit_CH_REG1_Desc[1] __attribute__((section(".sram4_retention")));
+
+/* USER CODE BEGIN Blue_Flick_Acq_Q_Master_Receive_Data_1_Desc[1] */
+/* USER CODE END Blue_Flick_Acq_Q_Master_Receive_Data_1_Desc[1] */
+static LPBAM_I2C_MasterRxDataDesc_t Blue_Flick_Acq_Q_Master_Receive_Data_1_Desc[1] __attribute__((section(".sram4_retention")));
+
+/* USER CODE BEGIN Blue_Flick_Acq_Q_Master_Transmit_Flick_REG1_Desc[1] */
+/* USER CODE END Blue_Flick_Acq_Q_Master_Transmit_Flick_REG1_Desc[1] */
+static LPBAM_I2C_MasterTxDataDesc_t Blue_Flick_Acq_Q_Master_Transmit_Flick_REG1_Desc[1] __attribute__((section(".sram4_retention")));
+
+/* USER CODE BEGIN Blue_Flick_Acq_Q_Master_Receive_Data_8_Desc[1] */
+/* USER CODE END Blue_Flick_Acq_Q_Master_Receive_Data_8_Desc[1] */
+static LPBAM_I2C_MasterRxDataDesc_t Blue_Flick_Acq_Q_Master_Receive_Data_8_Desc[1] __attribute__((section(".sram4_retention")));
+
+/* USER CODE BEGIN Blue_Flick_Acq_Q_Master_Transmit_CH_REG2_Desc[1] */
+/* USER CODE END Blue_Flick_Acq_Q_Master_Transmit_CH_REG2_Desc[1] */
+static LPBAM_I2C_MasterTxDataDesc_t Blue_Flick_Acq_Q_Master_Transmit_CH_REG2_Desc[1] __attribute__((section(".sram4_retention")));
+
+/* USER CODE BEGIN Blue_Flick_Acq_Q_Master_Receive_Data_2_Desc[1] */
+/* USER CODE END Blue_Flick_Acq_Q_Master_Receive_Data_2_Desc[1] */
+static LPBAM_I2C_MasterRxDataDesc_t Blue_Flick_Acq_Q_Master_Receive_Data_2_Desc[1] __attribute__((section(".sram4_retention")));
+
+/* USER CODE BEGIN Blue_Flick_Acq_Q_Master_Transmit_Flick_REG2_Desc[1] */
+/* USER CODE END Blue_Flick_Acq_Q_Master_Transmit_Flick_REG2_Desc[1] */
+static LPBAM_I2C_MasterTxDataDesc_t Blue_Flick_Acq_Q_Master_Transmit_Flick_REG2_Desc[1] __attribute__((section(".sram4_retention")));
+
+/* USER CODE BEGIN Blue_Flick_Acq_Q_Master_Receive_Data_3_Desc[1] */
+/* USER CODE END Blue_Flick_Acq_Q_Master_Receive_Data_3_Desc[1] */
+static LPBAM_I2C_MasterRxDataDesc_t Blue_Flick_Acq_Q_Master_Receive_Data_3_Desc[1] __attribute__((section(".sram4_retention")));
+
+/* USER CODE BEGIN Blue_Flick_Acq_Q_Master_Transmit_CH_REG3_Desc[1] */
+/* USER CODE END Blue_Flick_Acq_Q_Master_Transmit_CH_REG3_Desc[1] */
+static LPBAM_I2C_MasterTxDataDesc_t Blue_Flick_Acq_Q_Master_Transmit_CH_REG3_Desc[1] __attribute__((section(".sram4_retention")));
+
+/* USER CODE BEGIN Blue_Flick_Acq_Q_Master_Receive_Data_4_Desc[1] */
+/* USER CODE END Blue_Flick_Acq_Q_Master_Receive_Data_4_Desc[1] */
+static LPBAM_I2C_MasterRxDataDesc_t Blue_Flick_Acq_Q_Master_Receive_Data_4_Desc[1] __attribute__((section(".sram4_retention")));
+
+/* USER CODE BEGIN Blue_Flick_Acq_Q_Master_Transmit_Flick_REG3_Desc[1] */
+/* USER CODE END Blue_Flick_Acq_Q_Master_Transmit_Flick_REG3_Desc[1] */
+static LPBAM_I2C_MasterTxDataDesc_t Blue_Flick_Acq_Q_Master_Transmit_Flick_REG3_Desc[1] __attribute__((section(".sram4_retention")));
+
+/* USER CODE BEGIN Blue_Flick_Acq_Q_Master_Receive_Data_5_Desc[1] */
+/* USER CODE END Blue_Flick_Acq_Q_Master_Receive_Data_5_Desc[1] */
+static LPBAM_I2C_MasterRxDataDesc_t Blue_Flick_Acq_Q_Master_Receive_Data_5_Desc[1] __attribute__((section(".sram4_retention")));
+
+/* USER CODE BEGIN Blue_Flick_Acq_Q_Master_Transmit_CH_REG4_Desc[1] */
+/* USER CODE END Blue_Flick_Acq_Q_Master_Transmit_CH_REG4_Desc[1] */
+static LPBAM_I2C_MasterTxDataDesc_t Blue_Flick_Acq_Q_Master_Transmit_CH_REG4_Desc[1] __attribute__((section(".sram4_retention")));
+
+/* USER CODE BEGIN Blue_Flick_Acq_Q_Master_Receive_Data_6_Desc[1] */
+/* USER CODE END Blue_Flick_Acq_Q_Master_Receive_Data_6_Desc[1] */
+static LPBAM_I2C_MasterRxDataDesc_t Blue_Flick_Acq_Q_Master_Receive_Data_6_Desc[1] __attribute__((section(".sram4_retention")));
+
+/* USER CODE BEGIN Blue_Flick_Acq_Q_Master_Transmit_Flick_REG4_Desc[1] */
+/* USER CODE END Blue_Flick_Acq_Q_Master_Transmit_Flick_REG4_Desc[1] */
+static LPBAM_I2C_MasterTxDataDesc_t Blue_Flick_Acq_Q_Master_Transmit_Flick_REG4_Desc[1] __attribute__((section(".sram4_retention")));
+
+/* USER CODE BEGIN Blue_Flick_Acq_Q_Master_Receive_Data_7_Desc[1] */
+/* USER CODE END Blue_Flick_Acq_Q_Master_Receive_Data_7_Desc[1] */
+static LPBAM_I2C_MasterRxDataDesc_t Blue_Flick_Acq_Q_Master_Receive_Data_7_Desc[1] __attribute__((section(".sram4_retention")));
+
+/* USER CODE BEGIN Blue_Flick_Acq_Q_Master_Transmit_CH_REG5_Desc[1] */
+/* USER CODE END Blue_Flick_Acq_Q_Master_Transmit_CH_REG5_Desc[1] */
+static LPBAM_I2C_MasterTxDataDesc_t Blue_Flick_Acq_Q_Master_Transmit_CH_REG5_Desc[1] __attribute__((section(".sram4_retention")));
+
+/* USER CODE BEGIN Blue_Flick_Acq_Q_Master_Receive_Data_17_Desc[1] */
+/* USER CODE END Blue_Flick_Acq_Q_Master_Receive_Data_17_Desc[1] */
+static LPBAM_I2C_MasterRxDataDesc_t Blue_Flick_Acq_Q_Master_Receive_Data_17_Desc[1] __attribute__((section(".sram4_retention")));
+
+/* USER CODE BEGIN Blue_Flick_Acq_Q_Master_Transmit_Flick_REG5_Desc[1] */
+/* USER CODE END Blue_Flick_Acq_Q_Master_Transmit_Flick_REG5_Desc[1] */
+static LPBAM_I2C_MasterTxDataDesc_t Blue_Flick_Acq_Q_Master_Transmit_Flick_REG5_Desc[1] __attribute__((section(".sram4_retention")));
+
+/* USER CODE BEGIN Blue_Flick_Acq_Q_Final_Receive_Desc[1] */
+/* USER CODE END Blue_Flick_Acq_Q_Final_Receive_Desc[1] */
+static LPBAM_I2C_MasterRxFullDesc_t Blue_Flick_Acq_Q_Final_Receive_Desc[1] __attribute__((section(".sram4_retention")));
 
 /* USER CODE BEGIN I2C_Spec_I2C_RX_Descs 1 */
 /* USER CODE END I2C_Spec_I2C_RX_Descs 1 */
 
 /* Exported variables ------------------------------------------------------------------------------------------------*/
 /* LPBAM queues declaration */
-DMA_QListTypeDef I2CAcq_Q;
+DMA_QListTypeDef Blue_Flick_Acq_Q __attribute__((section(".sram4_retention")));
 
 /* External variables ------------------------------------------------------------------------------------------------*/
 /* USER CODE BEGIN EV */
 /* USER CODE END EV */
 
 /* Private function prototypes ---------------------------------------------------------------------------------------*/
-static void MX_I2CAcq_Q_Build(void);
+static void MX_Blue_Flick_Acq_Q_Build(void);
 
 /* USER CODE BEGIN PFP */
 /* USER CODE END PFP */
@@ -44,8 +116,8 @@
   /* USER CODE BEGIN I2C_Spec_I2C_RX_Build 0 */
   /* USER CODE END I2C_Spec_I2C_RX_Build 0 */
 
-  /* LPBAM build I2CAcq queue */
-  MX_I2CAcq_Q_Build();
+  /* LPBAM build Blue_Flick_Acq queue */
+  MX_Blue_Flick_Acq_Q_Build();
 
   /* USER CODE BEGIN I2C_Spec_I2C_RX_Build 1 */
   /* USER CODE END I2C_Spec_I2C_RX_Build 1 */
@@ -54,31 +126,33 @@
 /* Private functions -------------------------------------------------------------------------------------------------*/
 
 /**
-  * @brief  I2C_Spec application I2C_RX scenario I2CAcq queue build
+  * @brief  I2C_Spec application I2C_RX scenario Blue_Flick_Acq queue build
   * @param  None
   * @retval None
   */
-static void MX_I2CAcq_Q_Build(void)
+static void MX_Blue_Flick_Acq_Q_Build(void)
 {
   /* LPBAM build variable */
   LPBAM_DMAListInfo_t pDMAListInfo_I2C = {0};
   LPBAM_I2C_DataAdvConf_t pTxData_I2C = {0};
   LPBAM_COMMON_TrigAdvConf_t pTrigConfig_I2C = {0};
   LPBAM_I2C_DataAdvConf_t pRxData_I2C = {0};
+  LPBAM_I2C_FullAdvConf_t pRxFull_I2C = {0};
   uint32_t data_size = 0;
   uint32_t tmp_data_size = 0;
   uint32_t transfer_idx = 0;
 
   /**
-    * I2CAcq queue Master_Transmit_Data_1 build
+    * Blue_Flick_Acq queue Master_Transmit_CH_REG1 build
     */
    pDMAListInfo_I2C.QueueType= LPBAM_LINEAR_ADDRESSING_Q;
    pDMAListInfo_I2C.pInstance= LPDMA1;
   pTxData_I2C.AutoModeConf.TriggerState = LPBAM_I2C_AUTO_MODE_DISABLE;
    pTxData_I2C.AddressingMode = LPBAM_I2C_ADDRESSINGMODE_7BIT;
    pTxData_I2C.SequenceNumber = 1;
-   pTxData_I2C.pData = (uint8_t*)&AS7341_start_register[DataBufferOffset];
-   pTxData_I2C.DevAddress = 0x95;
+   pTxData_I2C.pData = (uint8_t*)&AS7341_start_register[0];
+   //0x39 shiftato a sinistra di 1 bit per il formato 7-bit (0x72) e con bit R/W=0 per la scrittura
+   pTxData_I2C.DevAddress = 0x72;
    pTxData_I2C.Size = 1;
   /* Set transfer parameters */
   data_size = pTxData_I2C.Size;
@@ -88,7 +162,7 @@
   /* Repeat inserting I2C master Tx data queue until completing all data */
   while (data_size != 0U)
   {
-    if (ADV_LPBAM_I2C_MasterTx_SetDataQ(I2C3, &pDMAListInfo_I2C, &pTxData_I2C, &I2CAcq_Q_Master_Transmit_Data_1_Desc[transfer_idx], &I2CAcq_Q) != LPBAM_OK)
+    if (ADV_LPBAM_I2C_MasterTx_SetDataQ(I2C3, &pDMAListInfo_I2C, &pTxData_I2C, &Blue_Flick_Acq_Q_Master_Transmit_CH_REG1_Desc[transfer_idx], &Blue_Flick_Acq_Q) != LPBAM_OK)
     {
       Error_Handler();
     }
@@ -110,19 +184,19 @@
   pTrigConfig_I2C.TriggerConfig.TriggerMode = LPBAM_DMA_TRIGM_BLOCK_TRANSFER;
   pTrigConfig_I2C.TriggerConfig.TriggerPolarity = LPBAM_DMA_TRIG_POLARITY_RISING;
   pTrigConfig_I2C.TriggerConfig.TriggerSelection = LPBAM_LPDMA1_TRIGGER_LPTIM1_CH1;
-  if (ADV_LPBAM_Q_SetTriggerConfig (&pTrigConfig_I2C, LPBAM_I2C_MASTERTX_DATAQ_CONFIG_NODE, &I2CAcq_Q_Master_Transmit_Data_1_Desc) != LPBAM_OK)
-  {
-    Error_Handler();
-  }
-
-  /**
-    * I2CAcq queue Master_Receive_Data_1 build
+  if (ADV_LPBAM_Q_SetTriggerConfig (&pTrigConfig_I2C, LPBAM_I2C_MASTERTX_DATAQ_CONFIG_NODE, &Blue_Flick_Acq_Q_Master_Transmit_CH_REG1_Desc) != LPBAM_OK)
+  {
+    Error_Handler();
+  }
+
+  /**
+    * Blue_Flick_Acq queue Master_Receive_Data_1 build
     */
   pRxData_I2C.AutoModeConf.TriggerState = LPBAM_I2C_AUTO_MODE_DISABLE;
    pRxData_I2C.AddressingMode = LPBAM_I2C_ADDRESSINGMODE_7BIT;
    pRxData_I2C.SequenceNumber = 1;
-   pRxData_I2C.pData = (uint8_t*)&AS7341_Rx_Buffer[DataBufferOffset];
-   pRxData_I2C.DevAddress = 0x39;
+   pRxData_I2C.pData = (uint8_t*)&AS7341_Rx_Buffer[0];
+   pRxData_I2C.DevAddress = 0x72;
    pRxData_I2C.Size = 12;
   /* Set transfer parameters */
   data_size = pRxData_I2C.Size;
@@ -132,7 +206,7 @@
   /* Repeat inserting I2C master Tx data queue until completing all data */
   while (data_size != 0U)
   {
-    if (ADV_LPBAM_I2C_MasterRx_SetDataQ(I2C3, &pDMAListInfo_I2C, &pRxData_I2C, &I2CAcq_Q_Master_Receive_Data_1_Desc[transfer_idx], &I2CAcq_Q) != LPBAM_OK)
+    if (ADV_LPBAM_I2C_MasterRx_SetDataQ(I2C3, &pDMAListInfo_I2C, &pRxData_I2C, &Blue_Flick_Acq_Q_Master_Receive_Data_1_Desc[transfer_idx], &Blue_Flick_Acq_Q) != LPBAM_OK)
     {
       Error_Handler();
     }
@@ -151,7 +225,681 @@
     pRxData_I2C.Size = data_size;
   }
   pRxData_I2C.Size = tmp_data_size;
-  if (ADV_LPBAM_Q_SetTriggerConfig (&pTrigConfig_I2C, LPBAM_I2C_MASTERRX_DATAQ_CONFIG_NODE, &I2CAcq_Q_Master_Receive_Data_1_Desc) != LPBAM_OK)
+  if (ADV_LPBAM_Q_SetTriggerConfig (&pTrigConfig_I2C, LPBAM_I2C_MASTERRX_DATAQ_CONFIG_NODE, &Blue_Flick_Acq_Q_Master_Receive_Data_1_Desc) != LPBAM_OK)
+  {
+    Error_Handler();
+  }
+
+  /**
+    * Blue_Flick_Acq queue Master_Transmit_Flick_REG1 build
+    */
+   pTxData_I2C.pData = (uint8_t*)&Flicker_REG[0];
+   pTxData_I2C.Size = 1;
+  /* Set transfer parameters */
+  data_size = pTxData_I2C.Size;
+  tmp_data_size = pTxData_I2C.Size;
+  transfer_idx = 0;
+
+  /* Repeat inserting I2C master Tx data queue until completing all data */
+  while (data_size != 0U)
+  {
+    if (ADV_LPBAM_I2C_MasterTx_SetDataQ(I2C3, &pDMAListInfo_I2C, &pTxData_I2C, &Blue_Flick_Acq_Q_Master_Transmit_Flick_REG1_Desc[transfer_idx], &Blue_Flick_Acq_Q) != LPBAM_OK)
+    {
+      Error_Handler();
+    }
+
+    transfer_idx++;
+
+    if (data_size > LPBAM_I2C_MAX_DATA_SIZE)
+    {
+      data_size -= LPBAM_I2C_MAX_DATA_SIZE;
+    }
+    else
+    {
+      data_size = 0U;
+    }
+
+    pTxData_I2C.Size = data_size;
+  }
+  pTxData_I2C.Size = tmp_data_size;
+  if (ADV_LPBAM_Q_SetTriggerConfig (&pTrigConfig_I2C, LPBAM_I2C_MASTERTX_DATAQ_CONFIG_NODE, &Blue_Flick_Acq_Q_Master_Transmit_Flick_REG1_Desc) != LPBAM_OK)
+  {
+    Error_Handler();
+  }
+
+  /**
+    * Blue_Flick_Acq queue Master_Receive_Data_8 build
+    */
+   pRxData_I2C.pData = (uint8_t*)&Flicker_buffer[0];
+   pRxData_I2C.DevAddress = 0x72;
+   pRxData_I2C.Size = 1;
+  /* Set transfer parameters */
+  data_size = pRxData_I2C.Size;
+  tmp_data_size = pRxData_I2C.Size;
+  transfer_idx = 0;
+
+  /* Repeat inserting I2C master Tx data queue until completing all data */
+  while (data_size != 0U)
+  {
+    if (ADV_LPBAM_I2C_MasterRx_SetDataQ(I2C3, &pDMAListInfo_I2C, &pRxData_I2C, &Blue_Flick_Acq_Q_Master_Receive_Data_8_Desc[transfer_idx], &Blue_Flick_Acq_Q) != LPBAM_OK)
+    {
+      Error_Handler();
+    }
+
+    transfer_idx++;
+
+    if (data_size > LPBAM_I2C_MAX_DATA_SIZE)
+    {
+      data_size -= LPBAM_I2C_MAX_DATA_SIZE;
+    }
+    else
+    {
+      data_size = 0U;
+    }
+
+    pRxData_I2C.Size = data_size;
+  }
+  pRxData_I2C.Size = tmp_data_size;
+  if (ADV_LPBAM_Q_SetTriggerConfig (&pTrigConfig_I2C, LPBAM_I2C_MASTERRX_DATAQ_CONFIG_NODE, &Blue_Flick_Acq_Q_Master_Receive_Data_8_Desc) != LPBAM_OK)
+  {
+    Error_Handler();
+  }
+
+  /**
+    * Blue_Flick_Acq queue Master_Transmit_CH_REG2 build
+    */
+   pTxData_I2C.pData = (uint8_t*)&AS7341_start_register[0];
+   pTxData_I2C.Size = 1;
+  /* Set transfer parameters */
+  data_size = pTxData_I2C.Size;
+  tmp_data_size = pTxData_I2C.Size;
+  transfer_idx = 0;
+
+  /* Repeat inserting I2C master Tx data queue until completing all data */
+  while (data_size != 0U)
+  {
+    if (ADV_LPBAM_I2C_MasterTx_SetDataQ(I2C3, &pDMAListInfo_I2C, &pTxData_I2C, &Blue_Flick_Acq_Q_Master_Transmit_CH_REG2_Desc[transfer_idx], &Blue_Flick_Acq_Q) != LPBAM_OK)
+    {
+      Error_Handler();
+    }
+
+    transfer_idx++;
+
+    if (data_size > LPBAM_I2C_MAX_DATA_SIZE)
+    {
+      data_size -= LPBAM_I2C_MAX_DATA_SIZE;
+    }
+    else
+    {
+      data_size = 0U;
+    }
+
+    pTxData_I2C.Size = data_size;
+  }
+  pTxData_I2C.Size = tmp_data_size;
+  if (ADV_LPBAM_Q_SetTriggerConfig (&pTrigConfig_I2C, LPBAM_I2C_MASTERTX_DATAQ_CONFIG_NODE, &Blue_Flick_Acq_Q_Master_Transmit_CH_REG2_Desc) != LPBAM_OK)
+  {
+    Error_Handler();
+  }
+
+  /**
+    * Blue_Flick_Acq queue Master_Receive_Data_2 build
+    */
+   pRxData_I2C.pData = (uint8_t*)&AS7341_Rx_Buffer[12];
+   pRxData_I2C.DevAddress = 0x72;
+   pRxData_I2C.Size = 12;
+  /* Set transfer parameters */
+  data_size = pRxData_I2C.Size;
+  tmp_data_size = pRxData_I2C.Size;
+  transfer_idx = 0;
+
+  /* Repeat inserting I2C master Tx data queue until completing all data */
+  while (data_size != 0U)
+  {
+    if (ADV_LPBAM_I2C_MasterRx_SetDataQ(I2C3, &pDMAListInfo_I2C, &pRxData_I2C, &Blue_Flick_Acq_Q_Master_Receive_Data_2_Desc[transfer_idx], &Blue_Flick_Acq_Q) != LPBAM_OK)
+    {
+      Error_Handler();
+    }
+
+    transfer_idx++;
+
+    if (data_size > LPBAM_I2C_MAX_DATA_SIZE)
+    {
+      data_size -= LPBAM_I2C_MAX_DATA_SIZE;
+    }
+    else
+    {
+      data_size = 0U;
+    }
+
+    pRxData_I2C.Size = data_size;
+  }
+  pRxData_I2C.Size = tmp_data_size;
+  if (ADV_LPBAM_Q_SetTriggerConfig (&pTrigConfig_I2C, LPBAM_I2C_MASTERRX_DATAQ_CONFIG_NODE, &Blue_Flick_Acq_Q_Master_Receive_Data_2_Desc) != LPBAM_OK)
+  {
+    Error_Handler();
+  }
+
+  /**
+    * Blue_Flick_Acq queue Master_Transmit_Flick_REG2 build
+    */
+   pTxData_I2C.pData = (uint8_t*)&Flicker_REG[0];
+   pTxData_I2C.Size = 1;
+  /* Set transfer parameters */
+  data_size = pTxData_I2C.Size;
+  tmp_data_size = pTxData_I2C.Size;
+  transfer_idx = 0;
+
+  /* Repeat inserting I2C master Tx data queue until completing all data */
+  while (data_size != 0U)
+  {
+    if (ADV_LPBAM_I2C_MasterTx_SetDataQ(I2C3, &pDMAListInfo_I2C, &pTxData_I2C, &Blue_Flick_Acq_Q_Master_Transmit_Flick_REG2_Desc[transfer_idx], &Blue_Flick_Acq_Q) != LPBAM_OK)
+    {
+      Error_Handler();
+    }
+
+    transfer_idx++;
+
+    if (data_size > LPBAM_I2C_MAX_DATA_SIZE)
+    {
+      data_size -= LPBAM_I2C_MAX_DATA_SIZE;
+    }
+    else
+    {
+      data_size = 0U;
+    }
+
+    pTxData_I2C.Size = data_size;
+  }
+  pTxData_I2C.Size = tmp_data_size;
+  if (ADV_LPBAM_Q_SetTriggerConfig (&pTrigConfig_I2C, LPBAM_I2C_MASTERTX_DATAQ_CONFIG_NODE, &Blue_Flick_Acq_Q_Master_Transmit_Flick_REG2_Desc) != LPBAM_OK)
+  {
+    Error_Handler();
+  }
+
+  /**
+    * Blue_Flick_Acq queue Master_Receive_Data_3 build
+    */
+   pRxData_I2C.pData = (uint8_t*)&Flicker_buffer[1];
+   pRxData_I2C.Size = 1;
+  /* Set transfer parameters */
+  data_size = pRxData_I2C.Size;
+  tmp_data_size = pRxData_I2C.Size;
+  transfer_idx = 0;
+
+  /* Repeat inserting I2C master Tx data queue until completing all data */
+  while (data_size != 0U)
+  {
+    if (ADV_LPBAM_I2C_MasterRx_SetDataQ(I2C3, &pDMAListInfo_I2C, &pRxData_I2C, &Blue_Flick_Acq_Q_Master_Receive_Data_3_Desc[transfer_idx], &Blue_Flick_Acq_Q) != LPBAM_OK)
+    {
+      Error_Handler();
+    }
+
+    transfer_idx++;
+
+    if (data_size > LPBAM_I2C_MAX_DATA_SIZE)
+    {
+      data_size -= LPBAM_I2C_MAX_DATA_SIZE;
+    }
+    else
+    {
+      data_size = 0U;
+    }
+
+    pRxData_I2C.Size = data_size;
+  }
+  pRxData_I2C.Size = tmp_data_size;
+  if (ADV_LPBAM_Q_SetTriggerConfig (&pTrigConfig_I2C, LPBAM_I2C_MASTERRX_DATAQ_CONFIG_NODE, &Blue_Flick_Acq_Q_Master_Receive_Data_3_Desc) != LPBAM_OK)
+  {
+    Error_Handler();
+  }
+
+  /**
+    * Blue_Flick_Acq queue Master_Transmit_CH_REG3 build
+    */
+   pTxData_I2C.pData = (uint8_t*)&AS7341_start_register[0];
+   pTxData_I2C.Size = 1;
+  /* Set transfer parameters */
+  data_size = pTxData_I2C.Size;
+  tmp_data_size = pTxData_I2C.Size;
+  transfer_idx = 0;
+
+  /* Repeat inserting I2C master Tx data queue until completing all data */
+  while (data_size != 0U)
+  {
+    if (ADV_LPBAM_I2C_MasterTx_SetDataQ(I2C3, &pDMAListInfo_I2C, &pTxData_I2C, &Blue_Flick_Acq_Q_Master_Transmit_CH_REG3_Desc[transfer_idx], &Blue_Flick_Acq_Q) != LPBAM_OK)
+    {
+      Error_Handler();
+    }
+
+    transfer_idx++;
+
+    if (data_size > LPBAM_I2C_MAX_DATA_SIZE)
+    {
+      data_size -= LPBAM_I2C_MAX_DATA_SIZE;
+    }
+    else
+    {
+      data_size = 0U;
+    }
+
+    pTxData_I2C.Size = data_size;
+  }
+  pTxData_I2C.Size = tmp_data_size;
+  if (ADV_LPBAM_Q_SetTriggerConfig (&pTrigConfig_I2C, LPBAM_I2C_MASTERTX_DATAQ_CONFIG_NODE, &Blue_Flick_Acq_Q_Master_Transmit_CH_REG3_Desc) != LPBAM_OK)
+  {
+    Error_Handler();
+  }
+
+  /**
+    * Blue_Flick_Acq queue Master_Receive_Data_4 build
+    */
+   pRxData_I2C.pData = (uint8_t*)&AS7341_Rx_Buffer[24];
+   pRxData_I2C.Size = 12;
+  /* Set transfer parameters */
+  data_size = pRxData_I2C.Size;
+  tmp_data_size = pRxData_I2C.Size;
+  transfer_idx = 0;
+
+  /* Repeat inserting I2C master Tx data queue until completing all data */
+  while (data_size != 0U)
+  {
+    if (ADV_LPBAM_I2C_MasterRx_SetDataQ(I2C3, &pDMAListInfo_I2C, &pRxData_I2C, &Blue_Flick_Acq_Q_Master_Receive_Data_4_Desc[transfer_idx], &Blue_Flick_Acq_Q) != LPBAM_OK)
+    {
+      Error_Handler();
+    }
+
+    transfer_idx++;
+
+    if (data_size > LPBAM_I2C_MAX_DATA_SIZE)
+    {
+      data_size -= LPBAM_I2C_MAX_DATA_SIZE;
+    }
+    else
+    {
+      data_size = 0U;
+    }
+
+    pRxData_I2C.Size = data_size;
+  }
+  pRxData_I2C.Size = tmp_data_size;
+  if (ADV_LPBAM_Q_SetTriggerConfig (&pTrigConfig_I2C, LPBAM_I2C_MASTERRX_DATAQ_CONFIG_NODE, &Blue_Flick_Acq_Q_Master_Receive_Data_4_Desc) != LPBAM_OK)
+  {
+    Error_Handler();
+  }
+
+  /**
+    * Blue_Flick_Acq queue Master_Transmit_Flick_REG3 build
+    */
+   pTxData_I2C.pData = (uint8_t*)&Flicker_REG[0];
+   pTxData_I2C.Size = 1;
+  /* Set transfer parameters */
+  data_size = pTxData_I2C.Size;
+  tmp_data_size = pTxData_I2C.Size;
+  transfer_idx = 0;
+
+  /* Repeat inserting I2C master Tx data queue until completing all data */
+  while (data_size != 0U)
+  {
+    if (ADV_LPBAM_I2C_MasterTx_SetDataQ(I2C3, &pDMAListInfo_I2C, &pTxData_I2C, &Blue_Flick_Acq_Q_Master_Transmit_Flick_REG3_Desc[transfer_idx], &Blue_Flick_Acq_Q) != LPBAM_OK)
+    {
+      Error_Handler();
+    }
+
+    transfer_idx++;
+
+    if (data_size > LPBAM_I2C_MAX_DATA_SIZE)
+    {
+      data_size -= LPBAM_I2C_MAX_DATA_SIZE;
+    }
+    else
+    {
+      data_size = 0U;
+    }
+
+    pTxData_I2C.Size = data_size;
+  }
+  pTxData_I2C.Size = tmp_data_size;
+  if (ADV_LPBAM_Q_SetTriggerConfig (&pTrigConfig_I2C, LPBAM_I2C_MASTERTX_DATAQ_CONFIG_NODE, &Blue_Flick_Acq_Q_Master_Transmit_Flick_REG3_Desc) != LPBAM_OK)
+  {
+    Error_Handler();
+  }
+
+  /**
+    * Blue_Flick_Acq queue Master_Receive_Data_5 build
+    */
+   pRxData_I2C.pData = (uint8_t*)&Flicker_buffer[2];
+   pRxData_I2C.Size = 1;
+  /* Set transfer parameters */
+  data_size = pRxData_I2C.Size;
+  tmp_data_size = pRxData_I2C.Size;
+  transfer_idx = 0;
+
+  /* Repeat inserting I2C master Tx data queue until completing all data */
+  while (data_size != 0U)
+  {
+    if (ADV_LPBAM_I2C_MasterRx_SetDataQ(I2C3, &pDMAListInfo_I2C, &pRxData_I2C, &Blue_Flick_Acq_Q_Master_Receive_Data_5_Desc[transfer_idx], &Blue_Flick_Acq_Q) != LPBAM_OK)
+    {
+      Error_Handler();
+    }
+
+    transfer_idx++;
+
+    if (data_size > LPBAM_I2C_MAX_DATA_SIZE)
+    {
+      data_size -= LPBAM_I2C_MAX_DATA_SIZE;
+    }
+    else
+    {
+      data_size = 0U;
+    }
+
+    pRxData_I2C.Size = data_size;
+  }
+  pRxData_I2C.Size = tmp_data_size;
+  if (ADV_LPBAM_Q_SetTriggerConfig (&pTrigConfig_I2C, LPBAM_I2C_MASTERRX_DATAQ_CONFIG_NODE, &Blue_Flick_Acq_Q_Master_Receive_Data_5_Desc) != LPBAM_OK)
+  {
+    Error_Handler();
+  }
+
+  /**
+    * Blue_Flick_Acq queue Master_Transmit_CH_REG4 build
+    */
+   pTxData_I2C.pData = (uint8_t*)&AS7341_start_register[0];
+   pTxData_I2C.Size = 1;
+  /* Set transfer parameters */
+  data_size = pTxData_I2C.Size;
+  tmp_data_size = pTxData_I2C.Size;
+  transfer_idx = 0;
+
+  /* Repeat inserting I2C master Tx data queue until completing all data */
+  while (data_size != 0U)
+  {
+    if (ADV_LPBAM_I2C_MasterTx_SetDataQ(I2C3, &pDMAListInfo_I2C, &pTxData_I2C, &Blue_Flick_Acq_Q_Master_Transmit_CH_REG4_Desc[transfer_idx], &Blue_Flick_Acq_Q) != LPBAM_OK)
+    {
+      Error_Handler();
+    }
+
+    transfer_idx++;
+
+    if (data_size > LPBAM_I2C_MAX_DATA_SIZE)
+    {
+      data_size -= LPBAM_I2C_MAX_DATA_SIZE;
+    }
+    else
+    {
+      data_size = 0U;
+    }
+
+    pTxData_I2C.Size = data_size;
+  }
+  pTxData_I2C.Size = tmp_data_size;
+  if (ADV_LPBAM_Q_SetTriggerConfig (&pTrigConfig_I2C, LPBAM_I2C_MASTERTX_DATAQ_CONFIG_NODE, &Blue_Flick_Acq_Q_Master_Transmit_CH_REG4_Desc) != LPBAM_OK)
+  {
+    Error_Handler();
+  }
+
+  /**
+    * Blue_Flick_Acq queue Master_Receive_Data_6 build
+    */
+   pRxData_I2C.pData = (uint8_t*)&AS7341_Rx_Buffer[36];
+   pRxData_I2C.Size = 12;
+  /* Set transfer parameters */
+  data_size = pRxData_I2C.Size;
+  tmp_data_size = pRxData_I2C.Size;
+  transfer_idx = 0;
+
+  /* Repeat inserting I2C master Tx data queue until completing all data */
+  while (data_size != 0U)
+  {
+    if (ADV_LPBAM_I2C_MasterRx_SetDataQ(I2C3, &pDMAListInfo_I2C, &pRxData_I2C, &Blue_Flick_Acq_Q_Master_Receive_Data_6_Desc[transfer_idx], &Blue_Flick_Acq_Q) != LPBAM_OK)
+    {
+      Error_Handler();
+    }
+
+    transfer_idx++;
+
+    if (data_size > LPBAM_I2C_MAX_DATA_SIZE)
+    {
+      data_size -= LPBAM_I2C_MAX_DATA_SIZE;
+    }
+    else
+    {
+      data_size = 0U;
+    }
+
+    pRxData_I2C.Size = data_size;
+  }
+  pRxData_I2C.Size = tmp_data_size;
+  if (ADV_LPBAM_Q_SetTriggerConfig (&pTrigConfig_I2C, LPBAM_I2C_MASTERRX_DATAQ_CONFIG_NODE, &Blue_Flick_Acq_Q_Master_Receive_Data_6_Desc) != LPBAM_OK)
+  {
+    Error_Handler();
+  }
+
+  /**
+    * Blue_Flick_Acq queue Master_Transmit_Flick_REG4 build
+    */
+   pTxData_I2C.pData = (uint8_t*)&Flicker_REG[0];
+   pTxData_I2C.Size = 1;
+  /* Set transfer parameters */
+  data_size = pTxData_I2C.Size;
+  tmp_data_size = pTxData_I2C.Size;
+  transfer_idx = 0;
+
+  /* Repeat inserting I2C master Tx data queue until completing all data */
+  while (data_size != 0U)
+  {
+    if (ADV_LPBAM_I2C_MasterTx_SetDataQ(I2C3, &pDMAListInfo_I2C, &pTxData_I2C, &Blue_Flick_Acq_Q_Master_Transmit_Flick_REG4_Desc[transfer_idx], &Blue_Flick_Acq_Q) != LPBAM_OK)
+    {
+      Error_Handler();
+    }
+
+    transfer_idx++;
+
+    if (data_size > LPBAM_I2C_MAX_DATA_SIZE)
+    {
+      data_size -= LPBAM_I2C_MAX_DATA_SIZE;
+    }
+    else
+    {
+      data_size = 0U;
+    }
+
+    pTxData_I2C.Size = data_size;
+  }
+  pTxData_I2C.Size = tmp_data_size;
+  if (ADV_LPBAM_Q_SetTriggerConfig (&pTrigConfig_I2C, LPBAM_I2C_MASTERTX_DATAQ_CONFIG_NODE, &Blue_Flick_Acq_Q_Master_Transmit_Flick_REG4_Desc) != LPBAM_OK)
+  {
+    Error_Handler();
+  }
+
+  /**
+    * Blue_Flick_Acq queue Master_Receive_Data_7 build
+    */
+   pRxData_I2C.pData = (uint8_t*)&Flicker_buffer[3];
+   pRxData_I2C.Size = 1;
+  /* Set transfer parameters */
+  data_size = pRxData_I2C.Size;
+  tmp_data_size = pRxData_I2C.Size;
+  transfer_idx = 0;
+
+  /* Repeat inserting I2C master Tx data queue until completing all data */
+  while (data_size != 0U)
+  {
+    if (ADV_LPBAM_I2C_MasterRx_SetDataQ(I2C3, &pDMAListInfo_I2C, &pRxData_I2C, &Blue_Flick_Acq_Q_Master_Receive_Data_7_Desc[transfer_idx], &Blue_Flick_Acq_Q) != LPBAM_OK)
+    {
+      Error_Handler();
+    }
+
+    transfer_idx++;
+
+    if (data_size > LPBAM_I2C_MAX_DATA_SIZE)
+    {
+      data_size -= LPBAM_I2C_MAX_DATA_SIZE;
+    }
+    else
+    {
+      data_size = 0U;
+    }
+
+    pRxData_I2C.Size = data_size;
+  }
+  pRxData_I2C.Size = tmp_data_size;
+  if (ADV_LPBAM_Q_SetTriggerConfig (&pTrigConfig_I2C, LPBAM_I2C_MASTERRX_DATAQ_CONFIG_NODE, &Blue_Flick_Acq_Q_Master_Receive_Data_7_Desc) != LPBAM_OK)
+  {
+    Error_Handler();
+  }
+
+  /**
+    * Blue_Flick_Acq queue Master_Transmit_CH_REG5 build
+    */
+   pTxData_I2C.pData = (uint8_t*)&AS7341_start_register[0];
+   pTxData_I2C.Size = 1;
+  /* Set transfer parameters */
+  data_size = pTxData_I2C.Size;
+  tmp_data_size = pTxData_I2C.Size;
+  transfer_idx = 0;
+
+  /* Repeat inserting I2C master Tx data queue until completing all data */
+  while (data_size != 0U)
+  {
+    if (ADV_LPBAM_I2C_MasterTx_SetDataQ(I2C3, &pDMAListInfo_I2C, &pTxData_I2C, &Blue_Flick_Acq_Q_Master_Transmit_CH_REG5_Desc[transfer_idx], &Blue_Flick_Acq_Q) != LPBAM_OK)
+    {
+      Error_Handler();
+    }
+
+    transfer_idx++;
+
+    if (data_size > LPBAM_I2C_MAX_DATA_SIZE)
+    {
+      data_size -= LPBAM_I2C_MAX_DATA_SIZE;
+    }
+    else
+    {
+      data_size = 0U;
+    }
+
+    pTxData_I2C.Size = data_size;
+  }
+  pTxData_I2C.Size = tmp_data_size;
+  if (ADV_LPBAM_Q_SetTriggerConfig (&pTrigConfig_I2C, LPBAM_I2C_MASTERTX_DATAQ_CONFIG_NODE, &Blue_Flick_Acq_Q_Master_Transmit_CH_REG5_Desc) != LPBAM_OK)
+  {
+    Error_Handler();
+  }
+
+  /**
+    * Blue_Flick_Acq queue Master_Receive_Data_17 build
+    */
+   pRxData_I2C.pData = (uint8_t*)&AS7341_Rx_Buffer[48];
+   pRxData_I2C.Size = 12;
+  /* Set transfer parameters */
+  data_size = pRxData_I2C.Size;
+  tmp_data_size = pRxData_I2C.Size;
+  transfer_idx = 0;
+
+  /* Repeat inserting I2C master Tx data queue until completing all data */
+  while (data_size != 0U)
+  {
+    if (ADV_LPBAM_I2C_MasterRx_SetDataQ(I2C3, &pDMAListInfo_I2C, &pRxData_I2C, &Blue_Flick_Acq_Q_Master_Receive_Data_17_Desc[transfer_idx], &Blue_Flick_Acq_Q) != LPBAM_OK)
+    {
+      Error_Handler();
+    }
+
+    transfer_idx++;
+
+    if (data_size > LPBAM_I2C_MAX_DATA_SIZE)
+    {
+      data_size -= LPBAM_I2C_MAX_DATA_SIZE;
+    }
+    else
+    {
+      data_size = 0U;
+    }
+
+    pRxData_I2C.Size = data_size;
+  }
+  pRxData_I2C.Size = tmp_data_size;
+  if (ADV_LPBAM_Q_SetTriggerConfig (&pTrigConfig_I2C, LPBAM_I2C_MASTERRX_DATAQ_CONFIG_NODE, &Blue_Flick_Acq_Q_Master_Receive_Data_17_Desc) != LPBAM_OK)
+  {
+    Error_Handler();
+  }
+
+  /**
+    * Blue_Flick_Acq queue Master_Transmit_Flick_REG5 build
+    */
+   pTxData_I2C.pData = (uint8_t*)&Flicker_REG[0];
+   pTxData_I2C.Size = 1;
+  /* Set transfer parameters */
+  data_size = pTxData_I2C.Size;
+  tmp_data_size = pTxData_I2C.Size;
+  transfer_idx = 0;
+
+  /* Repeat inserting I2C master Tx data queue until completing all data */
+  while (data_size != 0U)
+  {
+    if (ADV_LPBAM_I2C_MasterTx_SetDataQ(I2C3, &pDMAListInfo_I2C, &pTxData_I2C, &Blue_Flick_Acq_Q_Master_Transmit_Flick_REG5_Desc[transfer_idx], &Blue_Flick_Acq_Q) != LPBAM_OK)
+    {
+      Error_Handler();
+    }
+
+    transfer_idx++;
+
+    if (data_size > LPBAM_I2C_MAX_DATA_SIZE)
+    {
+      data_size -= LPBAM_I2C_MAX_DATA_SIZE;
+    }
+    else
+    {
+      data_size = 0U;
+    }
+
+    pTxData_I2C.Size = data_size;
+  }
+  pTxData_I2C.Size = tmp_data_size;
+  if (ADV_LPBAM_Q_SetTriggerConfig (&pTrigConfig_I2C, LPBAM_I2C_MASTERTX_DATAQ_CONFIG_NODE, &Blue_Flick_Acq_Q_Master_Transmit_Flick_REG5_Desc) != LPBAM_OK)
+  {
+    Error_Handler();
+  }
+
+  /**
+    * Blue_Flick_Acq queue Final_Receive build
+    */
+  pRxFull_I2C.AutoModeConf.TriggerState = LPBAM_I2C_AUTO_MODE_DISABLE;
+   pRxFull_I2C.Timing = 0x0;
+   pRxFull_I2C.WakeupIT = LPBAM_I2C_IT_TC;
+   pRxFull_I2C.AddressingMode = LPBAM_I2C_ADDRESSINGMODE_7BIT;
+   pRxFull_I2C.SequenceNumber = 1;
+   pRxFull_I2C.pData = (uint8_t*)&Flicker_buffer[4];
+   pRxFull_I2C.DevAddress = 0x72;
+   pRxFull_I2C.Size = 1;
+  /* Set transfer parameters */
+  data_size = pRxFull_I2C.Size;
+  tmp_data_size = pRxFull_I2C.Size;
+  transfer_idx = 0;
+
+  /* Repeat inserting I2C master Tx data queue until completing all data */
+  while (data_size != 0U)
+  {
+    if (ADV_LPBAM_I2C_MasterRx_SetFullQ(I2C3, &pDMAListInfo_I2C, &pRxFull_I2C, &Blue_Flick_Acq_Q_Final_Receive_Desc[transfer_idx], &Blue_Flick_Acq_Q) != LPBAM_OK)
+    {
+      Error_Handler();
+    }
+
+    transfer_idx++;
+
+    if (data_size > LPBAM_I2C_MAX_DATA_SIZE)
+    {
+      data_size -= LPBAM_I2C_MAX_DATA_SIZE;
+    }
+    else
+    {
+      data_size = 0U;
+    }
+
+    pRxFull_I2C.Size = data_size;
+  }
+  pRxFull_I2C.Size = tmp_data_size;
+  if (ADV_LPBAM_Q_SetTriggerConfig (&pTrigConfig_I2C, LPBAM_I2C_MASTERRX_FULLQ_CONFIG_NODE, &Blue_Flick_Acq_Q_Final_Receive_Desc) != LPBAM_OK)
   {
     Error_Handler();
   }
@@ -159,7 +907,7 @@
   /**
     * Set circular mode
     */
-  if (ADV_LPBAM_Q_SetCircularMode(&I2CAcq_Q_Master_Transmit_Data_1_Desc, LPBAM_I2C_MASTERTX_DATAQ_CONFIG_NODE, &I2CAcq_Q) != LPBAM_OK)
+  if (ADV_LPBAM_Q_SetCircularMode(&Blue_Flick_Acq_Q_Master_Transmit_CH_REG1_Desc, LPBAM_I2C_MASTERTX_DATAQ_CONFIG_NODE, &Blue_Flick_Acq_Q) != LPBAM_OK)
   {
     Error_Handler();
   }
```


### `LPBAM/I2C_Spec/lpbam_i2c_spec_i2c_rx_config.c`

*File LPBAM modificato. Lo strumento LPBAM di CubeMX rigenera completamente questi sorgenti da zero, perdendo ogni modifica manuale.*


```diff
@@ -8,7 +8,7 @@
 /* USER CODE END PTD */
 
 /* Private define --------------------------------------------------------------------------------------------------*/
-#define I2CACQ_Q_IDX (0U)
+#define BLUE_FLICK_ACQ_Q_IDX (0U)
 #define DMA_TIMEOUT_DURATION (0x1000U)
 
 /* USER CODE BEGIN PD */
@@ -25,7 +25,7 @@
 extern LPTIM_HandleTypeDef hlptim1;
 
 /* LPBAM queues declaration */
-extern DMA_QListTypeDef I2CAcq_Q;
+extern DMA_QListTypeDef Blue_Flick_Acq_Q;
 
 /* USER CODE BEGIN EV */
 /* USER CODE END EV */
@@ -52,8 +52,14 @@
 static void MX_AutonomousMode_DeInit(void);
 
 /* LPBAM queue linking/unlinking APIs */
-static void MX_I2CAcq_Q_Link(DMA_HandleTypeDef *hdma);
-static void MX_I2CAcq_Q_UnLink(DMA_HandleTypeDef *hdma);
+static void MX_Blue_Flick_Acq_Q_Link(DMA_HandleTypeDef *hdma);
+static void MX_Blue_Flick_Acq_Q_UnLink(DMA_HandleTypeDef *hdma);
+
+/* LPBAM DMA user callback APIs */
+static void MX_Blue_Flick_Acq_Q_DMA_TC_Callback(DMA_HandleTypeDef *hdma);
+static void MX_Blue_Flick_Acq_Q_DMA_Error_Callback(DMA_HandleTypeDef *hdma);
+/* LPBAM DMA NVIC API */
+static void MX_DMA_NVIC_Config(DMA_HandleTypeDef *hdma, uint32_t PreemptPriority, uint32_t SubPriority);
 
 /* USER CODE BEGIN PFP */
 /* USER CODE END PFP */
@@ -112,11 +118,11 @@
   /* USER CODE BEGIN I2C_Spec_I2C_RX_Link 0 */
   /* USER CODE END I2C_Spec_I2C_RX_Link 0 */
 
-  /* Link I2CAcq queue to DMA channel */
-  MX_I2CAcq_Q_Link(&hdma[I2CACQ_Q_IDX]);
-
-  /* USER CODE BEGIN LINK I2CACQ_Q_IDX */
-  /* USER CODE END LINK I2CACQ_Q_IDX */
+  /* Link Blue_Flick_Acq queue to DMA channel */
+  MX_Blue_Flick_Acq_Q_Link(&hdma[BLUE_FLICK_ACQ_Q_IDX]);
+
+  /* USER CODE BEGIN LINK BLUE_FLICK_ACQ_Q_IDX */
+  /* USER CODE END LINK BLUE_FLICK_ACQ_Q_IDX */
 
   /* USER CODE BEGIN I2C_Spec_I2C_RX_Link 1 */
   /* USER CODE END I2C_Spec_I2C_RX_Link 1 */
@@ -133,11 +139,11 @@
   /* USER CODE BEGIN I2C_Spec_I2C_RX_UnLink 0 */
   /* USER CODE END I2C_Spec_I2C_RX_UnLink 0 */
 
-  /* LPBAM unLink I2CAcq queue to DMA channel */
-  MX_I2CAcq_Q_UnLink(&hdma[I2CACQ_Q_IDX]);
-
-  /* USER CODE BEGIN UNLINK I2CACQ_Q_IDX */
-  /* USER CODE END UNLINK I2CACQ_Q_IDX */
+  /* LPBAM unLink Blue_Flick_Acq queue to DMA channel */
+  MX_Blue_Flick_Acq_Q_UnLink(&hdma[BLUE_FLICK_ACQ_Q_IDX]);
+
+  /* USER CODE BEGIN UNLINK BLUE_FLICK_ACQ_Q_IDX */
+  /* USER CODE END UNLINK BLUE_FLICK_ACQ_Q_IDX */
 
   /* USER CODE BEGIN I2C_Spec_I2C_RX_UnLink 1 */
   /* USER CODE END I2C_Spec_I2C_RX_UnLink 1 */
@@ -150,7 +156,7 @@
 void MX_I2C_Spec_I2C_RX_Start(DMA_HandleTypeDef *hdma)
 {
   /* LPBAM start DMA channel in linked-list mode */
-  if (HAL_DMAEx_List_Start(&hdma[I2CACQ_Q_IDX]) != HAL_OK)
+  if (HAL_DMAEx_List_Start(&hdma[BLUE_FLICK_ACQ_Q_IDX]) != HAL_OK)
   {
     Error_Handler();
   }
@@ -166,18 +172,18 @@
 void MX_I2C_Spec_I2C_RX_Stop(DMA_HandleTypeDef *hdma)
 {
   /* LPBAM stop DMA channel in linked-list mode */
-  if ((hdma[I2CACQ_Q_IDX].State == HAL_DMA_STATE_BUSY) && (hdma[I2CACQ_Q_IDX].LinkedListQueue->FirstCircularNode != 0U))
-  {
-    if (HAL_DMA_Abort(&hdma[I2CACQ_Q_IDX]) != HAL_OK)
+  if ((hdma[BLUE_FLICK_ACQ_Q_IDX].State == HAL_DMA_STATE_BUSY) && (hdma[BLUE_FLICK_ACQ_Q_IDX].LinkedListQueue->FirstCircularNode != 0U))
+  {
+    if (HAL_DMA_Abort(&hdma[BLUE_FLICK_ACQ_Q_IDX]) != HAL_OK)
     {
       Error_Handler();
     }
   }
 
   /* Check if DMA channel interrupt is enabled */
-  if ((hdma[I2CACQ_Q_IDX].State == HAL_DMA_STATE_BUSY) && (__HAL_DMA_GET_IT_SOURCE(&hdma[I2CACQ_Q_IDX], DMA_IT_TC) == 0U))
-  {
-    if (HAL_DMA_PollForTransfer(&hdma[I2CACQ_Q_IDX], HAL_DMA_FULL_TRANSFER, DMA_TIMEOUT_DURATION) != HAL_OK)
+  if ((hdma[BLUE_FLICK_ACQ_Q_IDX].State == HAL_DMA_STATE_BUSY) && (__HAL_DMA_GET_IT_SOURCE(&hdma[BLUE_FLICK_ACQ_Q_IDX], DMA_IT_TC) == 0U))
+  {
+    if (HAL_DMA_PollForTransfer(&hdma[BLUE_FLICK_ACQ_Q_IDX], HAL_DMA_FULL_TRANSFER, DMA_TIMEOUT_DURATION) != HAL_OK)
     {
       Error_Handler();
     }
@@ -487,9 +493,9 @@
   }
   hlptim1.Instance = LPTIM1;
   hlptim1.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
-  hlptim1.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV32;
+  hlptim1.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;
   hlptim1.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
-  hlptim1.Init.Period = 2047;
+  hlptim1.Init.Period = 31999;
   hlptim1.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
   hlptim1.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
   hlptim1.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
@@ -499,7 +505,7 @@
   {
     Error_Handler();
   }
-  sConfig1.Pulse = 0;
+  sConfig1.Pulse = 160;
   sConfig1.OCPolarity = LPTIM_OCPOLARITY_HIGH;
   if (HAL_LPTIM_OC_ConfigChannel(&hlptim1, &sConfig1, LPTIM_CHANNEL_1) != HAL_OK)
   {
@@ -559,7 +565,7 @@
   /** Initializes the peripherals clock
   */
     PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPTIM1;
-    PeriphClkInit.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_LSE;
+    PeriphClkInit.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_LSI;
     if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
     {
       Error_Handler();
@@ -591,10 +597,10 @@
 }
 
 /**
-  * @brief  I2CAcq queue link
-  * @retval None
-  */
-static void MX_I2CAcq_Q_Link(DMA_HandleTypeDef *hdma)
+  * @brief  Blue_Flick_Acq queue link
+  * @retval None
+  */
+static void MX_Blue_Flick_Acq_Q_Link(DMA_HandleTypeDef *hdma)
 {
   /* Enable LPDMA1 clock */
   __HAL_RCC_LPDMA1_CLK_ENABLE();
@@ -607,22 +613,38 @@
   {
     Error_Handler();
   }
-  if (HAL_DMAEx_List_LinkQ(hdma, &I2CAcq_Q) != HAL_OK)
-  {
-    Error_Handler();
-  }
+  if (HAL_DMAEx_List_LinkQ(hdma, &Blue_Flick_Acq_Q) != HAL_OK)
+  {
+    Error_Handler();
+  }
+  __HAL_DMA_ENABLE_IT(hdma, DMA_IT_TC | DMA_IT_DTE | DMA_IT_ULE | DMA_IT_USE);
 
   /* Register DMA channel error callbacks */
-}
-
-/**
-  * @brief  I2CAcq queue unlink
-  * @retval None
-  */
-static void MX_I2CAcq_Q_UnLink(DMA_HandleTypeDef *hdma)
-{
-  /* UnLink I2CAcq queue to DMA channel */
+  if (HAL_DMA_RegisterCallback(hdma, HAL_DMA_XFER_CPLT_CB_ID, MX_Blue_Flick_Acq_Q_DMA_TC_Callback) != HAL_OK)
+  {
+    Error_Handler();
+  }
+  if (HAL_DMA_RegisterCallback(hdma, HAL_DMA_XFER_ERROR_CB_ID, MX_Blue_Flick_Acq_Q_DMA_Error_Callback) != HAL_OK)
+  {
+    Error_Handler();
+  }
+  MX_DMA_NVIC_Config(hdma, 0, 0);
+}
+
+/**
+  * @brief  Blue_Flick_Acq queue unlink
+  * @retval None
+  */
+static void MX_Blue_Flick_Acq_Q_UnLink(DMA_HandleTypeDef *hdma)
+{
+  /* UnLink Blue_Flick_Acq queue to DMA channel */
   if (HAL_DMAEx_List_UnLinkQ(hdma) != HAL_OK)
+  {
+    Error_Handler();
+  }
+
+  /* Register DMA channel transfer complete callbacks */
+  if (HAL_DMA_UnRegisterCallback(hdma, HAL_DMA_XFER_CPLT_CB_ID) != HAL_OK)
   {
     Error_Handler();
   }
@@ -634,6 +656,71 @@
   }
 }
 
+static void MX_Blue_Flick_Acq_Q_DMA_Error_Callback(DMA_HandleTypeDef *hdma)
+{
+  printf("[DEBUG] DMA Transfer Error Callback Fired! ErrorCode = 0x%lx\n", hdma->ErrorCode);
+  if (hdma->Instance != NULL) {
+    DMA_Channel_TypeDef *ch = (DMA_Channel_TypeDef *)hdma->Instance;
+    printf("[DEBUG] LPDMA Regs - CSR: 0x%lx, CCR: 0x%lx, CLBAR: 0x%lx, CLLR: 0x%lx, CBR1: 0x%lx, CSAR: 0x%lx, CDAR: 0x%lx\n",
+           ch->CSR, ch->CCR, ch->CLBAR, ch->CLLR, ch->CBR1, ch->CSAR, ch->CDAR);
+  }
+  extern I2C_HandleTypeDef hi2c3;
+  if (hi2c3.Instance != NULL) {
+    printf("[DEBUG] I2C3 Regs - ISR: 0x%lx, CR1: 0x%lx, CR2: 0x%lx\n",
+           hi2c3.Instance->ISR, hi2c3.Instance->CR1, hi2c3.Instance->CR2);
+  }
+}
+
+/**
+  * @brief  Blue_Flick_Acq queue dma transfer complete callbacks
+  * @retval None
+  */
+static void MX_Blue_Flick_Acq_Q_DMA_TC_Callback(DMA_HandleTypeDef *hdma)
+{
+  /* USER CODE BEGIN Blue_Flick_Acq_DMA_TC_Callback */
+  /* USER CODE END Blue_Flick_Acq_DMA_TC_Callback */
+}
+
 /* USER CODE BEGIN I2C_Spec_I2C_RX_Config */
 /* USER CODE END I2C_Spec_I2C_RX_Config */
 
+/**
+  * @brief DMA channel NVIC configuration
+  * @retval None
+  */
+static void MX_DMA_NVIC_Config(DMA_HandleTypeDef *hdma, uint32_t PreemptPriority, uint32_t SubPriority)
+{
+  IRQn_Type irq = LPDMA1_Channel0_IRQn;
+
+  /* Check DMA channel instance */
+  switch ((uint32_t)hdma->Instance)
+  {
+    case (uint32_t)LPDMA1_Channel0: /* DMA channel_0 */
+    {
+      irq = LPDMA1_Channel0_IRQn;
+      break;
+    }
+
+    case (uint32_t)LPDMA1_Channel1: /* DMA channel_1 */
+    {
+      irq = LPDMA1_Channel1_IRQn;
+      break;
+    }
+
+    case (uint32_t)LPDMA1_Channel2: /* DMA channel_2 */
+    {
+      irq = LPDMA1_Channel2_IRQn;
+      break;
+    }
+
+    case (uint32_t)LPDMA1_Channel3: /* DMA channel_3 */
+    {
+      irq = LPDMA1_Channel3_IRQn;
+      break;
+    }
+  }
+
+  /* Enable NVIC for DMA channel */
+  HAL_NVIC_SetPriority(irq, PreemptPriority, SubPriority);
+  HAL_NVIC_EnableIRQ(irq);
+}
```



---

## 🟢 3. File Custom al Sicuro (Nessun Rischio)

I seguenti file sono stati scritti interamente da noi e non sono gestiti o sovrascritti da CubeMX. Rimangono intatti anche dopo una rigenerazione.


- `Core/Inc/Memory_operations.h`: *File sorgente custom modificato. Poiché non è gestito da CubeMX, non c'è alcun rischio di sovrascrittura.*

- `Core/Inc/callback_LPDMA.h`: *Nuovo file sorgente/header custom creato da noi. CubeMX non gestisce questo file, quindi è completamente al sicuro.*

- `Core/Inc/stm32u5xx_hal_conf.h`: *File sorgente custom modificato. Poiché non è gestito da CubeMX, non c'è alcun rischio di sovrascrittura.*

- `Core/Src/Memory_operations.c`: *File sorgente custom modificato. Poiché non è gestito da CubeMX, non c'è alcun rischio di sovrascrittura.*

- `Core/Src/SPI_NAND.c`: *File sorgente custom modificato. Poiché non è gestito da CubeMX, non c'è alcun rischio di sovrascrittura.*

- `Core/Src/Spec_AS7341.c`: *File sorgente custom modificato. Poiché non è gestito da CubeMX, non c'è alcun rischio di sovrascrittura.*

- `Core/Src/callback_LPDMA.c`: *Nuovo file sorgente/header custom creato da noi. CubeMX non gestisce questo file, quindi è completamente al sicuro.*


---

## 🟢 4. File Modificati Correttamente (Solo Dentro USER CODE)

Questi file contengono modifiche, ma tutte le modifiche si trovano all'interno delle sezioni protette `USER CODE BEGIN` / `USER CODE END` e non verranno sovrascritte da CubeMX.


- `Core/Inc/SPI.h`: *Tutte le modifiche apportate in questo file sono inserite correttamente all'interno dei blocchi `USER CODE BEGIN` / `USER CODE END` e sono quindi protette da sovrascritture.*

- `Core/Inc/main.h`: *Tutte le modifiche apportate in questo file sono inserite correttamente all'interno dei blocchi `USER CODE BEGIN` / `USER CODE END` e sono quindi protette da sovrascritture.*



---

### 💡 Suggerimenti per la rigenerazione di CubeMX

1. **Backup Preventivo**: Prima di premere 'Generate Code' in CubeMX, eseguire un commit di git (`git add . && git commit -m "pre-cubemx"`) per poter ripristinare o fare il merge delle modifiche perse.

2. **Configurazione LPTIM1**: Allineare le impostazioni di LPTIM1 all'interno dell'interfaccia grafica di CubeMX:

   - Impostare il Clock Source su **LSI**.

   - Impostare il Prescaler su **1**.

   - Impostare il Periodo su **31999**.

   - Impostare il Pulse del canale 1 su **160**.

3. **Gestione Interrupt LPDMA1**: Spostare l'abilitazione dell'interrupt di LPDMA1 e l'Handler `LPDMA1_Channel0_IRQHandler` in sezioni protette (ad esempio `main.c` all'interno di `USER CODE` o in un file di callback custom).
