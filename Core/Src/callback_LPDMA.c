#include "callback_LPDMA.h"
#include "main.h"
#include "Memory_operations.h"
#include <stdint.h>
#include "rtc.h"

extern uint16_t NAND_packet[2048];
extern uint16_t nand_offset;
extern data_packet pacchetto;
extern Time_Struct time_date;
extern uint8_t AS7341_Rx_Buffer[60];
extern uint8_t Flicker_buffer[5];
extern uint8_t real_samples_numb; 
extern uint16_t sample; 

// Array di Look-up per il guadagno reale dell'AS7341 in base al valore del registro AGAIN (0..10)
// Mappatura ufficiale datasheet: 0=0.5x, 1=1x, 2=2x, 3=4x, 4=8x, 5=16x, 6=32x, 7=64x, 8=128x, 9=256x, 10=512x
static const uint16_t AS7341_Gain_Value[] = {1, 1, 2, 4, 8, 16, 32, 64, 128, 256, 512};

void Elabora_e_Salva_Campionamento(void) 
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    // Prendiamo il tempo dal Real Time Clock
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    time_date = (Time_Struct){.hh = sTime.Hours, .mm = sTime.Minutes, .ss = sTime.Seconds};
    uint8_t k = time_date.ss;

    // Cicliamo attraverso tutti i campionamenti che LPBAM ha depositato in SRAM4
    for (uint16_t i = 0 ; i < real_samples_numb; i++, k++) {

        pacchetto.luce_artificiale = 0;

        // Calcoliamo l'indice di partenza per il campionamento corrente (12 byte per blocco)
        uint16_t color_idx = i * AS7341_COLOR_BPS;
        uint16_t flick_idx = i;

        // --- 1. ESTRAZIONE GAIN (Registro ASTATUS [3:0]) ---
        uint8_t gain_reg = AS7341_Rx_Buffer[color_idx + 1] & 0x0F;
        if (gain_reg > 10) {
            gain_reg = 1; // Fallback a 1x se fuori range nominale
        }
        uint16_t actual_gain = AS7341_Gain_Value[gain_reg];

        // --- 2. ESTRAZIONE DATI FLICKER ---
        uint8_t flicker_val = Flicker_buffer[flick_idx];
        uint8_t flicker_100           = (flicker_val & 0x01) ? 1 : 0; 
        uint8_t flicker_120           = (flicker_val & 0x02) ? 1 : 0; 
        uint8_t valid_100             = (flicker_val & 0x04) ? 1 : 0; 
        uint8_t valid_120             = (flicker_val & 0x08) ? 1 : 0; 
        uint8_t saturation_flicker    = (flicker_val & 0x10) ? 1 : 0; 
        uint8_t flicker_measure_valid = (flicker_val & 0x20) ? 1 : 0; 
        
        if (flicker_measure_valid && !saturation_flicker) {
            if ((valid_100 && flicker_100) || (valid_120 && flicker_120)) {
                pacchetto.luce_artificiale = 1; 
            }
        }

        // --- 3. RICOSTRUZIONE DATI RAW (Little Endian dall'I2C Stream) ---
        // I canali ADC occupano 2 byte ciascuno a partire dal byte 2 del buffer
        uint16_t ch0_raw = ((uint16_t)AS7341_Rx_Buffer[color_idx + 3]  << 8) | AS7341_Rx_Buffer[color_idx + 2];
        uint16_t ch1_raw = ((uint16_t)AS7341_Rx_Buffer[color_idx + 5]  << 8) | AS7341_Rx_Buffer[color_idx + 4];
        uint16_t ch2_raw = ((uint16_t)AS7341_Rx_Buffer[color_idx + 7]  << 8) | AS7341_Rx_Buffer[color_idx + 6];
        uint16_t ch3_raw = ((uint16_t)AS7341_Rx_Buffer[color_idx + 9]  << 8) | AS7341_Rx_Buffer[color_idx + 8];
        uint16_t ch4_raw = ((uint16_t)AS7341_Rx_Buffer[color_idx + 11] << 8) | AS7341_Rx_Buffer[color_idx + 10];

        // --- 4. NORMALIZZAZIONE CON COEFFICIENTI E GAIN REALE ---
        // Utilizziamo variabili temporanee a 64 bit per evitare qualsiasi overflow durante la moltiplicazione per 40000
        uint64_t temp_deep_blue = ((uint64_t)ch0_raw * 40000) / actual_gain;
        uint64_t temp_blue      = ((uint64_t)ch1_raw * 40000) / actual_gain;
        uint64_t temp_clear     = ((uint64_t)ch4_raw * 40000) / actual_gain;

        // Saturazione di sicurezza prima del cast a 16 bit per non corrompere il dato registrato
        pacchetto.deep_blue = (temp_deep_blue > 0xFFFF) ? 0xFFFF : (uint16_t)temp_deep_blue;
        pacchetto.blue      = (temp_blue > 0xFFFF) ? 0xFFFF : (uint16_t)temp_blue;
        pacchetto.clear     = (temp_clear > 0xFFFF) ? 0xFFFF : (uint16_t)temp_clear;
        
        // --- 5. LOG DI DEBUG ---
        printf("Campione %d: Gain_Reg=%d (x%d), DeepBlue=%u, Blue=%u, Clear=%u, Luce Artificiale=%d\n", 
               i, gain_reg, actual_gain, pacchetto.deep_blue, pacchetto.blue, pacchetto.clear, pacchetto.luce_artificiale);
        
        printf("[DEBUG] Raw Bytes: STATUS=0x%02X, ASTATUS=0x%02X, Flicker=0x%02X | CH0=%04X, CH1=%04X, CH2=%04X, CH3=%04X, CH4=%04X\n",
               AS7341_Rx_Buffer[color_idx + 0],
               AS7341_Rx_Buffer[color_idx + 1],
               flicker_val,
               ch0_raw, ch1_raw, ch2_raw, ch3_raw, ch4_raw);

        // --- 6. SALVATAGGIO IN NAND ---
        if (nand_offset > 2041) {
            write_memory();
            nand_offset = 0;
        }
        write_packet(nand_offset, time_date, pacchetto, NAND_packet, real_samples_numb, k, i); 
        nand_offset = nand_offset + 7; 
        sample += 1; 
    } 
}