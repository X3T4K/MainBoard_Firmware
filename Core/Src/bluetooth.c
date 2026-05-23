/**
 * \file bluetooth.c
 * \brief Implementation file for RN4871 Bluetooth Low Energy functions.
 *
 * This file contains the implementation of functions for configuring and
 * communicating with the Microchip RN4871 Bluetooth® Low Energy Module.
 *
 * The RN4871 module is a fully certified Bluetooth Smart module that is
 * controlled primarily through ASCII commands sent from a host MCU to its UART.
 * The module operates in two main modes:
 * - Data mode: Acts as a data pipe, transparently transferring serial data.
 * - Command mode: Interprets UART data as ASCII commands for configuration.
 *
 * Default UART Settings:
 * Baud Rate: 115200
 * Data Bits: 8
 * Parity: None
 * Stop Bits: 1
 * Flow Control: Disabled
 */

#include <bluetooth.h>
#include <stdio.h>
#include "SPI_NAND.h"
#include "main.h"
#include "string.h"
#include "stm32u5xx_hal.h"
#include "stm32u5xx_hal_cortex.h"

extern UART_HandleTypeDef huart3;
static uint32_t connection_start_time = 0;
static bool waiting_for_stream = false;

// --- RX Interrupt State Machine ---
typedef enum {
    RX_STATE_IDLE,
    RX_STATE_DATA_PACKET,   // sta raccogliendo {....}
    RX_STATE_STATUS_MSG,    // sta raccogliendo %....%
} BLE_RxState;

static uint8_t     rx_byte;                  // buffer singolo byte per IT
static uint8_t     rx_temp_buf[64];          // buffer di accumulo
static uint8_t     rx_temp_idx  = 0;
static BLE_RxState rx_state     = RX_STATE_IDLE;

static uint8_t          rx_data_packet[16];
uint8_t rx_data_len;
static char             rx_status_msg[64];
static volatile uint8_t rx_data_ready   = 0;
static volatile uint8_t rx_status_ready = 0;


// --- Helper Functions (Internal to this file) ---
// These functions are not meant to be called directly by the user.

static void enter_command_mode(void);
static void exit_command_mode(void);

// --- Public Function Implementations ---

/**
 * @brief Performs a hard reset of the BLE module via the MCU's reset pin.
 *
 * A hard reset is necessary to apply certain configuration changes and
 * to ensure the module is in a known, initial state before sending commands.
 */
void BLE_HardReset(void) {
    // Pull the reset pin low to activate the reset
    HAL_GPIO_WritePin(BLE_RESET_GPIO_Port, BLE_RESET_Pin, 0);
    HAL_Delay(1000); // Wait for the module to reset
    // Pull the reset pin high to exit the reset state
    HAL_GPIO_WritePin(BLE_RESET_GPIO_Port, BLE_RESET_Pin, 1);
}

/**
 * @brief Configures the RN4871 BLE Module at startup.
 *
 * This function sets up the device name, enables the Transparent UART service,
 * and enables the UART RX Indication pin functionality.
 */
void BLE_Initialize(void) {
    uint8_t reboot_response[9] = {0};
    uint8_t command_ok_response[100] = {0};

    // Perform a hard reset to get the module into a known state
    BLE_HardReset();
    HAL_UART_Receive(&huart3, reboot_response, sizeof(reboot_response), UART_TIMEOUT); // Read initial reboot string

    // Enter Command Mode to send configuration commands
    enter_command_mode();

    // Set the device name for easy identification
    uint8_t device_name[] = "SN,BLE_SW_Team_B8\r";
    BLE_SendData(device_name, sizeof(device_name) - 1);
    HAL_UART_Receive(&huart3, command_ok_response, sizeof(command_ok_response), UART_TIMEOUT); // Read 'AOK' response

    // Enable Transparent UART service (UUID: 49535343-FE7D-4AE5-8FA9-9FAFD205E455)
    uint8_t enable_transparent_uart[] = "SS,C0\r";
    BLE_SendData(enable_transparent_uart, sizeof(enable_transparent_uart) - 1);
    HAL_UART_Receive(&huart3, command_ok_response, sizeof(command_ok_response), UART_TIMEOUT);

    // Set Pin P16 to act as UART RX Indication, which is useful for waking up the module
    uint8_t enable_uart_rx_ind[] = "SW,0C,04\r";
    BLE_SendData(enable_uart_rx_ind, sizeof(enable_uart_rx_ind) - 1);
    HAL_UART_Receive(&huart3, command_ok_response, sizeof(command_ok_response), UART_TIMEOUT);

    // Reboot the module for the new settings to take effect
    uint8_t reboot_command[] = "R,1\r";
    BLE_SendData(reboot_command, sizeof(reboot_command) - 1);
    HAL_Delay(100);

    // Exit Command Mode and return to Data Mode
    exit_command_mode();
    printf("BLE ready");
}

/**
 * @brief Configures the RN4871 BLE Module to enter Dormant (Deep Sleep) mode.
 *
 * In this mode, the module consumes minimal power, and all RF communication is stopped.
 * The module can be woken up by toggling the UART_RX_IND pin.
 */
void BLE_EnterDormantMode(void) {
    uint8_t reboot_response[9] = {0};

    // Ensure the module is in a known state before sending commands
    BLE_HardReset();
    HAL_UART_Receive(&huart3, reboot_response, sizeof(reboot_response), UART_TIMEOUT);

    // Set the UART_RX_IND pin high to prepare for Dormant mode entry
    HAL_GPIO_WritePin(BLE_UART_RX_IND_GPIO_Port, BLE_UART_RX_IND_Pin, 1);
    HAL_Delay(10); // Small delay to allow the pin state to stabilize

    // Enter Command Mode
    enter_command_mode();

    // Send the command "O,0" to enter Dormant mode
    uint8_t dormant_mode_command[] = "O,0\r";
    BLE_SendData(dormant_mode_command, sizeof(dormant_mode_command) - 1);

    HAL_Delay(100); // Wait for the module to enter sleep
}

/**
 * @brief Wakes the RN4871 BLE Module from Dormant/Sleep mode.
 *
 * This function wakes the module by toggling the UART_RX_IND pin and then
 * re-initializes it to a functional state.
 */
void BLE_WakeUp(void) {
    // Toggle the UART_RX_IND pin to wake up the module
    // The module's 16 MHz clock restarts when this pin goes low.
    HAL_GPIO_WritePin(BLE_UART_RX_IND_GPIO_Port, BLE_UART_RX_IND_Pin, 0);
    HAL_Delay(5); // Wait for the clock to stabilize

    // After waking up, we need to re-enter command mode and return to data mode
    // to ensure the module is ready for communication.
    BLE_Initialize();
}

/**
 * @brief Configures the RN4871 BLE Module to enter Low-Power mode.
 *
 * In this mode, the module uses a 32kHz clock, significantly reducing power consumption.
 * A BLE connection can still be maintained, but the UART cannot receive data.
 */
void BLE_EnterLowPowerMode(void) {
    uint8_t command_ok_response[100] = {0};

    // Enter Command Mode
    enter_command_mode();

    // Send the command "SO,1" to enable Low-Power mode
    uint8_t low_power_command[] = "SO,1\r";
    BLE_SendData(low_power_command, sizeof(low_power_command) - 1);
    HAL_UART_Receive(&huart3, command_ok_response, sizeof(command_ok_response), UART_TIMEOUT);

    // Exit Command Mode
    exit_command_mode();
}

/**
 * @brief Exits the RN4871 BLE Module from Low-Power mode to Active mode.
 *
 * This function returns the module to its normal operating state (16MHz clock).
 */
void BLE_ExitLowPowerMode(void) {
    uint8_t command_ok_response[100] = {0};

    // Enter Command Mode
    enter_command_mode();

    // Send the command "SO,0" to exit Low-Power mode
    uint8_t exit_low_power_command[] = "SO,0\r";
    BLE_SendData(exit_low_power_command, sizeof(exit_low_power_command) - 1);
    HAL_UART_Receive(&huart3, command_ok_response, sizeof(command_ok_response), UART_TIMEOUT);

    // Exit Command Mode
    exit_command_mode();
}

/**
 * @brief Configures the RN4871 BLE Module for slow advertisements.
 *
 * This reduces power consumption by broadcasting less frequently.
 * The command "A,03E8,002F" sets the advertising interval to 1000ms.
 */
void BLE_SetSlowAdvertisements(void) {
    uint8_t command_ok_response[100] = {0};

    // Enter Command Mode
    enter_command_mode();

    // Send the command "A,03E8,002F" to set slow advertisement intervals
    // 03E8 is 1000 in hex (1000ms), 002F is 47ms (min interval)
    uint8_t slow_ads_command[] = "A,03E8,002F\r";
    BLE_SendData(slow_ads_command, sizeof(slow_ads_command) - 1);
    HAL_UART_Receive(&huart3, command_ok_response, sizeof(command_ok_response), UART_TIMEOUT);

    // Exit Command Mode
    exit_command_mode();
}

/**
 * @brief Sends data to a connected external BLE device.
 *
 * This function uses the Transparent UART service to transmit data.
 * @param data Pointer to the data buffer to send.
 * @param data_length The number of bytes to send.
 */
void BLE_SendData(uint8_t* data, uint8_t data_length) {
    HAL_UART_Transmit(&huart3, data, data_length, UART_TIMEOUT);
}

/**
 * @brief Receives data from a connected external BLE device.
 *
 * This function reads data from the Transparent UART service.
 * @param data Pointer to the buffer where received data will be stored.
 * @param data_length The number of bytes to read.
 */
void BLE_ReceiveData(uint8_t* data, uint8_t data_length) {
    HAL_UART_Receive(&huart3, data, data_length, UART_TIMEOUT);
}

/**
 * @brief Sends a structured data packet with a specific type and value.
 *
 * This function creates a standardized packet format to send specific sensor data.
 * The packet format is: { | Type | MSB of Value | ... | LSB of Value | ... | }
 * @param type The type of data being sent (e.g., acceleration, gyroscope).
 * @param value The 32-bit value to be sent.
 */
void BLE_SendPacket(BLE_DataType ble_data_type, uint8_t* data_buffer) {
    uint8_t ble_packet[PACKET_LENGTH];

    // Initialize the packet buffer
    ble_packet[0] = '{';
    ble_packet[PACKET_LENGTH - 1] = '}';
    for (uint8_t i = 1; i < PACKET_LENGTH - 1; i++) {
        ble_packet[i] = 0;
    }

    // Byte 1: Data type identifier
    switch (ble_data_type) {
        case DATA_TYPE_IMU_ACCELERATION:
            ble_packet[1] = 'A';
            break;
        case DATA_TYPE_IMU_GYROSCOPE:
            ble_packet[1] = 'G';
            break;
        default:
            ble_packet[1] = 'U'; // Unknown data type
            break;
    }

    // Bytes 2-4: The 32-bit value, packed in big-endian format
    ble_packet[2] = data_buffer[0]; // X Axis LSB
    ble_packet[3] = data_buffer[1]; // X Axis MSB
    ble_packet[4] = data_buffer[2]; // Y Axis LSB
    ble_packet[5] = data_buffer[3]; // Y Axis MSB
    ble_packet[6] = data_buffer[4]; // Z Axis LSB
    ble_packet[7] = data_buffer[5]; // Z Axis MSB

    // Send the complete packet over UART
    BLE_SendData(ble_packet, sizeof(ble_packet));
}

// --- Helper Function Implementations ---
// These helper functions encapsulate common, repeated tasks to improve code clarity.

/**
 * @brief Helper function to enter Command Mode.
 *
 * Sends the `$$$` sequence to the module to switch from Data Mode to Command Mode.
 */
static void enter_command_mode(void) {
    uint8_t command_mode_sequence[] = "$$$";
    uint8_t command_prompt_response[5] = {0};
    BLE_SendData(command_mode_sequence, sizeof(command_mode_sequence) - 1);
    HAL_UART_Receive(&huart3, command_prompt_response, sizeof(command_prompt_response), UART_TIMEOUT);
    HAL_Delay(100);
}

/**
 * @brief Helper function to exit Command Mode.
 *
 * Sends the `---` command to the module to switch back to Data Mode.
 */
static void exit_command_mode(void) {
    uint8_t data_mode_command[] = "---\r";
    //uint8_t command_ok_response[100] = {0};
    BLE_SendData(data_mode_command, sizeof(data_mode_command) - 1);
    //HAL_UART_Receive(&huart3, command_ok_response, sizeof(command_ok_response), UART_TIMEOUT);
    HAL_Delay(100);
}

void BLE_FlushRxBuffer(void) {
    uint8_t dummy;
    // Legge e scarta tutto ciò che è rimasto nel buffer, finché non c'è più nulla
    while (HAL_UART_Receive(&huart3, &dummy, 1, 5) == HAL_OK);
    printf("[BLE] Buffer pulito\n");
}


// // --- Callback Functions ---
// These functions are called by the HAL library in response to hardware events

/**
 * @brief Arma il primo interrupt RX. Va chiamata una volta sola dopo BLE_Initialize().
 */

void BLE_StartReceive(void) {
    HAL_UART_Receive_IT(&huart3, &rx_byte, 1);
}

/**
 * @brief Callback HAL: chiamata automaticamente ad ogni byte ricevuto.
 *        Esegue la state machine e ri-arma subito l'interrupt.
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart != &huart3) return;

    uint8_t b = rx_byte;

    switch (rx_state) {

        case RX_STATE_IDLE:
            if (b == '{') {
                rx_temp_buf[rx_temp_idx++] = b;
                rx_state = RX_STATE_DATA_PACKET;
            } else if (b == '%') {
                rx_temp_buf[rx_temp_idx++] = b;
                rx_state = RX_STATE_STATUS_MSG;
            }
            // qualsiasi altro byte in idle: scartato silenziosamente
            break;

        case RX_STATE_DATA_PACKET:
            rx_temp_buf[rx_temp_idx++] = b;
            if (b == '}') {
                // pacchetto completo
                rx_data_len = rx_temp_idx;
                memset(rx_data_packet, 0, sizeof(rx_data_packet));
                memcpy(rx_data_packet, rx_temp_buf, rx_temp_idx);
                rx_data_ready = 1;
                rx_temp_idx   = 0;
                rx_state      = RX_STATE_IDLE;
            } else if (rx_temp_idx >= sizeof(rx_temp_buf)) {
                // overflow: pacchetto malformato, reset
                rx_temp_idx = 0;
                rx_state    = RX_STATE_IDLE;
            }
            break;

        case RX_STATE_STATUS_MSG:
            rx_temp_buf[rx_temp_idx++] = b;
            if (b == '%' && rx_temp_idx > 1) {
                // messaggio di stato completo (es. %CONNECT,xxx%)
                memcpy(rx_status_msg, rx_temp_buf, rx_temp_idx);
                rx_status_msg[rx_temp_idx] = '\0';
                rx_status_ready = 1;
                rx_temp_idx     = 0;
                rx_state        = RX_STATE_IDLE;
            } else if (rx_temp_idx >= sizeof(rx_temp_buf)) {
                rx_temp_idx = 0;
                rx_state    = RX_STATE_IDLE;
            }
            break;
    }

    // Ri-arma subito per il prossimo byte
    HAL_UART_Receive_IT(&huart3, &rx_byte, 1);
}

/**
 * @brief Da chiamare nel main loop: processa i messaggi completati dall'interrupt.
 *        Non blocca mai.
 */
void BLE_ProcessRxBuffer(void) {
    if (rx_data_ready) {
        rx_data_ready = 0;
        
        // Caso ACK (6)
        if (rx_data_packet[1] == 6) {
            uint8_t ack_packet[] = {123, 7, 125};
            BLE_SendData(ack_packet, sizeof(ack_packet));
            printf("[BLE] ACK inviato\n");
        }
        // --- NUOVO: Caso Request Page ('P' / 80) ---
        else if (rx_data_packet[1] == 80) { // 'P' in ASCII
            // Ricostruiamo l'intero a 24-bit dai 3 byte ricevuti
            uint32_t requested_page = (rx_data_packet[2] << 16) | 
                                      (rx_data_packet[3] << 8)  | 
                                       rx_data_packet[4];
            
            printf("[BLE] Ricevuta richiesta per pagina assoluta: %lu\n", requested_page);
            
            // Richiama la funzione di trasmissione che abbiamo strutturato in precedenza
            BLE_Transmit_NAND_Page(requested_page);
        }
    }
    
    if (rx_status_ready) {

        printf("[BLE STATUS] %s\n", rx_status_msg);
        rx_status_ready = 0;
       if(strstr(rx_status_msg, "DISCONNECT")) {
            ble_connection_status = BLE_DISCONNECTED;
            printf("[BLE] Disconnesso\n");
        }else if (strstr(rx_status_msg, "CONNECT")) {
            waiting_for_stream = true;
            connection_start_time = HAL_GetTick(); // Registro il tempo di inizio
            printf("[BLE] Connesso, attendo STREAM_OPEN...\n");
            printf("[BLE] Tempo di inizio connessione: %lu\n", (unsigned long)connection_start_time);
        } 
        else if (strstr(rx_status_msg, "STREAM_OPEN")) {
            waiting_for_stream = false; // Handshake completato con successo
            printf("[BLE] Stream aperto\n");
        }
        rx_status_ready = 0;
        // Controllo del Timeout (es. 5 secondi)
        if (waiting_for_stream && (HAL_GetTick() - connection_start_time > 5000)) {
            printf("[BLE] TIMEOUT: Stream non aperto. Reset modulo...\n");
            waiting_for_stream = false;
            BLE_HardReset(); // Forza il modulo a tornare in Advertising
        }
    }
}


void read_spectrumData_and_BLE_transmit(){

}

void read_micData_and_BLE_transmit(){

}


// --- Funzioni per trasmissione dati da NAND ---


#define BLE_MAX_PAYLOAD 151
#define TYPE_DATA 0x44 // 'D'
#define TYPE_EOP  0x45 // 'E'
#define TYPE_EOD  0x46 // 'F' - Finished/End Of Dump

// Funzione helper per inviare un chunk formattato
void BLE_SendChunk(uint8_t type, uint8_t *payload, uint16_t length) {
    uint8_t packet[158]; 
    uint16_t packet_idx = 0;

    packet[packet_idx++] = '{';
    packet[packet_idx++] = type;
    packet[packet_idx++] = (length >> 8) & 0xFF;
    packet[packet_idx++] = length & 0xFF;

    memcpy(&packet[packet_idx], payload, length);
    packet_idx += length;

    uint16_t crc16 = BLE_CalculateCRC16(payload, length);
    packet[packet_idx++] = (crc16 >> 8) & 0xFF;
    packet[packet_idx++] = crc16 & 0xFF;
    packet[packet_idx++] = '}';

    BLE_SendData(packet, packet_idx);
}


void BLE_Transmit_NAND_Page(uint32_t absolute_page) {
    // 1. Mappatura Logico -> Fisico
    uint16_t logical_block = absolute_page / 64;
    uint8_t physical_page = absolute_page % 64;
    
    // 2. Controllo Limiti di Sicurezza (Memoria Finita o End Of Partition)
    // Se sforiamo l'array o becchiamo un blocco non inizializzato, inviamo l'EOD.
    if (logical_block >= 2048 || bad_blocks[logical_block] == (uint16_t)-1) {
        uint8_t dummy = 0;
        BLE_SendChunk(TYPE_EOD, &dummy, 1);
        return;
    }

    read_address_t row;
    row.block = bad_blocks[logical_block]; 
    row.page = physical_page;
    row.dummy = 0;

    column_address_t colonna = 0;
    spi_nand_page_read(row, colonna, data_letto, 4096);

    // 3. Controllo "Early EOD" (Pagina non ancora scritta)
    // Una NAND cancellata ha tutti i bit a 1 (0xFF). Controlliamo l'inizio.
    bool is_empty = true;
    for(int i = 0; i < 16; i++) {
        if(data_letto[i] != 0xFF) { 
            is_empty = false; 
            break; 
        }
    }

    if (is_empty) {
        printf("[BLE] Pagina vuota trovata a logica %lu. Invio EOD.\n", absolute_page);
        uint8_t dummy = 0;
        BLE_SendChunk(TYPE_EOD, &dummy, 1);
        return;
    }

    // 4. Se la pagina ha dati, procediamo col normale invio a chunk
    uint16_t offset = 0;
    while (offset < 4096) {
        uint16_t chunk_size = (4096 - offset > BLE_MAX_PAYLOAD) ? BLE_MAX_PAYLOAD : (4096 - offset);
        BLE_SendChunk(TYPE_DATA, data_letto + offset, chunk_size);
        offset += chunk_size;
        HAL_Delay(15); // Da rimuovere se implementi Flow Control Hardware (RTS/CTS)
    }

    // 5. Invio pacchetto EOP con CRC32
    uint32_t page_crc32 = BLE_CalculateCRC32(data_letto, 4096);
    uint8_t eop_payload[4];
    eop_payload[0] = (page_crc32 >> 24) & 0xFF;
    eop_payload[1] = (page_crc32 >> 16) & 0xFF;
    eop_payload[2] = (page_crc32 >> 8) & 0xFF;
    eop_payload[3] = page_crc32 & 0xFF;

    BLE_SendChunk(TYPE_EOP, eop_payload, 4);
}


uint16_t BLE_CalculateCRC16(uint8_t *data, uint16_t length) {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < length; i++) {
        crc ^= (uint16_t)(data[i] << 8);
        for (uint8_t j = 0; j < 8; j++) {
            if ((crc & 0x8000) != 0) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

uint32_t BLE_CalculateCRC32(uint8_t *data, uint16_t length) {
    uint32_t crc = 0xFFFFFFFF;
    for (uint16_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if ((crc & 1) != 0) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc ^ 0xFFFFFFFF;
}