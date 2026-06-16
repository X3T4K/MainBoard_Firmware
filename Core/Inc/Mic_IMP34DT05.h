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

void Mic_Start(void);
void Mic_Stop(void);

float Calculate_dB(int32_t *buffer, uint16_t size); 

