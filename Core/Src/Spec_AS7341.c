#include "Spec_AS7341.h"
#include "i2c.h" // Per avere accesso alla variabile hi2c3

// Funzione base per scrivere 1 byte in un registro del sensore
void SPEC_WriteRegister(uint8_t reg, uint8_t value) {
    HAL_I2C_Mem_Write(&hi2c3, SPEC_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, HAL_MAX_DELAY);
}

// Funzione base per leggere 1 byte da un registro
uint8_t SPEC_ReadRegister(uint8_t reg) {
    uint8_t value = 0;
    HAL_I2C_Mem_Read(&hi2c3, SPEC_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, HAL_MAX_DELAY);
    return value;
}

// Array dei 20 byte per mappare F1(adc0), F2(adc1), F3(adc2), Clear(adc4) ai 6 ADC
// (Valori presi da "Configuration Example 1" del datasheet)
const uint8_t smux_config_F1_F3_CLEAR[20] = {
    0x30, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, // Modificato byte 5 (era 0x42 -> 0x02)
    0x50, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x30, // Modificato byte 13 (era 0x04 -> 0x00)
    0x01, 0x50, 0x00, 0x00                          // Modificato byte 19 (era 0x06 -> 0x00)
};

void SPEC_Config_SMUX_F1_F3_CLEAR(void) {
    
    // 2. Scriviamo nel registro CFG6 (0xAF) il valore 0x10 per dire allo SMUX 
    //    di prepararsi a caricare la configurazione dalla RAM[cite: 3]
    SPEC_WriteRegister(0xAF, 0x10);

    // 3. Scriviamo i 20 byte di configurazione nella RAM del sensore[cite: 3]
    // (L'indirizzo di partenza della RAM è 0x00[cite: 3])
    // Se la tua libreria I2C supporta la scrittura multipla (Mem_Write), puoi farlo in un colpo solo:
    HAL_I2C_Mem_Write(&hi2c3, SPEC_I2C_ADDR, 0x00, I2C_MEMADD_SIZE_8BIT, (uint8_t*)smux_config_F1_F3_CLEAR, 20, 100);
    

    // 4. Avviamo il comando SMUX mantenendo il power ON (SMUXEN=1, PON=1 -> 0x11)[cite: 3]
    SPEC_WriteRegister(SPEC_REG_ENABLE, 0x11);

    // 5. Aspettiamo che il sensore finisca di applicare i cablaggi interni[cite: 3]
    HAL_Delay(2); // Un paio di millisecondi sono più che sufficienti
}


// Funzione di Inizializzazione
void SPEC_Init(void) {
    // 1. Accendiamo il sensore (Power ON)
    SPEC_WriteRegister(SPEC_REG_ENABLE, 0x01); 
    HAL_Delay(5); 

    SPEC_Config_SMUX_F1_F3_CLEAR(); // Configuriamo lo SMUX per F1-F3 e Clear


    //Abilitiamo gli interrupt per i canali spettrali (SP) e il Wait Timer (WT)
    SPEC_WriteRegister(SPEC_REG_IEN, 0x03);

    //abilitiamo l'autogain per lo spettro
    SPEC_WriteRegister(SPEC_REG_CFG8, 0x02); 

    //registro per scegliere il canale su cui impostare la soglia
    SPEC_WriteRegister(SPEC_REG_CFG12, 0x01); 

    // 2. Abilitiamo il Low Power Idle 
    SPEC_WriteRegister(SPEC_REG_CFG0, 0x20); 

    // 3. Impostiamo il WTIME (Pausa tra misurazioni)
    SPEC_WriteRegister(SPEC_REG_WTIME, 0xFF); 

    // =========================================================
    // 3.5 IMPOSTAZIONE DEL TEMPO DI INTEGRAZIONE (ATIME e ASTEP)
    // Esempio per impostare un tempo di integrazione di ~50 ms:
    // Scegliamo ATIME = 29 e ASTEP = 599.
    // (29 + 1) * (599 + 1) * 2.78 us = 50.04 ms
    // =========================================================
    
    // ATIME è un registro a 8 bit (indirizzo 0x81)
    SPEC_WriteRegister(0x81, 29); 
    
    // ASTEP è a 16 bit, diviso in L (0xCA) e H (0xCB)
    // 599 in esadecimale è 0x0257
    SPEC_WriteRegister(0xCA, 0x57); // Byte Basso (L)
    SPEC_WriteRegister(0xCB, 0x02); // Byte Alto (H)


    //set della treshold per gi interupt 
    SPEC_WriteRegister(SPEC_REG_SP_TH_L_L, 0x00); 
    SPEC_WriteRegister(SPEC_REG_SP_TH_L_H, 0x00);
    SPEC_WriteRegister(SPEC_REG_SP_TH_H_L, 0x40);
    SPEC_WriteRegister(SPEC_REG_SP_TH_H_H, 0x38); 

    SPEC_WriteRegister(SPEC_REG_PERS, 0x05); // 5 cicli di conferma per gli interrupt

    // Puoi anche impostare il guadagno (GAIN) qui, indirizzo 0xAA
    // SPEC_WriteRegister(0xAA, 0x07); // Esempio: GAIN 128x

    // 4. Configura le soglie di interrupt qui (se usi l'interrupt pin)
    // ...

    // 5. Alla fine, avviamo il motore spettrale e il Wait Timer!
    SPEC_WriteRegister(SPEC_REG_ENABLE, 0x0B); 
}