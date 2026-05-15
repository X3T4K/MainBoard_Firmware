#include "Spec_AS7341.h"
#include "i2c.h" // Per avere accesso alla variabile hi2c3

// Implementazione della scrittura su registro I2C
void SPEC_WriteRegister(uint8_t reg, uint8_t value) {
    // hi2c3 è l'handle dell'I2C, SPEC_I2C_ADDR è l'indirizzo a 8-bit definito nell'header
    HAL_I2C_Mem_Write(&hi2c3, SPEC_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, HAL_MAX_DELAY);
}

// Implementazione della lettura da registro I2C
uint8_t SPEC_ReadRegister(uint8_t reg) {
    uint8_t value = 0;
    HAL_I2C_Mem_Read(&hi2c3, SPEC_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, HAL_MAX_DELAY);
    return value;
}
// 1. PRIMA MODIFICA: Rimettiamo 0x06 alla fine per collegare il Flicker all'ADC 5
const uint8_t smux_config_F1_F3_CLEAR_FLICKER[20] = {
    0x30, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, // Modificato byte 5
    0x50, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x30, // Modificato byte 13
    0x01, 0x50, 0x00, 0x06                          // <--- MODIFICATO (era 0x00, ora è 0x06)
};

void SPEC_Config_SMUX_F1_F3_CLEAR_FLICKER(void) {
    SPEC_WriteRegister(0xAF, 0x10);

    // Passiamo il nuovo array con il Flicker configurato
    HAL_I2C_Mem_Write(&hi2c3, SPEC_I2C_ADDR, 0x00, I2C_MEMADD_SIZE_8BIT, (uint8_t*)smux_config_F1_F3_CLEAR_FLICKER, 20, 100);
    
    SPEC_WriteRegister(SPEC_REG_ENABLE, 0x11);
    HAL_Delay(2);
}

void SPEC_Init(void) {
    SPEC_WriteRegister(SPEC_REG_ENABLE, 0x01); 
    HAL_Delay(5); 

    SPEC_Config_SMUX_F1_F3_CLEAR_FLICKER(); // Richiamiamo la nuova funzione SMUX

    // [ATTENZIONE - Leggi la nota in fondo su questo registro]
    SPEC_WriteRegister(SPEC_REG_IEN, 0x08);

    SPEC_WriteRegister(SPEC_REG_CFG8, 0x02); 
    SPEC_WriteRegister(SPEC_REG_CFG12, 0x01); 
    SPEC_WriteRegister(SPEC_REG_CFG0, 0x20); 
    SPEC_WriteRegister(SPEC_REG_WTIME, 0xFF); 

    SPEC_WriteRegister(0x81, 29); 
    SPEC_WriteRegister(0xCA, 0x57); 
    SPEC_WriteRegister(0xCB, 0x02); 

    SPEC_WriteRegister(SPEC_REG_SP_TH_L_L, 0x00); 
    SPEC_WriteRegister(SPEC_REG_SP_TH_L_H, 0x00);
    SPEC_WriteRegister(SPEC_REG_SP_TH_H_L, 0x40);
    SPEC_WriteRegister(SPEC_REG_SP_TH_H_H, 0x38); 
    SPEC_WriteRegister(SPEC_REG_PERS, 0x05); 

    // =========================================================
    // 2. SECONDA MODIFICA: Avviamo Spettrometro E Flicker!
    // =========================================================
    // SP_EN (0x02) + WEN (0x08) + PON (0x01) = 0x0B
    // Aggiungiamo FDEN (0x40) -> 0x0B + 0x40 = 0x4B
    SPEC_WriteRegister(SPEC_REG_ENABLE, 0x4B); 
}