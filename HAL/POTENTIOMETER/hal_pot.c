/**
 ******************************************************************************
 * @file    hal_pot.c
 * @brief   Potentiometer HAL driver implementation
 ******************************************************************************
 */

#include "hal_pot.h"

static uint8_t pot_index = 0xFF;     /* index in the ADC scan list */

void POT_Init(uint8_t adc_channel)
{
    pot_index = Analog_RegisterChannel(adc_channel);
}

uint16_t POT_ReadRaw(void)
{
    if (pot_index == 0xFF) return 0;
    return Analog_Read(pot_index);
}

uint8_t POT_ReadPercent(void)
{
    /* 0..4095 -> 0..100 (integer) */
    return (uint8_t)(((uint32_t)POT_ReadRaw() * 100U) / ADC_MAX_VALUE);
}

uint16_t POT_ReadDuty1000(void)
{
    /* 0..4095 -> 0..1000 for Timer_SetDutyRaw (full PWM resolution) */
    return ADC_ToPWM(POT_ReadRaw(), 1000U);
}
