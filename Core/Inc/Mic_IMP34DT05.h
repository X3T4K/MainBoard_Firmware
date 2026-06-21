#ifndef MIC_IMP34DT05_H
#define MIC_IMP34DT05_H

#include "main.h" // Per avere accesso ai tipi base di STM32

/* Definizioni */
#define AUDIO_SAMPLES 512  // Numero di campioni per il calcolo

// Valori di calibrazione da aggiornare con i dati reali del microfono e dell'ADC
#define MIC_DIGITAL_SENSITIVITY_DBFS -26.0f    // riferimento SPL 0 dB = 20 µPa
#define MDF_MAX_VAL               8388608.0f // Valore massimo per un segnale a 24 bit (2^23)

/* Variabili globali */
extern int32_t audio_buffer_peak[AUDIO_SAMPLES];
extern int32_t audio_buffer_acq[AUDIO_SAMPLES];
extern float_t rms_value;
extern float_t dbfs_value;
extern float_t dbspl_value;

/* Prototipi delle funzioni */

void start_peak_detection(void);
void stop_peak_detection(void);

float Calculate_dB(int32_t *buffer, uint16_t size); 

void start_continuous_acquisition(void);
void stop_continuous_acquisition(void);

/**
 * @brief Stampa la diagnostica dei picchi acustici per la fascia DIURNA.
 */
void Mic_AnalyzePeak_Daytime(float_t dbspl_val);

/**
 * @brief Stampa la diagnostica dei picchi acustici per la fascia NOTTURNA.
 */
void Mic_AnalyzePeak_Nighttime(float_t dbspl_val);

/**
 * @brief Gestisce la frequenza dei trigger per evitare sovraccarichi in ambienti rumorosi (Cooldown).
 * @return 1 se l'evento deve essere elaborato, 0 se siamo in regime di protezione energetica.
 */
uint8_t Mic_ApplyCooldownProtection(void);

#endif /* MIC_IMP34DT05_H */
