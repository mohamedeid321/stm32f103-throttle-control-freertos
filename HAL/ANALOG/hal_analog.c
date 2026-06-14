/**
 ******************************************************************************
 * @file    hal_analog.c
 * @brief   Shared analog manager implementation
 *
 *   Key fix: registration and ADC start are now SEPARATE steps.
 *   RegisterChannel() only builds the channel list.
 *   Start() initialises the ADC once with the complete list.
 *   This eliminates the race condition where re-initing the ADC
 *   mid-scan scrambled the DMA buffer order.
 ******************************************************************************
 */

#include "hal_analog.h"

static uint8_t channels[ADC_MAX_CHANNELS];
static uint8_t count   = 0;
static uint8_t started = 0;    /* 1 after Analog_Start() */

/* ============================================================
 *   Analog_RegisterChannel
 *   Only records the channel number — does NOT touch hardware.
 * ============================================================ */
uint8_t Analog_RegisterChannel(uint8_t adc_channel)
{
    if (count >= ADC_MAX_CHANNELS)
        return 0xFF;

    /* already registered? return existing index */
    for (uint8_t i = 0; i < count; i++)
        if (channels[i] == adc_channel)
            return i;

    channels[count] = adc_channel;
    return count++;             /* return index, then increment */
}

/* ============================================================
 *   Analog_Start
 *   Call once, after ALL sensors have called RegisterChannel.
 * ============================================================ */
void Analog_Start(void)
{
    if (count == 0) return;     /* nothing registered */

    ADC_Config_t cfg = ADC_GetDefaultConfig();
    for (uint8_t i = 0; i < count; i++)
        cfg.channels[i] = channels[i];
    cfg.channel_count  = count;
    cfg.sample_time    = ADC_SAMPLE_55_5;

    ADC_Init(&cfg);
    started = 1;
}

/* ============================================================
 *   Read helpers
 * ============================================================ */
uint16_t Analog_Read(uint8_t index)
{
    if (!started || index >= count) return 0;
    return ADC_Read(index);
}

uint32_t Analog_ReadMilliVolts(uint8_t index)
{
    return ADC_ToMilliVolts(Analog_Read(index));
}
