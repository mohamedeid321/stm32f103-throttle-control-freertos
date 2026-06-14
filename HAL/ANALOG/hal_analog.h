/**
 ******************************************************************************
 * @file    hal_analog.h
 * @brief   Shared analog manager (HAL helper) for STM32F103C6
 * @author  Mohamed
 * @layer   HAL (uses MCAL: ADC only)
 *
 * @note    HOW TO USE (correct pattern):
 *
 *          // 1) register ALL channels first (NO ADC yet)
 *          uint8_t pot_idx = Analog_RegisterChannel(0);
 *          uint8_t lm35_idx = Analog_RegisterChannel(1);
 *
 *          // 2) start the ADC once with the full list
 *          Analog_Start();
 *
 *          // 3) read any time
 *          uint16_t pot  = Analog_Read(pot_idx);
 *          uint16_t temp = Analog_Read(lm35_idx);
 *
 *          The sensor HALs (hal_pot, hal_lm35 ...) call
 *          Analog_RegisterChannel() in their Init functions.
 *          YOU call Analog_Start() once after all sensor Inits.
 ******************************************************************************
 */

#ifndef HAL_ANALOG_H_
#define HAL_ANALOG_H_

#include "stm32f103c6_adc.h"
#include <stdint.h>

/**
 * @brief  Register one ADC channel. Does NOT start the ADC.
 *         Safe to call multiple times for the same channel.
 * @return the INDEX of this channel in the scan list (0-based).
 *         Pass it to Analog_Read(). Returns 0xFF if list is full.
 */
uint8_t Analog_RegisterChannel(uint8_t adc_channel);

/**
 * @brief  Initialize and START the ADC with ALL registered channels.
 *         Call this ONCE after all sensor Init functions.
 */
void Analog_Start(void);

/** @brief Latest raw value (0..4095) of a channel by its scan index. */
uint16_t Analog_Read(uint8_t index);

/** @brief Latest value converted to millivolts. */
uint32_t Analog_ReadMilliVolts(uint8_t index);

#endif /* HAL_ANALOG_H_ */
