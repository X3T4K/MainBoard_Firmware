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
uint32_t last_peak_time = 0;
uint8_t heavy_noise_zone_flag = 0;

/**
 * @brief Start microphone peak detection
 */
void start_peak_detection(void)
{
    HAL_StatusTypeDef status;
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

void stop_peak_detection(void)
{
    HAL_StatusTypeDef status;
    // Ferma il monitoraggio della soglia (Over-Limit Detector) sul Filtro 1 in modalità interrupt
    status = HAL_MDF_OldStop_IT(&MdfHandle1);
    if (status != HAL_OK)
    {
        printf("ERROR: MDF OldStop_IT failed! Status: %d\r\n", status);
        Error_Handler();
    }
    else
    {
        printf("DEBUG: MDF OldStop_IT stopped successfully.\r\n");
    }
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
        
        dbspl_value = dbfs_value - MIC_DIGITAL_SENSITIVITY_DBFS + 94.0f;
        if (dbspl_value < 0.0f) dbspl_value = 0.0f; // Limit representation to positive dBSPL
    } else {
        dbspl_value = 0.0f;
    }
    return dbspl_value;
}

/**
 * @brief Start continuous acquisition for periodic monitoring
 * @note This function configures the MDF Filter 0 for continuous acquisition.
 */
void start_continuous_acquisition(void){

    MDF_DmaConfigTypeDef mdfDmaConfig0 = {0};
    mdfDmaConfig0.Address    = (uint32_t)&audio_buffer_acq[0];
    mdfDmaConfig0.DataLength = AUDIO_SAMPLES * sizeof(audio_buffer_acq[0]);
    mdfDmaConfig0.MsbOnly    = DISABLE;

    // Impostiamo l'acquisizione in modalità sincrona continua: il trigger di TIM1 avvia la conversione
    // che prosegue ad alta frequenza fino al riempimento del buffer DMA (512 campioni).
    MdfFilterConfig0.AcquisitionMode = MDF_MODE_SYNC_CONT;

    HAL_StatusTypeDef status = HAL_MDF_AcqStart_DMA(&MdfHandle0, &MdfFilterConfig0, &mdfDmaConfig0);
    if (status != HAL_OK)
    {
        printf("ERROR: MDF AcqStart (Filter 0) failed! Status: %d\r\n", status);
        Error_Handler();
    }
    else
    {
        printf("DEBUG: MDF AcqStart (Filter 0) active for continuous acquisition.\r\n");
    }

    if(HAL_TIM_Base_Start(&htim1) != HAL_OK)
    {
        printf("ERROR: TIM1 start failed!\r\n");
        Error_Handler();
    }
    else
    {
        printf("DEBUG: TIM1 started successfully for periodic acquisition.\r\n");
    }
}

void stop_continuous_acquisition(void){

    // Stop the DMA acquisition on Filter 0
    HAL_StatusTypeDef status = HAL_MDF_AcqStop(&MdfHandle0);
    if (status != HAL_OK)
    {
        printf("ERROR: MDF AcqStop (Filter 0) failed! Status: %d\r\n", status);
        Error_Handler();
    }
    else
    {
        printf("DEBUG: MDF AcqStop (Filter 0) stopped successfully.\r\n");
    }

    // Stop the timer TIM1
    if(HAL_TIM_Base_Stop(&htim1) != HAL_OK)
    {
        printf("ERROR: TIM1 stop failed!\r\n");
        Error_Handler();
    }
    else
    {
        printf("DEBUG: TIM1 stopped successfully for periodic acquisition.\r\n");
    }
   }

/**
 * @brief Stampa la diagnostica dei picchi acustici per la fascia DIURNA.
 * Soglia minima di attenzione: 65 dBSPL (Traffico/Folla).
 */
void Mic_AnalyzePeak_Daytime(float_t dbspl_val)
{
    if (dbspl_val >= 65.0f)
    {
        printf("\r\n--- [MONITORAGGIO DIURNO: RUMORE IMPULSIVO] ---\r\n");
        printf("Intensita' rilevata: %.2f dBSPL\r\n", dbspl_val);

        if (dbspl_val >= 140.0f)
        {
            printf(">>> [PERICOLO ESTREMO] Picco shock oltre 140 dBSPL! Rischio trauma immediato. <<<\r\n");
        }
        else if (dbspl_val >= 120.0f)
        {
            printf(">> [ALLERTA CRITICA] Superata la soglia del dolore (>=120 dBSPL). <<\r\n");
        }
        else if (dbspl_val >= 85.0f)
        {
            printf("> [ATTENZIONE] Livello rischioso per esposizioni prolungate (Soglia OSHA >=85 dBSPL). <\r\n");
        }
        else
        {
            printf("[INFO] Picco acustico moderato.\r\n");
        }
        printf("-----------------------------------------------\r\n\n");
    }
    else
    {
        printf("[DEBUG-DAY] Picco ignorato (%.2f dBSPL < 65 dBSPL)\r\n", dbspl_val);
    }
}

/**
 * @brief Stampa la diagnostica dei picchi acustici per la fascia NOTTURNA.
 * Soglia minima di attenzione abbassata a 45 dBSPL (Disturbo del sonno).
 */
void Mic_AnalyzePeak_Nighttime(float_t dbspl_val)
{
    // Di notte abbassiamo la soglia a 45 dBSPL perché il silenzio di fondo è maggiore 
    // e i rumori improvvisi svegliano l'utente o alterano il ritmo circadiano.
    if (dbspl_val >= 45.0f)
    {
        printf("\r\n--- [MONITORAGGIO NOTTURNO: DISTURBO SONNO] ---\r\n");
        printf("Intensita' rilevata: %.2f dBSPL\r\n", dbspl_val);

        if (dbspl_val >= 120.0f)
        {
            printf(">>> [ALLERTA CRITICA NOTTURNA] Picco estremo (>=120 dBSPL) in orario di riposo! <<<\r\n");
        }
        else if (dbspl_val >= 85.0f)
        {
            printf(">> [GRAVE DISTURBO] Rumore sopra i 85 dBSPL. <<\r\n");
        }
        else if (dbspl_val >= 60.0f)
        {
            printf("> [DISTURBO] Rumore sopra i 60 dBSPL: forte frammentazione del sonno garantita. <\r\n");
        }
        else // Tra 45.0f e 59.99f
        {
            printf("[ATTENZIONE] Micro-risveglio o alterazione della fase REM.\r\n");
        }
        printf("-----------------------------------------------\r\n\n");
    }
    else
    {
        printf("[DEBUG-NIGHT] Sonno protetto. Picco sotto la soglia di disturbo (%.2f dBSPL)\r\n", dbspl_val);
    }
}


/**
 * @brief Gestisce la frequenza dei trigger per evitare sovraccarichi in ambienti rumorosi.
 * @return 1 se l'evento deve essere elaborato, 0 se siamo in regime di protezione energetica.
 */
uint8_t Mic_ApplyCooldownProtection(void)
{
    uint32_t current_time = HAL_GetTick(); // Ottiene i millisecondi correnti
    
    // Se l'ultimo picco è avvenuto meno di 2 secondi fa
    if ((current_time - last_peak_time) < 2000) 
    {
        if (!heavy_noise_zone_flag)
        {
            heavy_noise_zone_flag = 1;
            printf(">>> [MODALITA' PROTENZIONE] Trigger OLD troppo frequenti. L'utente si trova in una 'Zona ad Alto Rumore Costante'. Disattivazione log impulsivi per risparmio energetico. <<<\r\n");
        }
        last_peak_time = current_time;
        return 0; // Salta l'elaborazione pesante, non stampare e non scrivere in flash
    }
    
    // Se è passato abbastanza tempo, resetta la protezione
    heavy_noise_zone_flag = 0;
    last_peak_time = current_time;
    return 1;
}