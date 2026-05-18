#include "callback_LPDMA.h"
#include "main.h"
#include "Memory_operations.h"

extern uint8_t NAND_packet[4096];
extern uint16_t nand_offset;
extern data_packet;
float f1Cost = 69.660;  // VERIFICARE I VALORI, HO UN DEEP RESEARCH DA VALUTARE
float f2Cost = 34.830;

void Elabora_e_Salva_Campionamento_Multiplo(void) {

    // Cicliamo attraverso tutti i campionamenti che LPBAM ha depositato in SRAM4
    for (uint16_t i = 0, j=0; i < NUM_SAMPLES_PER_WAKEUP, j < NUM_SAMPLES_PER_WAKEUP; i++, j++) {

        // Calcoliamo l'indice di partenza per il campionamento corrente
        // Al giro 0 parte da 0. Al giro 1 parte da 12. Al giro 2 da 24, ecc.
        uint16_t color_idx = i * AS7341_COLOR_BPS;
    	uint16_t flick_idx = j;

        // --- 1. ESTRAZIONE GAIN (Relativo al campionamento corrente) ---
        uint8_t current_gain = AS7341_Rx_Buffer[color_idx + 1] & 0x0F;

        // --- 2. ESTRAZIONE DATI FLICKER(Relativo al campionamento corrente) ---

            uint8_t flicker_100 = 0; 
            uint8_t flicker_120 = 0; 
            uint8_t valid_100 = 0; 
            uint8_t valid_120 = 0; 
            uint8_t saturation_flicker = 0; 
            uint8_t flicker_measure_valid = 0; 
            uint8_t luce_artificiale = 0;

    	if(Flicker_buffer[flick_idx] & 0x00){
             uint8_t flicker_100 = 1; 
        }

        if(Flicker_buffer[flick_idx] & 0x01){
             uint8_t flicker_120 = 1; 
        }
        
        if(Flicker_buffer[flick_idx] & 0x02){
             uint8_t valid_100 = 1; 
        }

        if(Flicker_buffer[flick_idx] & 0x03){
             uint8_t valid_120 = 1; 
        }

        if(Flicker_buffer[flick_idx] & 0x04){
             uint8_t saturation_flicker = 1; 
        }

        if(Flicker_buffer[flick_idx] & 0x05){
             uint8_t flicker_measure_valid = 1; 
        }
        
    	if (valid_100 & valid_120 & flicker_measure_valid & !saturation_flicker) {
            
            if (flicker_100){
                data_packet.luce_artificiale = 1; 
            } 
            else if (flicker_120){
                data_packet.luce_artificiale = 1;
            }
            else {
                data_packet.luce_artificiale = 0; 
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

        data_packet.blue    = (uint16_t)(temp_blue >> current_gain);
        data_packet.deep_blue = (uint16_t)(temp_deep_blue >> current_gain);
        data_packet.clear = (uint16_t)(temp_clear >> current_gain);


        // --- 4. SALVATAGGIO IN NAND ---
        memcpy(&NAND_packet[nand_offset], &data_packet.deep_blue, sizeof(uint16_t));
        nand_offset += sizeof(uint16_t);

        memcpy(&NAND_packet[nand_offset], &data_packet.blue, sizeof(uint16_t));
        nand_offset += sizeof(uint16_t);

        memcpy(&NAND_packet[nand_offset], &data_packet.clear, sizeof(uint16_t));
        nand_offset += sizeof(uint16_t);

         memcpy(&NAND_packet[nand_offset], &data_packet.luce_artificiale, sizeof(uint8_t));
        nand_offset += sizeof(uint8_t);

        // Controllo della pagina NAND.
        // È fondamentale farlo DENTRO il ciclo for, perché potresti riempire
        // la pagina NAND proprio a metà dell'elaborazione di questi 10 campioni!
        if (nand_offset >= 4096) {
            write_memory();
            nand_offset = 0;
        }

    } // Fine del ciclo for: passa al prossimo campionamento nel buffer SRAM4
}

