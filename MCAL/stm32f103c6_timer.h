/**
 ******************************************************************************
 * @file    stm32f103c6_timer.h
 * @brief   Full Timer Driver for STM32F103C6 (TIM2 / TIM3 / TIM4)
 * @author  Mohamed
 * @note    Register-level driver (no HAL).
 *
 *          DESIGN (separated base + channel):
 *          --------------------------------------------------------
 *          1) Timer_InitBase()      -> sets PSC/ARR (the shared
 *                                      time-base / frequency) ONCE.
 *          2) Timer_ConfigChannel() -> configures ONE channel as
 *                                      PWM / Output-Compare. Call it
 *                                      for each channel you need.
 *          3) Timer_EnableOverflowInterrupt() -> optional, fires an
 *                                      interrupt every period. Can be
 *                                      combined with PWM channels
 *                                      because they share the same ARR.
 *          --------------------------------------------------------
 *          This lets you run several channels on one timer, and even
 *          OVERFLOW + PWM together (they share ARR, so same period).
 ******************************************************************************
 */

#ifndef STM32F103C6_TIMER_H_
#define STM32F103C6_TIMER_H_

#include "stm32f103_regs.h"
#include "stm32f103c6_gpio.h"
#include <stdint.h>

/* timer input clock: 8 MHz Proteus, 72 MHz typical real board */
#ifndef TIMER_CLOCK_HZ
#define TIMER_CLOCK_HZ   8000000UL
#endif
/* NOTE: if you add the ICU driver to a project, re-define the macro
 * for the timer the ICU owns (only that one) to avoid a duplicate
 * IRQ-handler symbol, e.g.:  #define TIMER_DISABLE_TIM2_IRQ
 * This project does not use the ICU, so all handlers stay enabled. */
/* ============================================================
 *                  Channel Selection
 * ============================================================ */
typedef enum {
    TIMER_CHANNEL_1 = 1,
    TIMER_CHANNEL_2 = 2,
    TIMER_CHANNEL_3 = 3,
    TIMER_CHANNEL_4 = 4
} Timer_Channel_t;

/* ============================================================
 *                  Channel Mode (per channel)
 * ============================================================ */
typedef enum {
    TIMER_CH_PWM            = 0,   /* PWM output                */
    TIMER_CH_OUTPUT_COMPARE = 1    /* toggle/set/clear on match */
} Timer_ChannelMode_t;

/* Output-compare action (OCxM) */
typedef enum {
    OC_ACTION_TOGGLE   = 0x3,
    OC_ACTION_ACTIVE   = 0x1,
    OC_ACTION_INACTIVE = 0x2
} OC_Action_t;

/* Overflow callback */
typedef void (*Timer_Callback_t)(void);

/* ============================================================
 *                  Function Prototypes
 * ============================================================ */

/**
 * @brief  Set up the timer time-base (PSC + ARR) from a frequency.
 *         ARR is fixed at 999 (1000 steps) and PSC computed from freq.
 *         Use this when you want PWM channels and/or a per-cycle
 *         overflow interrupt at this frequency.
 * @param  TIMx      : TIM2/3/4
 * @param  frequency : timebase / PWM frequency in Hz
 */
void Timer_InitBaseFreq(TIM_TypeDef *TIMx, uint32_t frequency);

/**
 * @brief  Set up the timer time-base from a PERIOD in milliseconds.
 *         Computes PSC and ARR to fit a wide range.
 *         Best for a pure OVERFLOW timer (no PWM).
 * @param  TIMx      : TIM2/3/4
 * @param  period_ms : overflow period in ms
 */
void Timer_InitBasePeriod(TIM_TypeDef *TIMx, uint32_t period_ms);

/**
 * @brief  Configure ONE channel as PWM. Call after Timer_InitBase*.
 *         Sets the GPIO pin (AF), PWM mode 1, duty, enables output.
 * @param  TIMx    : TIM2/3/4
 * @param  channel : 1..4
 * @param  duty    : 0..100 (%)
 */
void Timer_ConfigChannelPWM(TIM_TypeDef *TIMx, Timer_Channel_t channel,
                            uint8_t duty);

/**
 * @brief  Configure ONE channel as Output Compare.
 * @param  TIMx    : TIM2/3/4
 * @param  channel : 1..4
 * @param  action  : toggle / active / inactive
 */
void Timer_ConfigChannelOC(TIM_TypeDef *TIMx, Timer_Channel_t channel,
                           OC_Action_t action);

/**
 * @brief  Enable an overflow (update) interrupt on the timer.
 *         Fires once per period (same ARR as the PWM channels).
 *         Combine freely with PWM channels.
 * @param  TIMx     : TIM2/3/4
 * @param  callback : function called every period
 */
void Timer_EnableOverflowInterrupt(TIM_TypeDef *TIMx,
                                    Timer_Callback_t callback);

/** @brief Start the counter (CEN = 1). */
void Timer_Start(TIM_TypeDef *TIMx);

/** @brief Stop the counter (CEN = 0). */
void Timer_Stop(TIM_TypeDef *TIMx);

/** @brief Change a PWM channel duty (0..100%) at runtime. */
void Timer_SetDuty(TIM_TypeDef *TIMx, Timer_Channel_t channel, uint8_t duty);

/**
 * @brief  Set the PWM compare value directly (raw), range 0..1000
 *         (because ARR is fixed at 999 -> 1000 steps).
 *         Use this for full resolution, e.g. straight from an ADC.
 */
void Timer_SetDutyRaw(TIM_TypeDef *TIMx, Timer_Channel_t channel,
                      uint16_t value);

#endif /* STM32F103C6_TIMER_H_ */
