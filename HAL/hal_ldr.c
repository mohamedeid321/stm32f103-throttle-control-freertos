/**
 ******************************************************************************
 * @file    hal_ldr.c
 * @brief   LDR HAL driver implementation
 ******************************************************************************
 */

#include "hal_ldr.h"

static uint8_t ldr_index = 0xFF;

void LDR_Init(uint8_t adc_channel)
{
    ldr_index = Analog_RegisterChannel(adc_channel);
}

uint16_t LDR_ReadRaw(void)
{
    if (ldr_index == 0xFF) return 0;
    return Analog_Read(ldr_index);
}

uint8_t LDR_ReadLightPercent(void)
{
    uint8_t pct = (uint8_t)(((uint32_t)LDR_ReadRaw() * 100U) / ADC_MAX_VALUE);

#if LDR_INVERT
    pct = (uint8_t)(100U - pct);
#endif

    return pct;
}
