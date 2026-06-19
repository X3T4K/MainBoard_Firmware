/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main application file for MainBoard_IMU_Logger project.
  ******************************************************************************
  * @functionality  : This firmware implements a complete Data Logger for the on board IMU.
  * @details        : The application operates using a State Machine triggered by a
  * single USER BUTTON. It performs three primary tasks:
  * 1. Real-time Acquisition: Reads Accelerometer/Gyroscope data
  * via I2C, synchronized by a TIM2 interrupt.
  * 2. Wireless Transmission: Sends data packets via Bluetooth Low Energy (BLE)
  * using the UART interface.
  * 3. Data Logging: Saves acquired data to NAND Flash memory.
  *
  * Saved data can be downloaded via a USB Virtual COM Port (VCP)
  * connection, also initiated by the USER BUTTON.
  *
  * @intended_use   : Starting template for Smart Wearables Course
  * exploring IMU interfacing, BLE communication, and memory management.
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpdma.h"
#include "i2c.h"
#include "icache.h"
#include "lpdma.h"
#include "lptim.h"
#include "mdf.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include "stdio.h"
#include "../../USB_Device/App/usb_device.h"
#include "SPI.h"
#include "SPI_NAND.h"
#include "Memory_operations.h"
#include "led_driver.h"
#include "imu_driver.h"
#include "bluetooth.h"
#include "Mic_IMP34DT05.h"
#include "Spec_AS7341.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

// Registro di partenza (Nota: meglio uint8_t per registri I2C)
//uint8_t AS7341_start_register = 0x95; 

// Buffer in SRAM4 per LPBAM/DMA
//uint8_t AS7341_Rx_Buffer[12] __attribute__((section(".sram4")));
//uint8_t IMP34DT05_Rx_Buffer[12] __attribute__((section(".sram4")));  

// Offset per la gestione dei dati
uint8_t DataBufferOffset = 0;
// --- State Machine ---
// The current state of the application. Initial state is IDLE.
volatile AppState current_state = STATE_IDLE;

// --- Global Flags and Variables ---
// Flag to indicate a USB connection event.
// Set to 1 when a USB connection is detected.
uint8_t usb_flag = 0;

// IMU data structures for accelerometer and gyroscope.
//static IMU_Data accelerometer_data;
//static IMU_Data gyroscope_data;

//uint8_t raw_accelerometer[6] = {0};
//uint8_t raw_gyroscope[6] = {0};

/// ----- NAND FLASH variables ----- ///

__attribute__((section(".sram4_retention"))) volatile uint8_t button_force_stop;
__attribute__((section(".sram4_retention"))) uint16_t total_good_blocks;
__attribute__((section(".sram4_retention"))) uint16_t session_start_block;
__attribute__((section(".sram4_retention"))) uint8_t session_start_page;
__attribute__((section(".sram4_retention"))) uint8_t session_active;

extern DMA_HandleTypeDef handle_GPDMA1_Channel0;

uint8_t NAND_packet[4096] = {0};
uint16_t sample = 0;
uint32_t global_sample_count = 0;
uint16_t blocco_scritto = 0;
uint8_t pagina_scritta=0;
uint16_t b = 1024; // start writing continuous audio data from middle of NAND

read_address_t blocco_peak;
read_address_t blocco_acq;
read_address_t blocco;
column_address_t colonna = 0;

uint16_t bad_blocks[2048]={-1}; // bad blocks array for writing/reading
uint8_t bad_blocks2[2048]={0}; // bad blocks array for erasing

uint8_t data_letto[4096] = {0};
int exit_flag = 0;

// Timestamp variables //
Time_Struct timestamp_monitoring;
Time_Struct timestamp_peak;
uint16_t tim = 0;

// Sound Acquisition variables //
static bool peak_detected = false;
static bool acquisition_active = false;
static float current_peak_dbspl = 0.0f;
static float current_acquisition_dbspl = 0.0f;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// Redirezione della printf verso ITM Stimulus Port 0
int _write(int file, char *ptr, int len) {
    for (int i = 0; i < len; i++) {
        // ITM_SendChar Ã¨ una funzione CMSIS che scrive direttamente 
        // nel registro hardware dell'unitÃ  di trace.
        ITM_SendChar(*ptr++);
    }
    return len;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_LPDMA1_Init();
  MX_GPDMA1_Init();
  MX_I2C3_Init();
  MX_LPTIM1_Init();
  MX_ICACHE_Init();
  MX_MDF1_Init();
  MX_SPI2_Init();
  MX_SPI3_Init();
  MX_TIM2_Init();
  MX_USART3_UART_Init();
  MX_TIM1_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
/* USER CODE BEGIN 2 */


  // Cold start initialization for SRAM4 retention variables (NOLOAD)
  button_force_stop = 0;
  total_good_blocks = 0;
  session_start_block = 0;
  session_start_page = 0;
  session_active = 0;

  printf("\r\n--- MainBoard IMU Logger ---\r\n");
  // Turn the RED LED on to indicate the start of the initialization process
  LED_On(LED_RED);

  // Initialize all hardware peripherals (ble, usb, nand flash, imu)
  BLE_Initialize();
  MX_USB_Device_Init();
  HAL_Delay(1000);


  spi_nand_init();
  find_bad_blocks(bad_blocks); // find bad_blocks and save them

  if(IMU_Init() == 1) {
    // If IMU is successfully initialized, configure the sensors
    // Configure Accelerometer: 52 Hz ODR, ±2g FS, High Performance
    IMU_ConfigAccelerometer(ACC_ODR_52HZ, ACC_FS_2G, 1);
    // Configure Gyroscope: 52 Hz ODR, 250 dps FS, High Performance
    IMU_ConfigGyroscope(GYR_ODR_52HZ, GYR_FS_250DPS, 1);
  } else {
    // IMU initialization failed, blink red LED for 2 seconds
	LED_Toggle(LED_RED);
	HAL_Delay(500);
	LED_Toggle(LED_RED);
	HAL_Delay(500);
	LED_Toggle(LED_RED);
	HAL_Delay(500);
	LED_Toggle(LED_RED);
	HAL_Delay(500);
	LED_Toggle(LED_RED);
	HAL_Delay(500);
	LED_Toggle(LED_RED);
	HAL_Delay(500);
  }

  // Turn off the red LED to indicate that initialization is complete
  LED_Off(LED_RED);

  SPEC_Init(); // Inizializza il sensore AS7341
  HAL_DBGMCU_DisableDBGStopMode();
  __HAL_RCC_PWR_CLK_ENABLE();
  /* USER CODE BEGIN MDF Start */
  HAL_DBGMCU_EnableDBGStopMode(); // Permette al debug di funzionare anche in modalità STOP
  // Avvia il monitoraggio acustico (Timer1 per il clock e MDF1 Filter 1 per la soglia)
  printf("\r\n--- Test Microfono Digital IMP34DT05 ---\r\n");
  printf("System: Avvio monitoraggio soglia acustica...\r\n");
  HAL_Delay(50);
  Mic_Start();
  HAL_Delay(100);
  printf("MDF1 Base: GCR=0x%08X, CKGCR=0x%08X\r\n", (unsigned int)MDF1->GCR, (unsigned int)MDF1->CKGCR);
  printf("RCC Regs: CR=0x%08X, PLL3CFGR=0x%08X, PLL3DIVR=0x%08X, CCIPR2=0x%08X\r\n",
         (unsigned int)RCC->CR, (unsigned int)RCC->PLL3CFGR, (unsigned int)RCC->PLL3DIVR, (unsigned int)RCC->CCIPR2);
  printf("MDF1_FLT1: DFLTCR=0x%08X, DFLTCICR=0x%08X, DFLTIER=0x%08X, DFLTISR=0x%08X\r\n", 
         (unsigned int)MdfHandle1.Instance->DFLTCR, (unsigned int)MdfHandle1.Instance->DFLTCICR, 
         (unsigned int)MdfHandle1.Instance->DFLTIER, (unsigned int)MdfHandle1.Instance->DFLTISR);
  printf("MDF1_FLT1 OLD: OLDCR=0x%08X, OLDTHLR=0x%08X, OLDTHHR=0x%08X, DFLTDR=0x%08X\r\n", 
         (unsigned int)MdfHandle1.Instance->OLDCR, (unsigned int)MdfHandle1.Instance->OLDTHLR, 
         (unsigned int)MdfHandle1.Instance->OLDTHHR, (unsigned int)MdfHandle1.Instance->DFLTDR);
  HAL_Delay(100);
  printf("System: Ingresso in SLEEP.\r\n");
  HAL_Delay(50);
  /* USER CODE END MDF Start */

  HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI); // wake up only when there is an interrupt

  printf("System: Risvegliato da SLEEP!\r\n");
  HAL_Delay(50);
  printf("MDF1_FLT1 Post-SLEEP: DFLTISR=0x%08X, DFLTDR=0x%08X\r\n", 
         (unsigned int)MdfHandle1.Instance->DFLTISR, (unsigned int)MdfHandle1.Instance->DFLTDR);
  HAL_Delay(50);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

		//LED_Toggle(LED_GREEN);
		//HAL_Delay(1000);

	  switch(current_state)
	  {
	  	  case STATE_IDLE:
	  		// Check if a USB connection has been detected
	  		if(!usb_flag)
		    {
	  			//MX_USB_Device_Init();
		    }
	  		else
	  		{
			   // Transition to the USB_CONNECTED state
	  		   current_state = STATE_USB_CONNECTED;
			   // Green LED on upon USB Connection
			   LED_On(LED_GREEN);
		    }
	  		break;

	  	  case STATE_ACQUISITION:
	  		   // All data acquisition is handled by the timer interrupt

			break;

	  	  case STATE_USB_CONNECTED:
	  		break;

	  	  case STATE_DOWNLOAD:

	  		   // This state manages reading data blocks and sending them via USB.
	  		  // Once download is complete, the state returns to USB_CONNECTED.
	  		  // Read data packets from memory
	  		  read_memory_and_transmit();

			 current_state = STATE_USB_CONNECTED;
	  		 break;
	  }

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.LSIDiv = RCC_LSI_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMBOOST = RCC_PLLMBOOST_DIV2;
  RCC_OscInitStruct.PLL.PLLM = 2;
  RCC_OscInitStruct.PLL.PLLN = 12;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLLVCIRANGE_1;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/**
  * @brief  Callback function for the timer period elapsed event.
  * This function is triggered by a hardware timer at a fixed interval.
  * @param  htim: Pointer to the timer handle.
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
  if(htim == &htim2){
    printf("Size of audio_buffer_acq: %lu bytes\r\n", (unsigned long)sizeof(audio_buffer_acq));
    
    // Read raw data from microphone
    printf("[DEBUG] TIM2 Callback! MdfHandle0.State = %d\r\n", (int)MdfHandle0.State);
    if (MdfHandle0.State == HAL_MDF_STATE_READY)
    {
      acquisition_active = true;

      // 1. Accendi il LED di allerta
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);

      printf("Inizio Monitoraggio Sonoro periodico! Avvio cattura DMA...\r\n");
      
      // 2. Fai partire una cattura rapida di campioni col Filtro 0 e salva in timestamp 
      MDF_DmaConfigTypeDef mdfDmaConfig0 = {0};
      mdfDmaConfig0.Address    = (uint32_t)&audio_buffer_acq[0];
      mdfDmaConfig0.DataLength = AUDIO_SAMPLES * sizeof(audio_buffer_acq[0]);
      mdfDmaConfig0.MsbOnly    = DISABLE;

      if (HAL_MDF_AcqStart_DMA(&MdfHandle0, &MdfFilterConfig0, &mdfDmaConfig0) != HAL_OK)
      {
          Error_Handler();
          acquisition_active = false; //Reset del flag in caso di errore
      }
      
      RTC_TimeTypeDef sTime = {0};
      RTC_DateTypeDef sDate = {0};
      HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
      HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
      timestamp_monitoring = (Time_Struct){.hh = sTime.Hours, .mm = sTime.Minutes, .ss = sTime.Seconds};

    }
    else if (MdfHandle0.State == HAL_MDF_STATE_ACQUISITION)
    {
      printf("[DMA DEBUG] State=%d, Error=0x%08lx, RemainingBytes=%ld, CSR=0x%08lx, CSAR=0x%08lx, CDAR=0x%08lx\r\n",
             (int)handle_GPDMA1_Channel0.State,
             (unsigned long)handle_GPDMA1_Channel0.ErrorCode,
             (long)(GPDMA1_Channel0->CBR1 & 0x3FFFF),
             (unsigned long)GPDMA1_Channel0->CSR,
             (unsigned long)GPDMA1_Channel0->CSAR,
             (unsigned long)GPDMA1_Channel0->CDAR);
    }
  }
}



/**
  * @brief  Callback function for external interrupt events (e.g., a button press).
  * This function is triggered by the rising edge of the user button's signal.
  * @param  GPIO_Pin: The pin that triggered the interrupt.
  */
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == USER_BUTTON_Pin)
	{
		// A button press can trigger different state transitions depending on the current state.
		switch(current_state) {
			case STATE_IDLE:
				// If the device is idle, start data acquisition.
				// If the previous session wrote some data, circularly advance to the next good block
				if (pagina_scritta > 0) {
					b++;
          if (b >= total_good_blocks || bad_blocks[b] == 0xFFFF) {
            b = 1024; // wrap within upper half of memory
          }
					pagina_scritta = 0;
				}
				// Set up session boundary pointers
				session_start_block = a_scritta;
				session_active = 0;b;
				session_start_page = pagin
				global_sample_count = 0;

				current_state = STATE_ACQUISITION;
        printf("Starting data acquisition (POLLING mode)...\n");
				
				// Erase the initial block to prepare for sequential writes
				read_address_t erase_addr;
				erase_addr.block = bad_blocks[b];
				erase_addr.page = 0;
				erase_addr.dummy = 0;
				printf("[NAND] Erasing start block %u...\r\n", erase_addr.block);
				spi_nand_block_erase(erase_addr);

				// HAL_TIM_Base_Start_IT(&htim2); // Start the timer for periodic data reading
				LED_On(LED_GREEN); // Provide visual feedback for starting acquisition
			break;
			case STATE_ACQUISITION:
				// If data acquisition is active, stop it.
        button_force_stop = 1; // Set a flag to say that the acquisition has been interrupted
				current_state = STATE_IDLE;
				// HAL_TIM_Base_Stop_IT(&htim2); // Stop the timer

				LED_Off(LED_GREEN); // Turn off the LED
        printf("Data acquisition stopped by user.\n");
        flush_memory();
        Debug_Read_And_Print_Nand();
				break;
			case STATE_USB_CONNECTED:
				// If USB is connected, start the download process.
				exit_flag = 0;
				current_state = STATE_DOWNLOAD;
				break;
			default:
				// Do nothing for other states (e.g., if button is pressed during DOWNLOAD).
				break;
		}
  }
}
// Falling Edge when User Button is not pressed
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == USER_BUTTON_Pin)
	{

	}
}
int32_t global_max_peak = 0;

// Callback per il rilevamento di eventi di stress acustico (SCD)
void HAL_MDF_OldCallback(MDF_HandleTypeDef *hmdf, uint32_t TresholdInfo)
{
    //printf("MDF Callback: Soglia acustica superata! TresholdInfo: 0x%08lX\r\n", TresholdInfo);
    if (hmdf->Instance == MDF1_Filter1)
    {
        // Se c'è già una cattura in corso su Filtro 0, non facciamo nulla
        if (MdfHandle0.State == HAL_MDF_STATE_READY)
        {

          peak_detected = true;

          // 1. Accendi il LED di allerta
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
          
          printf("MDF Callback: Superata soglia acustica! Avvio cattura DMA...\r\n");
          
          // 2. Fai partire una cattura rapida di campioni col Filtro 0  
          MDF_DmaConfigTypeDef mdfDmaConfig0 = {0};
          mdfDmaConfig0.Address    = (uint32_t)&audio_buffer_peak[0];
          mdfDmaConfig0.DataLength = AUDIO_SAMPLES * sizeof(audio_buffer_peak[0]);
          mdfDmaConfig0.MsbOnly    = DISABLE;
          if (HAL_MDF_AcqStart_DMA(&MdfHandle0, &MdfFilterConfig0, &mdfDmaConfig0) != HAL_OK)
          {
              Error_Handler();
              peak_detected = false; // Reset del flag in caso di errore
          }

          // Salva il timestamp del rilevamento del picco
          RTC_TimeTypeDef sTime = {0};
          RTC_DateTypeDef sDate = {0};
          HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
          HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
          timestamp_peak = (Time_Struct){.hh = sTime.Hours, .mm = sTime.Minutes, .ss = sTime.Seconds};

        }
    }
}

// Quando il buffer è pieno, calcoliamo i dB
void HAL_MDF_AcqCpltCallback(MDF_HandleTypeDef *hmdf)
{
    if (hmdf->Instance == MDF1_Filter0)
    {
        if(peak_detected) {
          // Se questa callback è stata chiamata da una cattura rapida in seguito al rilevamento di un picco,
          //  calcoliamo i dB e poi spegniamo il LED di allerta

          printf("MDF Callback: Cattura DMA completata dopo rilevamento picco! Calcolo dB...\r\n");
          peak_detected = false; // Reset del flag

            // Ferma l'acquisizione su Filtro 0 per reimpostare lo stato a READY per il prossimo trigger
          HAL_MDF_AcqStop(&MdfHandle0);

          // Spegni il LED di allerta
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);

          // Calcola il valore di picco assoluto nel buffer corrente (valori a 24-bit allineati)
          int32_t current_peak = 0;
          for (int i = 0; i < AUDIO_SAMPLES; i++)
          {
              int32_t val = audio_buffer_peak[i] >> 8;
              if (val < 0) val = -val;
              if (val > current_peak) current_peak = val;
          }

          // Se il picco corrente supera il massimo registrato, lo stampiamo
          if (current_peak > global_max_peak)
          {
              global_max_peak = current_peak;
              printf(">>> NUOVO PICCO GLOBALE RILEVATO (valore di soglia): %ld <<<\r\n", (long)global_max_peak);
          }

          // All'inizio della callback del picco, verifichi il cooldown energetico
          if (Mic_ApplyCooldownProtection() == 0) 
          {
            return; // Salta l'elaborazione se siamo sommersi da troppi interrupt vicini
          }

          // Calcola anche i dB per riferimento
          current_peak_dbspl = Calculate_dB(audio_buffer_peak, AUDIO_SAMPLES);

          // 2. Controllo Orario e Smistamento alla funzione Diurna o Notturna
          // timestamp_peak contiene l'ora estratta dall'RTC al momento del trigger dell'OLD
          if (timestamp_peak.hh >= 7 && timestamp_peak.hh < 23)
          {
              // Fascia oraria diurna (07:00 - 22:59)
              Mic_AnalyzePeak_Daytime(current_peak_dbspl);
          }
          else
          {
              // Fascia oraria notturna (23:00 - 06:59)
              Mic_AnalyzePeak_Nighttime(current_peak_dbspl);
          }

        } else if (acquisition_active) {

          printf("MDF Callback: Cattura DMA completata durante acquisizione periodica! Calcolo dB...\r\n");
          acquisition_active = false; // Reset del flag

            // Ferma l'acquisizione su Filtro 0 per reimpostare lo stato a READY per il prossimo trigger
          HAL_MDF_AcqStop(&MdfHandle0);

          // Spegni il LED di allerta
          HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);

          // Se questa callback è stata chiamata da una cattura periodica, calcoliamo i dB per riferimento
          printf("[DEBUG] Primi campioni acquisiti: [0]=%ld, [1]=%ld, [2]=%ld\r\n", 
                 (long)audio_buffer_acq[0], 
                 (long)audio_buffer_acq[1], 
                 (long)audio_buffer_acq[2]);
          current_acquisition_dbspl = Calculate_dB(audio_buffer_acq, AUDIO_SAMPLES);
          printf("[Acquisition] Campione salvato: %02d:%02d:%02d -> %.2f dBSPL (Totale: %d)\r\n",
                 timestamp_monitoring.hh, timestamp_monitoring.mm, timestamp_monitoring.ss,
                 current_acquisition_dbspl, sample);

          write_packet(sample, timestamp_monitoring, current_acquisition_dbspl, NAND_packet); // Salva su NAND Flash
          sample++;

        }
        write_memory(); // Salva su NAND Flash
    }
}

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{

  /* Disables the MPU */
  HAL_MPU_Disable();

  /* Enables the MPU */
  HAL_MPU_Enable(MPU_HFNMI_PRIVDEF);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  //User can add his own implementation to report the file name and line number,
  printf("Wrong parameters value: file %s on line %d\r\n", file, line);
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
