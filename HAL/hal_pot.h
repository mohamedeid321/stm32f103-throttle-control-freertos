/**
 ******************************************************************************
 * @file    hal_pot.h
 * @brief   Potentiometer HAL driver for STM32F103C6
 * @author  Mohamed
 * @layer   HAL (uses hal_analog -> MCAL ADC)
 *
 * @note    A 10K pot wired: one end 3.3V, other end GND, wiper to the
 *          ADC pin. Main use here: motor duty control, so we provide
 *          a 0..1000 output that feeds Timer_SetDutyRaw() directly
 *          (full resolution, no float).
 ******************************************************************************
 */

#ifndef HAL_POT_H_
#define HAL_POT_H_

#include "hal_analog.h"
#include <stdint.h>

/**
 * @brief  Register the pot's ADC channel (e.g. 2 for PA2).
 */
void POT_Init(uint8_t adc_channel);

/** @brief Raw reading 0..4095. */
uint16_t POT_ReadRaw(void);

/** @brief Position as a percentage 0..100. */
uint8_t POT_ReadPercent(void);

/**
 * @brief  Position scaled to 0..1000 — matches Timer_SetDutyRaw()
 *         (ARR=999 -> 1000 steps), so:
 *           Timer_SetDutyRaw(TIM3, ch, POT_ReadDuty1000());
 */
uint16_t POT_ReadDuty1000(void);

#endif /* HAL_POT_H_ */
