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
extern uint8_t real_samples_numb; // Variabile per contare i campioni reali acquisiti in un ciclo.
extern uint16_t sample; // Variabile globale per tenere traccia del campione corrente da elaborare/salvare

void Elabora_e_Salva_Campionamento(void) //dato che le variabili che si usano sono globali non serve passarle alla funzione
{
    
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    // Prendiamo il tempo subito
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	time_date = (Time_Struct){.hh = sTime.Hours, .mm = sTime.Minutes, .ss = sTime.Seconds};
    uint8_t k = time_date.ss;
    

    // Cicliamo attraverso tutti i campionamenti che LPBAM ha depositato in SRAM4
    for (uint16_t i = 0 ; i < real_samples_numb; i++, k++) {

        // Calcoliamo l'indice di partenza per il campionamento corrente
        // Al giro 0 parte da 0. Al giro 1 parte da 12. Al giro 2 da 24, ecc.
        uint16_t color_idx = i * AS7341_COLOR_BPS;
    	uint16_t flick_idx = i;

        // --- 1. ESTRAZIONE GAIN (Relativo al campionamento corrente) ---
        uint8_t current_gain = AS7341_Rx_Buffer[color_idx + 1] & 0x0F;

        // --- 2. ESTRAZIONE DATI FLICKER(Relativo al campionamento corrente) ---

            uint8_t flicker_100 = 0; 
            uint8_t flicker_120 = 0; 
            uint8_t valid_100 = 0; 
            uint8_t valid_120 = 0; 
            uint8_t saturation_flicker = 0; 
            uint8_t flicker_measure_valid = 0; 
            

    	if(Flicker_buffer[flick_idx] & 0x01){
             flicker_100 = 1; 
        }

        if(Flicker_buffer[flick_idx] & 0x02){
              flicker_120 = 1; 
        }
        
        if(Flicker_buffer[flick_idx] & 0x04){
              valid_100 = 1; 
        }

        if(Flicker_buffer[flick_idx] & 0x08){
              valid_120 = 1; 
        }

        if(Flicker_buffer[flick_idx] & 0x10){
             saturation_flicker = 1; 
        }

        if(Flicker_buffer[flick_idx] & 0x20){
             flicker_measure_valid = 1; 
        }
        
    	if (valid_100 && valid_120 && flicker_measure_valid && !saturation_flicker) {
            
            if (flicker_100){
                pacchetto.luce_artificiale = 1; 
            } 
            else if (flicker_120){
                pacchetto.luce_artificiale = 1;
            }
            else {
                pacchetto.luce_artificiale = 0; 
            }
        }

        // --- 2. RICOSTRUZIONE DATI (Aggiungendo il color_idx) ---
        // Canale deep Blu (F1) si trova ai byte 3 e 4 del blocco corrente
        uint16_t deep_blue_raw = ((uint16_t)AS7341_Rx_Buffer[color_idx + 3] << 8) | AS7341_Rx_Buffer[color_idx + 2];
         // Canale Blu (F2) si trova ai byte 3 e 4 del blocco corrente
        uint16_t blue_raw  = ((uint16_t)AS7341_Rx_Buffer[color_idx + 5] << 8) | AS7341_Rx_Buffer[color_idx + 4];
        // Canale Clear si trova ai byte 9 e 10 del blocco corrente
        uint16_t clear_raw = ((uint16_t)AS7341_Rx_Buffer[color_idx + 11] << 8) | AS7341_Rx_Buffer[color_idx + 10];

        
        // --- 3. NORMALIZZAZIONE E CONVERSION IN DATI FISICI ---
		// 40000= 20*1000	20 è fisso perchè è 1/0.05, 1000 è variabile, scelto arbitrariamente
        uint32_t temp_blue  = (uint32_t)blue_raw * 40000;
        uint32_t temp_deep_blue  = (uint32_t)deep_blue_raw * 40000;
        uint32_t temp_clear = (uint32_t)clear_raw * 40000;
        
        // Applichiamo la scalatura ottimizzata (shift a destra invece di divisione), cioe divide per il gain attuale 
        
        pacchetto.blue    = (uint16_t)(temp_blue >> (current_gain-1));
        pacchetto.deep_blue = (uint16_t)(temp_deep_blue >> (current_gain-1));
        pacchetto.clear = (uint16_t)(temp_clear >> (current_gain-1));
        
        //for debug
        printf("Campione %d: Gain=%d, DeepBlue=%d, Blue=%d, Clear=%d, Luce Artificiale=%d\n", i, current_gain, pacchetto.deep_blue, pacchetto.blue, pacchetto.clear, pacchetto.luce_artificiale);


        // --- 4. SALVATAGGIO IN NAND ---

        if (nand_offset >= 2048) {
            write_memory();
            nand_offset = 0;
        }
        write_packet(nand_offset / 7, i, time_date, pacchetto, NAND_packet, real_samples_numb, k); // Scrive il pacchetto elaborato nel buffer NAND
        nand_offset= nand_offset + 7; // Aggiorna l'offset per il prossimo campione (14 byte per campione: 6 di timestamp + 8 di dati)
        sample += 1; // Aggiorna il contatore del campione globale, per tenere traccia di quanti campioni abbiamo scritto in totale (non solo in questo ciclo)
       // Salva in memoria ogni campione, per sicurezza 
          
    } // Fine del ciclo for: passa al prossimo campionamento nel buffer SRAM4
}








 // ---  SALVATAGGIO IN NAND ALTERNATIVO ---
/*
        memcpy(&NAND_packet[nand_offset], &paccheto.deep_blue, sizeof(uint16_t));
        nand_offset += sizeof(uint16_t);

        memcpy(&NAND_packet[nand_offset], &paccheto.blue, sizeof(uint16_t));
        nand_offset += sizeof(uint16_t);

        memcpy(&NAND_packet[nand_offset], &paccheto.clear, sizeof(uint16_t));
        nand_offset += sizeof(uint16_t);

         memcpy(&NAND_packet[nand_offset], &paccheto.luce_artificiale, sizeof(uint8_t));
        nand_offset += sizeof(uint8_t);

        // Controllo della pagina NAND.
        // È fondamentale farlo DENTRO il ciclo for, perché potresti riempire
        // la pagina NAND proprio a metà dell'elaborazione di questi 10 campioni!
        if (nand_offset >= 4096) {
            write_memory();
            nand_offset = 0;
        }
*/