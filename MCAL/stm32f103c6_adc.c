/**
 ******************************************************************************
 * @file    stm32f103c6_adc.c
 * @brief   ADC Driver (Scan + Continuous + DMA) for STM32F103C6
 ******************************************************************************
 */

#include "stm32f103c6_adc.h"
#include "stm32f103c6_dma.h"

/* ============================================================
 *   Results buffer: the DMA writes each channel's value here.
 *   adc_results[i] = latest value of the i-th channel in the
 *   scan list. Volatile because the DMA changes it in background.
 * ============================================================ */
static volatile uint16_t adc_results[ADC_MAX_CHANNELS] = {0};
static uint8_t adc_num_channels = 0;

/* ============================================================
 *   Map a channel number to its GPIO pin (analog input).
 *   CH0-7 -> PA0..PA7, CH8-9 -> PB0..PB1, CH10-15 -> PC0..PC5.
 * ============================================================ */
static void ADC_ConfigChannelPin(uint8_t channel)
{
    if (channel <= 7) {
        RCC->APB2ENR |= (1U << 2);   /* GPIOA */
        GPIO_InitPin(GPIOA, (GPIO_Pin_t)channel, GPIO_MODE_INPUT_ANALOG);
    } else if (channel <= 9) {
        RCC->APB2ENR |= (1U << 3);   /* GPIOB */
        GPIO_InitPin(GPIOB, (GPIO_Pin_t)(channel - 8), GPIO_MODE_INPUT_ANALOG);
    } else if (channel <= 15) {
        RCC->APB2ENR |= (1U << 4);   /* GPIOC */
        GPIO_InitPin(GPIOC, (GPIO_Pin_t)(channel - 10), GPIO_MODE_INPUT_ANALOG);
    }
}

/* set the 3-bit sample time of one channel (SMPR1/SMPR2) */
static void ADC_SetSampleTime(uint8_t channel, ADC_SampleTime_t st)
{
    if (channel <= 9) {
        uint32_t shift = (uint32_t)channel * 3U;
        ADC1->SMPR2 &= ~(0x7U << shift);
        ADC1->SMPR2 |=  ((uint32_t)st << shift);
    } else {
        uint32_t shift = (uint32_t)(channel - 10) * 3U;
        ADC1->SMPR1 &= ~(0x7U << shift);
        ADC1->SMPR1 |=  ((uint32_t)st << shift);
    }
}

/* ============================================================
 *   Write the channel order into the SQR registers.
 *   SQR3: conversions 1..6  (5 bits each)
 *   SQR2: conversions 7..12
 *   SQR1: conversions 13..16 + the length L (bits 20:23)
 * ============================================================ */
static void ADC_SetSequence(const uint8_t *channels, uint8_t count)
{
    ADC1->SQR1 = 0;
    ADC1->SQR2 = 0;
    ADC1->SQR3 = 0;

    for (uint8_t i = 0; i < count; i++)
    {
        if (i < 6)
            ADC1->SQR3 |= ((uint32_t)channels[i] << (i * 5));
        else if (i < 12)
            ADC1->SQR2 |= ((uint32_t)channels[i] << ((i - 6) * 5));
        else
            ADC1->SQR1 |= ((uint32_t)channels[i] << ((i - 12) * 5));
    }

    /* L = number of conversions - 1, in bits 20:23 of SQR1 */
    ADC1->SQR1 |= ((uint32_t)(count - 1) << 20);
}

/* ============================================================
 *                    DMA setup for ADC
 *   ADC1 is hardwired to DMA1 channel 1. We now use the generic
 *   DMA driver instead of poking the registers here directly.
 *   Peripheral = &ADC1->DR, Memory = adc_results[], circular.
 * ============================================================ */
static void ADC_DMA_Init(uint8_t count)
{
    /* stop the channel first — safe to call even if it wasn't running */
    DMA_Stop(DMA_CHANNEL_1);

    DMA_Config_t d = DMA_GetDefaultConfig();
    d.channel          = DMA_CHANNEL_1;            /* ADC1 -> channel 1 */
    d.direction        = DMA_PERIPH_TO_MEMORY;     /* read from ADC     */
    d.peripheral_addr  = (uint32_t)&ADC1->DR;      /* ADC data register */
    d.memory_addr      = (uint32_t)adc_results;    /* results buffer    */
    d.count            = count;                    /* one per channel   */
    d.periph_size      = DMA_SIZE_16BIT;           /* ADC result 16-bit */
    d.memory_size      = DMA_SIZE_16BIT;
    d.periph_increment = 0;                        /* DR is fixed       */
    d.mem_increment    = 1;                        /* buffer advances   */
    d.circular         = 1;                        /* keep refreshing   */
    d.priority         = DMA_PRIORITY_HIGH;        /* ADC is time-sensitive */
    d.callback         = 0;                        /* no interrupt needed */

    DMA_Init(&d);
    DMA_Start(DMA_CHANNEL_1);
}

/* ============================================================
 *                    ADC_GetDefaultConfig
 * ============================================================ */
ADC_Config_t ADC_GetDefaultConfig(void)
{
    ADC_Config_t cfg;
    cfg.channels[0]   = 0;            /* CH0 = PA0 */
    cfg.channel_count = 1;
    cfg.sample_time   = ADC_SAMPLE_55_5;
    return cfg;
}

/* ============================================================
 *                    ADC_Init
 * ============================================================ */
void ADC_Init(const ADC_Config_t *cfg)
{
    adc_num_channels = cfg->channel_count;

    /* ---- 1) enable ADC1 clock (APB2 bit 9) ---- */
    RCC->APB2ENR |= (1U << 9);

    /* ---- 2) configure each channel's pin + sample time ---- */
    for (uint8_t i = 0; i < cfg->channel_count; i++) {
        ADC_ConfigChannelPin(cfg->channels[i]);
        ADC_SetSampleTime(cfg->channels[i], cfg->sample_time);
    }

    /* ---- 3) SCAN mode (CR1 bit 8) ---- */
    ADC1->CR1 |= (1U << 8);          /* SCAN */

    /* ---- 4) CONTINUOUS + DMA + right align (CR2) ---- */
    ADC1->CR2 |= (1U << 1);          /* CONT: continuous          */
    ADC1->CR2 |= (1U << 8);          /* DMA: enable ADC DMA mode  */
    ADC1->CR2 &= ~(1U << 11);        /* right alignment           */

    /* ---- 5) build the channel sequence ---- */
    ADC_SetSequence(cfg->channels, cfg->channel_count);

    /* ---- 6) DMA setup ---- */
    ADC_DMA_Init(cfg->channel_count);

    /* ---- 7) power on + calibrate ----
     *   Reset procedure (safe to call even if ADC was already running):
     *   a) turn off (ADON=0)
     *   b) reset calibration registers (RSTCAL)
     *   c) wait RSTCAL clears
     *   d) turn on (ADON=1), short delay
     *   e) start calibration (CAL=1), wait with timeout
     */
    ADC1->CR2 &= ~(1U << 0);          /* ADON = 0: power off first   */
    ADC1->CR2 |=  (1U << 3);          /* RSTCAL: reset cal registers */
    uint32_t t = 100000;
    while ((ADC1->CR2 & (1U << 3)) && --t); /* wait RSTCAL clears   */

    ADC1->CR2 |= (1U << 0);           /* ADON: power on              */
    for (volatile int i = 0; i < 10000; i++);

    ADC1->CR2 |= (1U << 2);           /* CAL: start calibration      */
    t = 100000;
    while ((ADC1->CR2 & (1U << 2)) && --t); /* wait with timeout     */

    /* ---- 8) start conversions (write ADON again + SWSTART) ---- */
    ADC1->CR2 |= (1U << 0);           /* ADON */
    ADC1->CR2 |= (1U << 22);          /* SWSTART */
}

/* ============================================================
 *                    ADC_Read
 *   index = position in the scan list (0 = first channel).
 * ============================================================ */
uint16_t ADC_Read(uint8_t index)
{
    if (index >= adc_num_channels) return 0;
    return adc_results[index];
}

/* ============================================================
 *                    ADC_ToMilliVolts
 * ============================================================ */
uint32_t ADC_ToMilliVolts(uint16_t raw)
{
    return ((uint32_t)raw * ADC_VREF_MV) / ADC_MAX_VALUE;
}

/* ============================================================
 *                    ADC_ToPWM
 *   Scale 0..4095 (ADC) onto 0..pwm_max (PWM).
 *   pwm_value = raw * pwm_max / 4095   (integer, no float).
 *   The 32-bit product avoids overflow (4095*1000 fits easily).
 * ============================================================ */
uint16_t ADC_ToPWM(uint16_t raw, uint16_t pwm_max)
{
    return (uint16_t)(((uint32_t)raw * pwm_max) / ADC_MAX_VALUE);
}
