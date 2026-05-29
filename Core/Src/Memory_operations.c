/*
 * Memory_operations.c
 *
 *  This file contains the high-level operations that can be used for managing the SPI NAND
 *  You will need the SPI_NAND.c library.
 *
 */


#include "string.h"
#include "stdio.h"
#include "stdbool.h"
#include "main.h"
#include "SPI.h"
#include "SPI_NAND.h"
#include <stdint.h>
#include "Memory_operations.h"

NAND_info data;
//void write_packet(uint8_t sample, RTC_TimeTypeDef sTime, uint8_t *magnetometer, uint8_t *pressure, uint8_t *temp, uint8_t *gyroscope,uint8_t *accelerometer, uint8_t *gyroscope2,uint8_t *accelerometer2,uint8_t *ppg_sample, uint8_t *NAND_packet);

// todo:
/*
 * All'inizio faccio un spi_nand_init(); che non cancella il contenuto della memoria
 * Il flusso di operazioni sarà: inizializzo, leggo tutto, identifico i bad blocks, erase tutto, scrivo tutto
 * Idea: uso il primo blocco disponibile per salvarmi il blocco e la pagina a cui sono arrivata -> può avere dei problemi.
 */

extern uint16_t total_good_blocks;

void find_bad_blocks(uint16_t *bad_blocks){
	// inizializzo bad_blocks con -1
	// metto l'indice del good_block nel vettore
	// i++ e prendo il numero scritto dentro nel vettore, non il suo indice quando devo andare a scrivere

	read_address_t blocco;
	blocco.block=0;
	blocco.page=0;
	blocco.dummy=0;
	bool is_bad_mark=true;
	int j = 0;
	for(int i = 0; i<1024; i++){
		blocco.block=i;
		spi_nand_block_is_bad(blocco, &is_bad_mark);
		/*
		if(is_bad_mark){
		  bad_blocks[i]=1;
		}*/
		if(!is_bad_mark) {
		  bad_blocks[j]=i;
		  j++;
		}
	}
	
	// Pad remaining unpopulated elements of bad_blocks with 0xFFFF (invalid block marker)
	for (int k = j; k < 1024; k++) {
		bad_blocks[k] = 0xFFFF;
	}
	total_good_blocks = j; // Store the exact count of discovered good blocks
}

// Questo da tenere così
void erase_good_blocks(uint16_t *bad_blocks){
	read_address_t blocco;
	blocco.block=0;
	blocco.page=0;
	blocco.dummy=0;
	bool is_bad_mark=true;
	for(int i = 0; i<1024; i++){
		blocco.block=i;
		spi_nand_block_is_bad(blocco, &is_bad_mark);
		if(is_bad_mark){
		  bad_blocks[i]=1;
		}
		if(!is_bad_mark) {
		  bad_blocks[i]=0;
		  spi_nand_block_erase(blocco);
		}
	}
}

void write_packet(uint16_t nand_offset, Time_Struct time_date, data_packet pacchetto, uint16_t *NAND_packet,uint8_t samples_numb, uint8_t k, uint16_t i) {
	
	if(k < samples_numb){  // ora è un if perchè essendo dentro il 	for  la chiamata, di base scorre, non serve il while.
		time_date.ss = 60-(samples_numb-k-1); //correezione per tempo esatto, prima non arrivava al tempo dell'ultima misura.
		if (time_date.mm == 0)
		{
			if(time_date.hh == 0)
			{
			time_date.hh = 23;
			} else {
			time_date.hh = time_date.hh - 1;
			}
		time_date.mm = 59;
		} else {
			time_date.mm = time_date.mm - 1;
		}
		// Time
		NAND_packet[0 + nand_offset] = time_date.hh;
		NAND_packet[1 + nand_offset] = time_date.mm;
		NAND_packet[2 + nand_offset] = time_date.ss;
		// Channels
		NAND_packet[3 + nand_offset] = pacchetto.luce_artificiale ;
		NAND_packet[4 + nand_offset] = pacchetto.blue;
		NAND_packet[5 + nand_offset] = pacchetto.deep_blue;
		NAND_packet[6 + nand_offset] = pacchetto.clear;
		// spostato k++ direttamente nel ciclo for di callback_LPDMA.c
	} else {
	// Time
	NAND_packet[0 + nand_offset] = time_date.hh;
	NAND_packet[1 + nand_offset] = time_date.mm;
	NAND_packet[2 + nand_offset] = (time_date.ss)-samples_numb+i; 
	//Channels
	NAND_packet[3 + nand_offset] = pacchetto.luce_artificiale ;
	NAND_packet[4 + nand_offset] = pacchetto.blue;
	NAND_packet[5 + nand_offset] = pacchetto.deep_blue;
	NAND_packet[6 + nand_offset] = pacchetto.clear;
	}
}

extern uint16_t bad_blocks[1024];
extern uint16_t data_letto[2048];

void Debug_Read_And_Print_NAND(void) {
    printf("\n==================================================\n");
    printf("[NAND DEBUG] Inizio lettura NAND dopo Reset...\n");
    printf("==================================================\n");

    read_address_t debug_block;
    debug_block.dummy = 0;

    uint32_t total_packets_printed = 0;
    bool found_any_data = false;

    // Cicla su tutti i blocchi buoni
    for (int bloc_idx = 0; bloc_idx < 1024; bloc_idx++) {
        uint16_t current_physical_block = bad_blocks[bloc_idx];
        if (current_physical_block == 0xFFFF) {
            // Raggiunta la fine dei blocchi validi
            break;
        }

        debug_block.block = current_physical_block;

        for (int pag_idx = 0; pag_idx < 64; pag_idx++) {
            debug_block.page = pag_idx;

            // Legge la pagina fisica intera (4096 byte = 2048 uint16_t)
            int ret = spi_nand_page_read(debug_block, 0, (uint8_t*)data_letto, sizeof(data_letto));
            if (ret != SPI_NAND_RET_OK) {
                printf("[NAND DEBUG] Errore di lettura: Blocco %u, Pagina %u (Ret=%d)\n", 
                       current_physical_block, pag_idx, ret);
                continue;
            }

            // Verifica se la pagina è vuota (se il primo pacchetto è vuoto o se tutti i valori sono 0xFFFF)
            // Di solito, se la pagina è vuota, le prime parole sono 0xFFFF.
            if (data_letto[0] == 0xFFFF && data_letto[1] == 0xFFFF && data_letto[2] == 0xFFFF) {
                // Se la pagina corrente è vuota, assumiamo che le successive lo siano pure,
                // poiché scriviamo in modo sequenziale.
                goto fine_lettura;
            }

            found_any_data = true;
            printf("\n--- [NAND DEBUG] Blocco Fisico %u (Indice %d), Pagina %u ---\n", 
                   current_physical_block, bloc_idx, pag_idx);

            // Ogni pacchetto occupa 7 parole uint16_t (2048 / 7 = 292 campioni max per pagina)
            for (uint16_t offset = 0; offset <= 2041; offset += 7) {
                uint16_t hh = data_letto[offset + 0];
                uint16_t mm = data_letto[offset + 1];
                uint16_t ss = data_letto[offset + 2];
                uint16_t artificial_light = data_letto[offset + 3];
                uint16_t blue = data_letto[offset + 4];
                uint16_t deep_blue = data_letto[offset + 5];
                uint16_t clear = data_letto[offset + 6];

                // Se incontriamo un pacchetto vuoto (0xFFFF per timestamp) ci fermiamo
                if (hh == 0xFFFF && mm == 0xFFFF && ss == 0xFFFF) {
                    break;
                }

                printf("  [%05lu] Ora: %02u:%02u:%02u | Luce Artif: %u | Blue: %u | DeepBlue: %u | Clear: %u\n",
                       (unsigned long)total_packets_printed, hh, mm, ss, artificial_light, blue, deep_blue, clear);
                total_packets_printed++;
            }
        }
    }

fine_lettura:
    if (!found_any_data) {
        printf("[NAND DEBUG] Nessun dato presente in memoria (tutta vuota o 0xFFFF).\n");
    } else {
        printf("[NAND DEBUG] Lettura completata. Totale campioni letti: %lu\n", (unsigned long)total_packets_printed);
    }
    printf("==================================================\n\n");
}
