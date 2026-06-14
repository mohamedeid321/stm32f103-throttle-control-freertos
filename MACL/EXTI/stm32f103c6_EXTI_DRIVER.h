/**
 ******************************************************************************
 * @file    stm32f103c6_exti.h
 * @brief   External Interrupt (EXTI) Driver for STM32F103C6
 * @author  Mohamed
 * @note    Register-level driver (no HAL). Supports callbacks per line.
 ******************************************************************************
 */

#ifndef STM32F103C6_EXTI_H_
#define STM32F103C6_EXTI_H_

#include "stm32f103_regs.h"
#include "stm32f103c6_gpio.h"
#include <stdint.h>

/* ============================================================
 *                  1) Trigger Edge Selection
 *   Which edge on the pin should fire the interrupt.
 * ============================================================ */
typedef enum {
    EXTI_TRIGGER_RISING  = 0,   /* low -> high                 */
    EXTI_TRIGGER_FALLING = 1,   /* high -> low (active-low btn)*/
    EXTI_TRIGGER_BOTH    = 2    /* any change                  */
} EXTI_Trigger_t;

/* ============================================================
 *                  2) Port Selection (for AFIO->EXTICR)
 *   These values are what AFIO expects: A=0, B=1, C=2, D=3
 * ============================================================ */
typedef enum {
    EXTI_PORT_A = 0,
    EXTI_PORT_B = 1,
    EXTI_PORT_C = 2,
    EXTI_PORT_D = 3
} EXTI_Port_t;

/* ============================================================
 *                  3) Callback Function Type
 *   The user passes a function with this signature; the driver
 *   stores it and calls it from the ISR when the line fires.
 * ============================================================ */
typedef void (*EXTI_Callback_t)(void);

/* ============================================================
 *                  4) Function Prototypes
 * ============================================================ */

/**
 * @brief  Initialize an EXTI line for a given port/pin.
 *
 *         This single call does everything:
 *         - enables the AFIO clock
 *         - maps the EXTI line to the chosen port (AFIO->EXTICR)
 *         - selects the trigger edge (RTSR/FTSR)
 *         - unmasks the interrupt (IMR)
 *         - enables the matching IRQ in the NVIC
 *         - stores the user callback
 *
 * @param  port     : EXTI_PORT_A .. EXTI_PORT_D
 * @param  pin      : pin number 0..15 (also the EXTI line number)
 * @param  trigger  : rising / falling / both
 * @param  callback : function to call when the interrupt fires
 *                    (pass NULL if you don't need one)
 */
void EXTI_Init(EXTI_Port_t port, GPIO_Pin_t pin,
               EXTI_Trigger_t trigger, EXTI_Callback_t callback);

/**
 * @brief  Disable an EXTI line (mask it again).
 * @param  pin : the line number 0..15
 */
void EXTI_Disable(GPIO_Pin_t pin);

#endif /* STM32F103C6_EXTI_H_ */
