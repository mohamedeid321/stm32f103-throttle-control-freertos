/**
 ******************************************************************************
 * @file    hal_lm35.h
 * @brief   LM35 temperature sensor HAL driver for STM32F103C6
 * @author  Mohamed
 * @layer   HAL (uses hal_analog -> MCAL ADC)
 *
 * @note    LM35 outputs 10 mV per °C (0°C = 0 mV, 25.0°C = 250 mV).
 *          So:  millivolts = temperature * 10
 *          ->   temperature x10 (fixed point) == millivolts directly!
 *
 *          We use integer fixed-point (x10) — no float:
 *            255 means 25.5 °C.
 *          Print it like:  send(t/10); send('.'); send(t%10);
 ******************************************************************************
 */

#ifndef HAL_LM35_H_
#define HAL_LM35_H_

#include "hal_analog.h"
#include <stdint.h>

/**
 * @brief  Register the LM35's ADC channel (e.g. 0 for PA0).
 */
void LM35_Init(uint8_t adc_channel);

/**
 * @brief  Temperature in tenths of °C (fixed point x10).
 *         Example: returns 255 for 25.5 °C.
 */
uint16_t LM35_ReadTempCx10(void);

/** @brief Temperature in whole °C (truncated). */
uint8_t LM35_ReadTempC(void);

/** @brief Raw ADC value 0..4095 (handy for debugging the wiring). */
uint16_t LM35_ReadRaw(void);

#endif /* HAL_LM35_H_ */
