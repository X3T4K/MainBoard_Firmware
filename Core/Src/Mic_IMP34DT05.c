/**
 * @file Mic_IMP34DT05.c
 * @brief Microphone driver implementation
 */

#include "Mic_IMP34DT05.h"
#include <math.h>
#include "mdf.h"
#include "tim.h"
#include <stdio.h>

/* Variabili globali per gestione audio */
int32_t audio_buffer_peak[AUDIO_SAMPLES];
int32_t audio_buffer_acq[AUDIO_SAMPLES];
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
        printf("ERROR: TIM1 start failed!\r\n");
        Error_Handler();
    }
    else
    {
        printf("DEBUG: TIM1 started successfully.\r\n");
    }
    
    // Configura e avvia il Canale Filtro 1 in modalità polling (necessario per abilitare DFLTEN e applicare MdfFilterConfig1)
    HAL_StatusTypeDef status = HAL_MDF_AcqStart(&MdfHandle1, &MdfFilterConfig1);
    if (status != HAL_OK)
    {
        printf("ERROR: MDF AcqStart (Filter 1) failed! Status: %d\r\n", status);
        Error_Handler();
    }
    else
    {
        printf("DEBUG: MDF AcqStart (Filter 1) active.\r\n");
    }
    
    // Avvia il monitoraggio della soglia (Over-Limit Detector) sul Filtro 1 in modalità interrupt
    status = HAL_MDF_OldStart_IT(&MdfHandle1, &mdfOldConfig1);
    if (status != HAL_OK)
    {
        printf("ERROR: MDF OldStart_IT failed! Status: %d\r\n", status);
        Error_Handler();
    }
    else
    {
        printf("DEBUG: MDF OldStart_IT started successfully.\r\n");
    }
}

/**
 * @brief Stop microphone acquisition trigger and threshold detection
 */
void Mic_Stop(void)
{
    // Ferma il monitoraggio della soglia
    HAL_MDF_OldStop_IT(&MdfHandle1);
    
    // Ferma l'acquisizione sul Canale Filtro 1
    HAL_MDF_AcqStop(&MdfHandle1);
    
    // Ferma il timer TIM1
    HAL_TIM_Base_Stop(&htim1);
}


/**
 * @brief Calculate dB from audio buffer energy
 * @param buffer: Pointer to audio samples buffer (32-bit MDF data)
 * @param size: Number of samples in buffer
 */
float Calculate_dB(int32_t *buffer, uint16_t size) 
{
    float rms_value, dbfs_value, dbspl_value;
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
    return dbspl_value;
}

