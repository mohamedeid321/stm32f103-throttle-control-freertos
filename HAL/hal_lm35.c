/**
 ******************************************************************************
 * @file    hal_lm35.c
 * @brief   LM35 HAL driver implementation
 ******************************************************************************
 */

#include "hal_lm35.h"

static uint8_t lm35_index = 0xFF;

void LM35_Init(uint8_t adc_channel)
{
    lm35_index = Analog_RegisterChannel(adc_channel);
}

uint16_t LM35_ReadTempCx10(void)
{
    if (lm35_index == 0xFF) return 0;

    /* LM35: 10 mV/°C  =>  temp_x10 == millivolts (see header) */
    return (uint16_t)Analog_ReadMilliVolts(lm35_index);
}

uint8_t LM35_ReadTempC(void)
{
    return (uint8_t)(LM35_ReadTempCx10() / 10U);
}

uint16_t LM35_ReadRaw(void)
{
    if (lm35_index == 0xFF) return 0;
    return Analog_Read(lm35_index);
}
