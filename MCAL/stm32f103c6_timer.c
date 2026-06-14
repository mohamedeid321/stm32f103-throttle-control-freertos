/**
 ******************************************************************************
 * @file    stm32f103c6_timer.c
 * @brief   Full Timer Driver (separated base + channel) for STM32F103C6
 ******************************************************************************
 */

#include "stm32f103c6_timer.h"

/* ARR used when the base is set from a frequency (PWM/OC) */
#define TIMER_ARR_FIXED  999U

/* ============================================================
 *          Callback storage (one per timer)
 * ============================================================ */
static Timer_Callback_t tim2_cb = 0;
static Timer_Callback_t tim3_cb = 0;
static Timer_Callback_t tim4_cb = 0;

/* ============================================================
 *   Default GPIO pin for each timer/channel (no remap).
 * ============================================================ */
static GPIO_TypeDef* Timer_GetChannelPin(TIM_TypeDef *TIMx,
                                          Timer_Channel_t ch,
                                          GPIO_Pin_t *pin_out)
{
    if (TIMx == TIM2) {                       /* PA0..PA3 */
        *pin_out = (GPIO_Pin_t)(GPIO_PIN_0 + (ch - 1));
        return GPIOA;
    } else if (TIMx == TIM3) {                /* PA6,PA7,PB0,PB1 */
        if (ch == TIMER_CHANNEL_1) { *pin_out = GPIO_PIN_6; return GPIOA; }
        if (ch == TIMER_CHANNEL_2) { *pin_out = GPIO_PIN_7; return GPIOA; }
        if (ch == TIMER_CHANNEL_3) { *pin_out = GPIO_PIN_0; return GPIOB; }
        *pin_out = GPIO_PIN_1; return GPIOB;
    } else {                                  /* TIM4: PB6..PB9 */
        *pin_out = (GPIO_Pin_t)(GPIO_PIN_6 + (ch - 1));
        return GPIOB;
    }
}

static void Timer_EnableClock(TIM_TypeDef *TIMx)
{
    if      (TIMx == TIM2) RCC->APB1ENR |= (1U << 0);
    else if (TIMx == TIM3) RCC->APB1ENR |= (1U << 1);
    else if (TIMx == TIM4) RCC->APB1ENR |= (1U << 2);
}

static void Timer_EnableIRQ(TIM_TypeDef *TIMx)
{
    if      (TIMx == TIM2) NVIC_ENABLE_IRQ(TIM2_IRQn);
    else if (TIMx == TIM3) NVIC_ENABLE_IRQ(TIM3_IRQn);
    else if (TIMx == TIM4) NVIC_ENABLE_IRQ(TIM4_IRQn);
}

/* compute PSC & ARR for a period in ms (wide range) */
static void Timer_ComputePeriod(uint32_t period_ms,
                                uint16_t *psc_out, uint16_t *arr_out)
{
    uint64_t total = ((uint64_t)TIMER_CLOCK_HZ * period_ms) / 1000ULL;
    uint32_t psc = 0, arr;
    do {
        arr = (uint32_t)(total / (psc + 1U));
        if (arr == 0) arr = 1;
        arr -= 1U;
        if (arr <= 0xFFFFU) break;
        psc++;
    } while (psc <= 0xFFFFU);
    if (psc > 0xFFFFU) psc = 0xFFFFU;
    if (arr > 0xFFFFU) arr = 0xFFFFU;
    *psc_out = (uint16_t)psc;
    *arr_out = (uint16_t)arr;
}

/* ============================================================
 *                STEP 1a: base from frequency (PWM/OC)
 * ============================================================ */
void Timer_InitBaseFreq(TIM_TypeDef *TIMx, uint32_t frequency)
{
    Timer_EnableClock(TIMx);

    uint32_t psc = 0;
    if (frequency > 0)
        psc = (TIMER_CLOCK_HZ / (frequency * (TIMER_ARR_FIXED + 1U))) - 1U;

    TIMx->PSC = (uint16_t)psc;
    TIMx->ARR = TIMER_ARR_FIXED;
    TIMx->CR1 |= (1U << 7);     /* ARPE: auto-reload preload */

    /* load values now */
    TIMx->EGR |= (1U << 0);
    TIMx->SR  &= ~(1U << 0);
}

/* ============================================================
 *                STEP 1b: base from period in ms (OVERFLOW)
 * ============================================================ */
void Timer_InitBasePeriod(TIM_TypeDef *TIMx, uint32_t period_ms)
{
    Timer_EnableClock(TIMx);

    uint16_t psc, arr;
    Timer_ComputePeriod(period_ms, &psc, &arr);
    TIMx->PSC = psc;
    TIMx->ARR = arr;

    TIMx->EGR |= (1U << 0);
    TIMx->SR  &= ~(1U << 0);
}

/* ============================================================
 *          Internal: set a channel's mode field (CCMR)
 * ============================================================ */
static void Timer_SetChannelMode(TIM_TypeDef *TIMx, Timer_Channel_t ch,
                                 uint32_t ocxm, uint8_t preload)
{
    uint32_t field = (ocxm << 4);
    if (preload) field |= (1U << 3);

    if (ch == TIMER_CHANNEL_1) {
        TIMx->CCMR1 &= ~(0xFFU << 0); TIMx->CCMR1 |= (field << 0);
    } else if (ch == TIMER_CHANNEL_2) {
        TIMx->CCMR1 &= ~(0xFFU << 8); TIMx->CCMR1 |= (field << 8);
    } else if (ch == TIMER_CHANNEL_3) {
        TIMx->CCMR2 &= ~(0xFFU << 0); TIMx->CCMR2 |= (field << 0);
    } else {
        TIMx->CCMR2 &= ~(0xFFU << 8); TIMx->CCMR2 |= (field << 8);
    }
}

static void Timer_WriteCCR(TIM_TypeDef *TIMx, Timer_Channel_t ch, uint32_t val)
{
    switch (ch) {
        case TIMER_CHANNEL_1: TIMx->CCR1 = val; break;
        case TIMER_CHANNEL_2: TIMx->CCR2 = val; break;
        case TIMER_CHANNEL_3: TIMx->CCR3 = val; break;
        case TIMER_CHANNEL_4: TIMx->CCR4 = val; break;
    }
}

/* ============================================================
 *                STEP 2a: configure a channel as PWM
 * ============================================================ */
void Timer_ConfigChannelPWM(TIM_TypeDef *TIMx, Timer_Channel_t channel,
                            uint8_t duty)
{
    GPIO_Pin_t pin; GPIO_TypeDef *port;
    port = Timer_GetChannelPin(TIMx, channel, &pin);
    GPIO_InitPin(port, pin, GPIO_MODE_OUTPUT_AF_PP);

    /* PWM mode 1 (OCxM=110) + preload */
    Timer_SetChannelMode(TIMx, channel, 0x6, 1);

    /* duty -> CCR (ARR is whatever the base set; assume 999 for %) */
    uint32_t arr = TIMx->ARR;
    uint32_t ccr = ((uint32_t)duty * (arr + 1U)) / 100U;
    Timer_WriteCCR(TIMx, channel, ccr);

    /* enable output: CCxE = (ch-1)*4 */
    TIMx->CCER |= (1U << ((channel - 1) * 4));
}

/* ============================================================
 *                STEP 2b: configure a channel as Output Compare
 * ============================================================ */
void Timer_ConfigChannelOC(TIM_TypeDef *TIMx, Timer_Channel_t channel,
                           OC_Action_t action)
{
    GPIO_Pin_t pin; GPIO_TypeDef *port;
    port = Timer_GetChannelPin(TIMx, channel, &pin);
    GPIO_InitPin(port, pin, GPIO_MODE_OUTPUT_AF_PP);

    Timer_SetChannelMode(TIMx, channel, (uint32_t)action, 0);

    uint32_t arr = TIMx->ARR;
    Timer_WriteCCR(TIMx, channel, (arr + 1U) / 2U);

    TIMx->CCER |= (1U << ((channel - 1) * 4));
}

/* ============================================================
 *                STEP 3: overflow interrupt (combinable!)
 *   Fires once per period. Shares the ARR with any PWM channels,
 *   so the interrupt rate = the PWM frequency.
 * ============================================================ */
void Timer_EnableOverflowInterrupt(TIM_TypeDef *TIMx,
                                    Timer_Callback_t callback)
{
    if (TIMx == TIM2) tim2_cb = callback;
    else if (TIMx == TIM3) tim3_cb = callback;
    else if (TIMx == TIM4) tim4_cb = callback;

    /* clear any pending flag first, then enable */
    TIMx->SR  &= ~(1U << 0);
    TIMx->DIER |= (1U << 0);   /* UIE */
    Timer_EnableIRQ(TIMx);
}

/* ============================================================
 *                    Start / Stop
 * ============================================================ */
void Timer_Start(TIM_TypeDef *TIMx) { TIMx->CR1 |= (1U << 0); }
void Timer_Stop (TIM_TypeDef *TIMx) { TIMx->CR1 &= ~(1U << 0); }

/* ============================================================
 *                    Timer_SetDuty (runtime)
 * ============================================================ */
void Timer_SetDuty(TIM_TypeDef *TIMx, Timer_Channel_t channel, uint8_t duty)
{
    if (duty > 100) duty = 100;
    uint32_t arr = TIMx->ARR;
    uint32_t ccr = ((uint32_t)duty * (arr + 1U)) / 100U;
    Timer_WriteCCR(TIMx, channel, ccr);
}

/* ============================================================
 *                    Timer_SetDutyRaw (full resolution)
 *   value: 0 .. (ARR+1). With ARR=999 that means 0..1000.
 *   Writes the compare value straight into the CCR — no percent
 *   conversion — so you keep full resolution (e.g. from an ADC).
 * ============================================================ */
void Timer_SetDutyRaw(TIM_TypeDef *TIMx, Timer_Channel_t channel,
                      uint16_t value)
{
    uint32_t max = TIMx->ARR + 1U;     /* 1000 when ARR = 999 */
    if (value > max) value = (uint16_t)max;
    Timer_WriteCCR(TIMx, channel, value);
}

/* ============================================================
 *                    Interrupt Handlers
 *
 *   IMPORTANT: the ICU driver also defines TIMx_IRQHandler.
 *   If you use BOTH drivers in one project, you must disable
 *   the handler here for any timer the ICU owns, to avoid a
 *   duplicate-symbol linker error.
 *
 *   To disable a handler, define the matching macro in your
 *   build (or just above including this), e.g. for using TIM2
 *   as an ICU while TIM3 stays a normal timer:
 *
 *       #define TIMER_DISABLE_TIM2_IRQ
 *
 *   By default all are enabled.
 * ============================================================ */
#ifndef TIMER_DISABLE_TIM2_IRQ
void TIM2_IRQHandler(void)
{
    if (TIM2->SR & (1U << 0)) { TIM2->SR &= ~(1U << 0); if (tim2_cb) tim2_cb(); }
}
#endif

#ifndef TIMER_DISABLE_TIM3_IRQ
void TIM3_IRQHandler(void)
{
    if (TIM3->SR & (1U << 0)) { TIM3->SR &= ~(1U << 0); if (tim3_cb) tim3_cb(); }
}
#endif

#ifndef TIMER_DISABLE_TIM4_IRQ
void TIM4_IRQHandler(void)
{
    if (TIM4->SR & (1U << 0)) { TIM4->SR &= ~(1U << 0); if (tim4_cb) tim4_cb(); }
}
#endif
