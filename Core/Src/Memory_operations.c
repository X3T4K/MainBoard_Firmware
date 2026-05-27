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
