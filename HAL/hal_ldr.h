/**
 ******************************************************************************
 * @file    hal_ldr.h
 * @brief   LDR (light sensor module, analog out) HAL driver
 * @author  Mohamed
 * @layer   HAL (uses hal_analog -> MCAL ADC)
 *
 * @note    Use the module's AO (analog out) pin into an ADC channel.
 *          Output is a light level 0..100 %.
 *
 *          POLARITY: depending on the module's voltage divider, the
 *          voltage may RISE or FALL with more light. If your readings
 *          feel inverted, build with:  #define LDR_INVERT 1
 ******************************************************************************
 */

#ifndef HAL_LDR_H_
#define HAL_LDR_H_

#include "hal_analog.h"
#include <stdint.h>

/* set to 1 if your module's voltage drops when light increases */
#ifndef LDR_INVERT
#define LDR_INVERT  0
#endif

/**
 * @brief  Register the LDR's ADC channel (e.g. 1 for PA1).
 */
void LDR_Init(uint8_t adc_channel);

/** @brief Raw reading 0..4095. */
uint16_t LDR_ReadRaw(void);

/** @brief Light level as a percentage 0..100. */
uint8_t LDR_ReadLightPercent(void);

#endif /* HAL_LDR_H_ */
