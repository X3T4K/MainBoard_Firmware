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
#include "Memory_operations.h"

NAND_info data;
//void write_packet(uint8_t sample, RTC_TimeTypeDef sTime, uint8_t *magnetometer, uint8_t *pressure, uint8_t *temp, uint8_t *gyroscope,uint8_t *accelerometer, uint8_t *gyroscope2,uint8_t *accelerometer2,uint8_t *ppg_sample, uint8_t *NAND_packet);

// todo:
/*
 * All'inizio faccio un spi_nand_init(); che non cancella il contenuto della memoria
 * Il flusso di operazioni sarà: inizializzo, leggo tutto, identifico i bad blocks, erase tutto, scrivo tutto
 * Idea: uso il primo blocco disponibile per salvarmi il blocco e la pagina a cui sono arrivata -> può avere dei problemi.
 */

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
	for(int i = 0; i<2048; i++){
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
}

// Questo da tenere così
void erase_good_blocks(uint8_t *bad_blocks){
	read_address_t blocco;
	blocco.block=0;
	blocco.page=0;
	blocco.dummy=0;
	bool is_bad_mark=true;
	for(int i = 0; i<2048; i++){
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

void write_packet(uint16_t sample, Time_Struct timestamp, uint8_t *gyroscope, uint8_t *accelerometer, uint8_t *NAND_packet){

	NAND_packet[0 + (sample * BYTES_PER_SAMPLE)] = timestamp.hh;
	NAND_packet[1 + (sample * BYTES_PER_SAMPLE)] = timestamp.mm;
	NAND_packet[2 + (sample * BYTES_PER_SAMPLE)] = timestamp.ss;

	uint16_t milli = timestamp.sss;
	uint8_t m[2];
	m[0] = milli & 0xff;
	m[1] = milli >> 8;

	NAND_packet[3 + (sample * BYTES_PER_SAMPLE)] = m[0];
	NAND_packet[4 + (sample * BYTES_PER_SAMPLE)] = m[1];

	NAND_packet[5 + (sample * BYTES_PER_SAMPLE)] = accelerometer[0];
	NAND_packet[6 + (sample * BYTES_PER_SAMPLE)] = accelerometer[1];
	NAND_packet[7 + (sample * BYTES_PER_SAMPLE)] = accelerometer[2];
	NAND_packet[8 + (sample * BYTES_PER_SAMPLE)] = accelerometer[3];
	NAND_packet[9 + (sample * BYTES_PER_SAMPLE)] = accelerometer[4];
	NAND_packet[10 + (sample * BYTES_PER_SAMPLE)] = accelerometer[5];

	NAND_packet[11 + (sample * BYTES_PER_SAMPLE)] = gyroscope[0];
	NAND_packet[12 + (sample * BYTES_PER_SAMPLE)] = gyroscope[1];
	NAND_packet[13 + (sample * BYTES_PER_SAMPLE)] = gyroscope[2];
	NAND_packet[14 + (sample * BYTES_PER_SAMPLE)] = gyroscope[3];
	NAND_packet[15 + (sample * BYTES_PER_SAMPLE)] = gyroscope[4];
	NAND_packet[16 + (sample * BYTES_PER_SAMPLE)] = gyroscope[5];

}

void Debug_Write_Test_Page(void) {
	extern uint16_t bad_blocks[2048];
	
	// Cerca il primo blocco fisico sano
	uint16_t target_block = 0xFFFF;
	for (int i = 0; i < 2048; i++) {
		if (bad_blocks[i] != 0xFFFF && bad_blocks[i] < 2048) {
			target_block = bad_blocks[i];
			break;
		}
	}
	
	// Se la LUT dei bad_blocks è vuota o non inizializzata, prova ad inizializzarla adesso
	if (target_block == 0xFFFF) {
		printf("[NAND TEST] LUT bad_blocks non inizializzata. Eseguo find_bad_blocks...\n");
		find_bad_blocks(bad_blocks);
		for (int i = 0; i < 2048; i++) {
			if (bad_blocks[i] != 0xFFFF && bad_blocks[i] < 2048) {
				target_block = bad_blocks[i];
				break;
			}
		}
	}
	
	if (target_block == 0xFFFF) {
		printf("[NAND TEST] ERRORE: Nessun blocco sano trovato!\n");
		return;
	}
	
	printf("[NAND TEST] Scrittura pagina di test su Blocco Fisico %u, Pagina 0...\n", target_block);
	
	// Inizializza l'indirizzo riga: blocco e pagina 0
	read_address_t row;
	row.block = target_block;
	row.page = 0;
	row.dummy = 0;
	
	// Cancella il blocco prima di programmare
	int erase_ret = spi_nand_block_erase(row);
	if (erase_ret != SPI_NAND_RET_OK) {
		printf("[NAND TEST] Errore Erase Blocco %u: %d\n", target_block, erase_ret);
		return;
	}
	printf("[NAND TEST] Erase del blocco %u completato con successo.\n", target_block);
	
	// Alloca il buffer per la pagina (4096 byte)
	uint8_t test_page[4096];
	memset(test_page, 0xFF, sizeof(test_page)); // Riempie di 0xFF (padding per i byte non usati)
	
	// Struttura che rappresenta un singolo campione salvato in NAND (14 byte)
	// corrispondente a come salvava SP_Develop (7 valori uint16_t in Little Endian)
	typedef struct __attribute__((packed)) {
		uint16_t hh;
		uint16_t mm;
		uint16_t ss;
		uint16_t luce_artificiale;
		uint16_t blue;
		uint16_t deep_blue;
		uint16_t clear;
	} Test_Sample;
	
	Test_Sample *samples = (Test_Sample *)test_page;
	
	// Genera dati di test crescenti e facilmente riconoscibili
	uint8_t hour = 12;
	uint8_t minute = 30;
	uint8_t second = 0;
	
	// In 4096 byte ci stanno 292 campioni da 14 byte (292 * 14 = 4088 byte)
	for (int i = 0; i < 292; i++) {
		samples[i].hh = hour;
		samples[i].mm = minute;
		samples[i].ss = second;
		
		// Luce artificiale toggolata ogni 10 campioni per testare il flag flicker
		samples[i].luce_artificiale = (i % 10 == 0) ? 1 : 0;
		
		// Campioni di colore finti e crescenti
		samples[i].blue = 1000 + i;
		samples[i].deep_blue = 2000 + i * 2;
		samples[i].clear = 5000 + i * 3;
		
		// Incrementa il tempo di un secondo ad ogni campione
		second++;
		if (second >= 60) {
			second = 0;
			minute++;
			if (minute >= 60) {
				minute = 0;
				hour = (hour + 1) % 24;
			}
		}
	}
	
	// Esegue il page program hardware su colonna 0
	column_address_t colonna = 0;
	int write_ret = spi_nand_page_program(row, colonna, test_page, 4096);
	if (write_ret != SPI_NAND_RET_OK) {
		printf("[NAND TEST] Errore Program Pagina %u: %d\n", row.page, write_ret);
	} else {
		printf("[NAND TEST] Scrittura pagina di test completata con successo!\n");
	}
}

void Debug_Read_And_Print_NAND(void) {
    extern uint16_t bad_blocks[2048];
    extern uint8_t data_letto[4096];

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

            // Cast data_letto to uint16_t* to access 2-byte words correctly
            uint16_t *data_letto_u16 = (uint16_t *)data_letto;

            // Verifica se la pagina è vuota (se il primo pacchetto è vuoto o se tutti i valori sono 0xFFFF)
            // Di solito, se la pagina è vuota, le prime parole sono 0xFFFF.
            if (data_letto_u16[0] == 0xFFFF && data_letto_u16[1] == 0xFFFF && data_letto_u16[2] == 0xFFFF) {
                // Se la pagina corrente è vuota, assumiamo che le successive lo siano pure,
                // poiché scriviamo in modo sequenziale.
                goto fine_lettura;
            }

            found_any_data = true;
            printf("\n--- [NAND DEBUG] Blocco Fisico %u (Indice %d), Pagina %u ---\n", 
                   current_physical_block, bloc_idx, pag_idx);

            // Ogni pacchetto occupa 7 parole uint16_t (2048 / 7 = 292 campioni max per pagina)
            for (uint16_t offset = 0; offset <= 2041; offset += 7) {
                uint16_t hh = data_letto_u16[offset + 0];
                uint16_t mm = data_letto_u16[offset + 1];
                uint16_t ss = data_letto_u16[offset + 2];
                uint16_t artificial_light = data_letto_u16[offset + 3];
                uint16_t blue = data_letto_u16[offset + 4];
                uint16_t deep_blue = data_letto_u16[offset + 5];
                uint16_t clear = data_letto_u16[offset + 6];

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
