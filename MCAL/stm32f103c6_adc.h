/**
 ******************************************************************************
 * @file    stm32f103c6_adc.h
 * @brief   ADC Driver for STM32F103C6 (ADC1) — Scan + Continuous + DMA
 * @author  Mohamed
 * @note    Register-level driver (no HAL). 12-bit ADC.
 *
 *          DESIGN:
 *          - Always CONTINUOUS conversion (free-running).
 *          - SCAN mode: you give a list of channels; the ADC converts
 *            them in order, over and over.
 *          - DMA1 channel 1 copies each result into a results array
 *            automatically (needed because there is only one DR).
 *
 *          You just read the results array any time — it always holds
 *          the latest value of every channel you selected.
 ******************************************************************************
 */

#ifndef STM32F103C6_ADC_H_
#define STM32F103C6_ADC_H_

#include "stm32f103_regs.h"
#include "stm32f103c6_gpio.h"
#include <stdint.h>

/* Reference voltage in millivolts. Change to match your board. */
#ifndef ADC_VREF_MV
#define ADC_VREF_MV   3300U
#endif

#define ADC_MAX_VALUE     4095U   /* 12-bit                       */
#define ADC_MAX_CHANNELS  16U     /* max channels in one scan list */

/* ============================================================
 *                  Sample Time (per channel)
 *   Longer = more accurate, slower.
 * ============================================================ */
typedef enum {
    ADC_SAMPLE_1_5   = 0x0,
    ADC_SAMPLE_7_5   = 0x1,
    ADC_SAMPLE_13_5  = 0x2,
    ADC_SAMPLE_28_5  = 0x3,
    ADC_SAMPLE_41_5  = 0x4,
    ADC_SAMPLE_55_5  = 0x5,
    ADC_SAMPLE_71_5  = 0x6,
    ADC_SAMPLE_239_5 = 0x7
} ADC_SampleTime_t;

/* ============================================================
 *                  Configuration structure
 *   channels[]    : list of channel numbers to scan (0..15)
 *   channel_count : how many entries in channels[]
 *   sample_time   : applied to all listed channels
 * ============================================================ */
typedef struct {
    uint8_t          channels[ADC_MAX_CHANNELS];
    uint8_t          channel_count;
    ADC_SampleTime_t sample_time;
} ADC_Config_t;

/* ============================================================
 *                  Function Prototypes
 * ============================================================ */

/**
 * @brief  Default config: a single channel (0 / PA0), 55.5 cycles.
 *         Edit channels[]/channel_count for more channels.
 */
ADC_Config_t ADC_GetDefaultConfig(void);

/**
 * @brief  Initialize ADC1 in scan + continuous mode with DMA.
 *         After this, conversions run forever and the DMA keeps
 *         the results array up to date. Call ADC_Read() any time.
 */
void ADC_Init(const ADC_Config_t *cfg);

/**
 * @brief  Get the latest raw value (0..4095) of a channel by its
 *         INDEX in the scan list (0 = first channel you listed).
 */
uint16_t ADC_Read(uint8_t index);

/**
 * @brief  Convert a raw value to millivolts.
 */
uint32_t ADC_ToMilliVolts(uint16_t raw);

/**
 * @brief  Map a raw ADC value (0..4095) onto a PWM range (0..pwm_max).
 *         Integer math, no float. Typical pwm_max = 1000 (ARR=999).
 *         Example: ADC 2048 with pwm_max 1000 -> ~500.
 *
 * @param  raw      : ADC reading 0..4095
 * @param  pwm_max  : top of the PWM range (e.g. 1000)
 * @return value scaled into 0..pwm_max
 */
uint16_t ADC_ToPWM(uint16_t raw, uint16_t pwm_max);

#endif /* STM32F103C6_ADC_H_ */
