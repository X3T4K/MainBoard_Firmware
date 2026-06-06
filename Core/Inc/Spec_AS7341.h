#ifndef SPEC_H
#define SPEC_H

#include "main.h" // Per avere accesso ai tipi base di STM32

/* L'indirizzo I2C a 7-bit del datasheet è 0x39. 
 * Le librerie HAL di STM32 vogliono l'indirizzo a 8-bit, quindi lo shiftiamo a sinistra di 1 */
#define SPEC_I2C_ADDR  (0x39 << 1)

/* --- Mappa dei Registri Principali (dal datasheet) --- */
#define SPEC_REG_ENABLE    0x80
#define SPEC_REG_ATIME     0x81
#define SPEC_REG_WTIME     0x83
#define SPEC_REG_SP_TH_L_L 0x84
#define SPEC_REG_SP_TH_L_H 0x85
#define SPEC_REG_SP_TH_H_L 0x86
#define SPEC_REG_SP_TH_H_H 0x87
#define SPEC_REG_CFG0      0xA9
#define SPEC_REG_CFG12     0xB5
#define SPEC_REG_PERS      0xBD
#define SPEC_REG_IEN       0xF9
#define SPEC_REG_STATUS    0x93
#define SPEC_REG_CFG8      0xB1
#define SPEC_REG_STATUS3   0xA4



// ... aggiungerai gli altri man mano che ti servono

/* --- Prototipi delle funzioni --- */
void SPEC_Init(void);
void SPEC_WriteRegister(uint8_t reg, uint8_t value);
uint8_t SPEC_ReadRegister(uint8_t reg);

#endif /* SPEC_H */