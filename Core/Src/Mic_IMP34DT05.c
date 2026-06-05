/**
 * @file Mic_IMP34DT05.c
 * @brief Microphone driver implementation
 */

#include "Mic_IMP34DT05.h"
#include <math.h>
#include "mdf.h"
#include "tim.h"

/* Variabili globali per gestione audio */
int32_t audio_buffer[AUDIO_SAMPLES];
float_t rms_value;
float_t dbfs_value;
float_t dbspl_value;

/**
 * @brief Start microphone acquisition trigger and threshold detection
 */
void Mic_Start(void)
{
    // Avvia il timer TIM1 che genera il clock per il microfono PDM (CCK0)
    if (HAL_TIM_Base_Start(&htim1) != HAL_OK)
    {
        Error_Handler();
    }
    
    // Avvia il monitoraggio della soglia (Over-Limit Detector) sul Filtro 1 in modalità interrupt
    if (HAL_MDF_OldStart_IT(&MdfHandle1, &mdfOldConfig1) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief Stop microphone acquisition trigger and threshold detection
 */
void Mic_Stop(void)
{
    // Ferma il monitoraggio della soglia
    HAL_MDF_OldStop_IT(&MdfHandle1);
    
    // Ferma il timer TIM1
    HAL_TIM_Base_Stop(&htim1);
}


/**
 * @brief Calculate dB from audio buffer energy
 * @param buffer: Pointer to audio samples buffer (32-bit MDF data)
 * @param size: Number of samples in buffer
 */
void Calculate_dB(int32_t *buffer, uint16_t size) 
{
    long long sum_sq = 0;
    
    for(int i = 0; i < size; i++) 
    {
        // Lo shift aritmetico rimane necessario per l'allineamento dell'MDF
        int32_t sample = buffer[i] >> 8; 
        sum_sq += (long long)sample * sample;
    }
    
    if (size > 0 && sum_sq > 0) {
        rms_value = sqrtf((float)sum_sq / size);
    } else {
        rms_value = 0.0f;
    }
    
    if (rms_value > 0.0f) {
        // Se MDF_MAX_VAL è corretto per il Sinc5, dbfs_value sarà perfetto
        dbfs_value = 20.0f * log10f(rms_value / MDF_MAX_VAL);
        if (dbfs_value > 0.0f) dbfs_value = 0.0f; 
    } else {
        dbfs_value = -100.0f;
    }

    if (dbfs_value > -100.0f) {
        // La conversione acustica dBSPL non cambia, perché si basa sulla proporzione lineare
        dbspl_value = dbfs_value - MIC_DIGITAL_SENSITIVITY_DBFS + 94.0f;
    } else {
        dbspl_value = 0.0f;
    }
}

