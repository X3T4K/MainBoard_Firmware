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
#include "Spec_AS7341.h"
#include "callback_LPDMA.h"
#include "lpbam_i2c_spec.h"
#include <stdint.h>


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

/// @brief 

data_packet pacchetto;
Time_Struct time_date;

//uint8_t AS7341_start_register = 0x95; //inizio a leggere da CH0
__attribute__((section(".sram4_retention"))) uint8_t AS7341_start_register[1]; //inizio a leggere da STATUS, mi serve ASTATUS per avere il gain
// Registro di partenza (Nota: meglio uint8_t per registri I2C)
__attribute__((section(".sram4_retention"))) uint8_t Flicker_REG[1];

volatile uint8_t as7341_int_alarm = 0; // interrupt di soglia

// Buffer in SRAM4 per LPBAM/DMA
__attribute__((section(".sram4_retention"))) uint8_t AS7341_Rx_Buffer[60]; // Buffer per i dati luce blu
uint8_t DataBufferOffset = 0; // Offset per leggere i dati luce blu (CH0-CH5) dopo i primi 3 byte di STATUS, ASTATUS e GAIN
__attribute__((section(".sram4_retention"))) uint8_t Flicker_buffer[5]; // Buffer per i dati del flicker

// ==========================================
// SRAM4 Retention Variables (No Initializers)
// ==========================================
__attribute__((section(".sram4_retention"))) volatile AppState current_state;
__attribute__((section(".sram4_retention"))) uint16_t nand_offset;
__attribute__((section(".sram4_retention"))) volatile uint8_t lpbam_cycle_complete;
__attribute__((section(".sram4_retention"))) volatile uint8_t button_force_stop;
__attribute__((section(".sram4_retention"))) uint8_t real_samples_numb;
__attribute__((section(".sram4_retention"))) uint16_t NAND_packet[2048];
__attribute__((section(".sram4_retention"))) uint16_t sample;
__attribute__((section(".sram4_retention"))) uint16_t blocco_scritto;
__attribute__((section(".sram4_retention"))) uint8_t pagina_scritta;
__attribute__((section(".sram4_retention"))) uint16_t b;
__attribute__((section(".sram4_retention"))) uint16_t bad_blocks[1024];
__attribute__((section(".sram4_retention"))) uint16_t bad_blocks2[1024];
__attribute__((section(".sram4_retention"))) uint16_t total_good_blocks;
__attribute__((section(".sram4_retention"))) int exit_flag;

// Session-scoped circular boundary tracking pointers
__attribute__((section(".sram4_retention"))) uint16_t session_start_block;
__attribute__((section(".sram4_retention"))) uint8_t session_start_page;
__attribute__((section(".sram4_retention"))) uint16_t session_end_block;
__attribute__((section(".sram4_retention"))) uint8_t session_end_page;
__attribute__((section(".sram4_retention"))) uint8_t session_active;

// --- Global Flags and Variables ---
// Flag to indicate a USB connection event.
// Set to 1 when a USB connection is detected.
uint8_t usb_flag = 0;

/// ----- NAND FLASH variables ----- ///

read_address_t blocco;
column_address_t colonna = 0;

uint16_t data_letto[2048] = {0};

// Timestamp variables //
Time_Struct timestamp;
uint16_t tim = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
/* USER CODE BEGIN PFP */

// Callback che scatta quando il DMA ha finito il suo ultimo trasferimento/nodo
void HAL_DMA_RxCpltCallback(DMA_HandleTypeDef *hdma) {
	// Sostituisci "handle_LPDMA1_Channel0" con la variabile generata dal tuo CubeMX
	// per il canale DMA che gestisce la tua coda LPBAM.
	// Puoi anche usare (hdma->Instance == LPDMA1_Channel0)
	if (hdma == &handle_LPDMA1_Channel0) {
		lpbam_cycle_complete = 1; // Il ciclo autonomo è finito, sveglia la CPU!
	}
}

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
  MX_I2C3_Init();
  MX_LPTIM1_Init();
  MX_ICACHE_Init();
  MX_MDF1_Init();
  MX_RTC_Init();
  MX_SPI2_Init();
  MX_SPI3_Init();
  MX_TIM2_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  /* USER CODE BEGIN 2 */

  // Cold start initialization for SRAM4 retention variables (NOLOAD)
  current_state = STATE_IDLE;
  nand_offset = 0;
  lpbam_cycle_complete = 0;
  button_force_stop = 0;
  real_samples_numb = 0;
  sample = 0;
  blocco_scritto = 0;
  pagina_scritta = 0;
  b = 0;
  exit_flag = 0;
  session_start_block = 0;
  session_start_page = 0;
  session_end_block = 0;
  session_end_page = 0;
  session_active = 0;
  AS7341_start_register[0] = 0x93;
  Flicker_REG[0] = 0xDB;
  memset((void*)NAND_packet, 0, sizeof(NAND_packet));

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
  uint8_t spec_id = SPEC_ReadRegister(0x92);
  printf("[BOOT] AS7341 Device ID: 0x%02X (Expected: 0x24 or similar)\n", spec_id);
  // LPBAM I2C Spec Setup
  __HAL_RCC_LPDMA1_FORCE_RESET();
  __HAL_RCC_LPTIM1_FORCE_RESET();
  __HAL_RCC_I2C3_FORCE_RESET();
  HAL_Delay(10);
  __HAL_RCC_LPDMA1_RELEASE_RESET();
  __HAL_RCC_LPTIM1_RELEASE_RESET();
  __HAL_RCC_I2C3_RELEASE_RESET();
  
  extern DMA_QListTypeDef Blue_Flick_Acq_Q;
  memset(&Blue_Flick_Acq_Q, 0, sizeof(Blue_Flick_Acq_Q)); // removed to avoid undeclared symbol
  MX_I2C_Spec_Init();                                     // Inizializza l'applicazione base
  MX_I2C_Spec_I2C_RX_Init();                              // Inizializza il tuo scenario
  MX_I2C_Spec_I2C_RX_Build();                             // Costruisce la Linked List in memoria
  MX_I2C_Spec_I2C_RX_Link(&handle_LPDMA1_Channel0);       // Collega la coda al canale DMA
  MX_I2C_Spec_I2C_RX_Start(&handle_LPDMA1_Channel0); 
  printf("[BOOT] Initialization completed successfully. Entering main loop...\n");



  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // vai in Stop 2 SOLO se non stai lavorando (USB o download)
    if (current_state == STATE_ACQUISITION) 
    {   
        HAL_DBGMCU_EnableDBGStopMode(); // Keep debug active in Stop mode for ITM/SWO printf
       __HAL_RCC_PWR_CLK_ENABLE();
        HAL_Delay(100); // Give UART/ITM buffers time to serialize and flush completely before cutting clocks!
        HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
          
        // Il PLL si spegne in Stop 2. ripristino il clock al risveglio!
        SystemClock_Config(); 
        HAL_ResumeTick(); // Ripristina il Systick
        //printf("[DEBUG] CPU woke up from Stop 2 mode!\n");
    }  
    
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
	  			if(button_force_stop==1) // se non sono connesso via USB, ma premo il bottone, entro in acquisition mode
          {
            // 1. Leggiamo l'indirizzo di memoria a cui è arrivato l'LPDMA
            uint32_t current_dma_address = handle_LPDMA1_Channel0.Instance->CDAR;

            // 2. Calcoliamo l'indirizzo di partenza del nostro buffer
            uint32_t start_buffer_address = (uint32_t)&AS7341_Rx_Buffer[0];

            // 3. Facciamo la sottrazione per ottenere i byte totali EFFETTIVAMENTE trasferiti (solo se l'indirizzo è valido)
            uint32_t bytes_transferred = 0;
            if (current_dma_address >= start_buffer_address && current_dma_address <= start_buffer_address + sizeof(AS7341_Rx_Buffer)) {
                bytes_transferred = current_dma_address - start_buffer_address;
            }

            // 4. Calcoliamo quanti campioni sani da 12 byte abbiamo
            real_samples_numb = bytes_transferred / AS7341_COLOR_BPS;
            
            printf("[DEBUG] Stop Button: CDAR=0x%08lX, Buffer=0x%08lX, BytesTransferred=%lu, Samples=%d\n",
                   (unsigned long)current_dma_address, (unsigned long)start_buffer_address,
                   (unsigned long)bytes_transferred, (int)real_samples_numb);

            if (real_samples_numb > 5) {
                real_samples_numb = 5; // Limita al massimo a 5 campioni per sicurezza
            }

            // 5. Elabora i dati acquisiti fino a quel momento (real_samples_numb) e salva in memoria
            if (real_samples_numb > 0) {
                Elabora_e_Salva_Campionamento(); 
            }

            // FORCE WRITE THE LAST PARTIAL PAGE TO NAND TO PREVENT DATA LOSS
            flush_nand_memory(nand_offset);

            button_force_stop = 0; // Reset flag to prevent endless loop execution in STATE_IDLE
          }
        }
        //MX_USB_Device_Init();
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
           // 2. Interrupt arrivato, 2 casi
          if (lpbam_cycle_complete) {
              // Caso 1: Il ciclo LPBAM è completo, salvo i dati
              lpbam_cycle_complete = 0; // Resetta la bandierina

              // Temporarily pause background LPTIM triggers to avoid I2C bus collision
              HAL_LPTIM_PWM_Stop(&hlptim1, LPTIM_CHANNEL_1);

              real_samples_numb = NUM_SAMPLES_PER_WAKEUP; // so che sono 5 quando chiamo questa
              Elabora_e_Salva_Campionamento(); // Elabora i dati acquisiti e salva in memoria

              //  Pulisce l'interrupt sul sensore AS7341
              // Legge il registro STATUS (0x93) e lo riscrive per pulire il bit AINT
              uint8_t status_reg = 0;
              // Legge lo stato (e i flag degli interrupt attivi)
              HAL_I2C_Mem_Read(&hi2c3, SPEC_I2C_ADDR, 0x93, I2C_MEMADD_SIZE_8BIT, &status_reg, 1, HAL_MAX_DELAY);

              // Riscrive lo stesso valore. I bit a "1" verranno azzerati dal sensore
              HAL_I2C_Mem_Write(&hi2c3, SPEC_I2C_ADDR, 0x93, I2C_MEMADD_SIZE_8BIT, &status_reg, 1, HAL_MAX_DELAY);

              // Restart LPTIM triggers once manual I2C communication is complete
              HAL_LPTIM_PWM_Start(&hlptim1, LPTIM_CHANNEL_1);
          }else if (as7341_int_alarm){
              // Caso 2: L'interrupt di soglia è arrivato, avverte subito via BLE
              uint8_t AS7341_TRESHOLD[]={[0]=123, [1]=9, [2]=125}; 
              BLE_SendData(AS7341_TRESHOLD, sizeof(AS7341_TRESHOLD));
              printf("[BLE] Soglia superata\n");// invia la notifica attraverso BLE
              as7341_int_alarm = 0; // Resetta la bandierina
          }

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

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.LSIDiv = RCC_LSI_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
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

  /* Enable HSI in Stop mode (HSIKERON) so LPBAM can autonomously request it during Stop 2 sleep! */
  __HAL_RCC_HSISTOP_ENABLE();

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
  /*
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim == &htim2){

        // Read sensor data from the IMU
        IMU_ReadAccelerometerData(&accelerometer_data, raw_accelerometer);
        IMU_ReadGyroscopeData(&gyroscope_data, raw_gyroscope);

        // Send the accelerometer and gyroscope data via BLE
        // We are sending only the X-axis data
        BLE_SendPacket(DATA_TYPE_IMU_ACCELERATION, raw_accelerometer);
        //TODO: Change Gyro function
        //BLE_SendPacket(DATA_TYPE_IMU_GYROSCOPE, (uint32_t)gyroscope_data.x);

        // Save the raw accelerometer and gyroscope data in memory
        // Create timestamp with sampling frequency @100 Hz
        timestamp.sss=tim*10;
		if(timestamp.sss == 1000) {
			timestamp.ss=timestamp.ss+1;
			timestamp.sss= 0;
			tim = 0;
			if (timestamp.ss==60){
				timestamp.mm=timestamp.mm+1;
				timestamp.ss=0;
				if (timestamp.mm==60){
					timestamp.hh=timestamp.hh+1;
					timestamp.mm=0;
				}
			}
		}

		tim++;

		// Create the data packet to be saved in memory
		write_packet(sample, timestamp, raw_accelerometer, raw_gyroscope, NAND_packet);
		sample++;
		// Write data packet in memory
        write_memory();

	}
}
*/

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
						b = 0;
					}
					pagina_scritta = 0;
				}
				// Set up session boundary pointers
				session_start_block = b;
				session_start_page = pagina_scritta;
				session_active = 0;

				current_state = STATE_ACQUISITION;
        printf("Starting data acquisition...\n");
				HAL_TIM_Base_Start_IT(&htim2); // Start the timer for periodic data reading
				LED_On(LED_GREEN); // Provide visual feedback for starting acquisition
			break;
			case STATE_ACQUISITION:
				// If data acquisition is active, stop it.
        button_force_stop = 1; // Set a flag to say that the acquisition has been interrupted
				current_state = STATE_IDLE;
				HAL_TIM_Base_Stop_IT(&htim2); // Stop the timer

				LED_Off(LED_GREEN); // Turn off the LED
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

// Falling Edge when User Button is not pressed or Spectrometer triggers
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == USER_BUTTON_Pin)
	{

	}
	else if (GPIO_Pin == SP_INT_Pin)
	{
		as7341_int_alarm = 1;
		printf("[DEBUG] Spectrometer EXTI5 Interrupt Fired! (as7341_int_alarm=1)\n");
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
  {  }
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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
