/**
 ******************************************************************************
 * @file    stm32f103c6_gpio.h
 * @brief   GPIO Driver for STM32F103C6 (Cortex-M3)
 * @author  Mohamed
 * @note    Register-level driver (no HAL) - Proteus compatible
 ******************************************************************************
 */

#ifndef STM32F103C6_GPIO_H_
#define STM32F103C6_GPIO_H_

#include "stm32f103_regs.h"   /* manual register definitions (no CMSIS) */
#include <stdint.h>

/* ============================================================
 *                       1) Pin Numbers
 * ============================================================ */
typedef enum {
    GPIO_PIN_0 = 0, GPIO_PIN_1,  GPIO_PIN_2,  GPIO_PIN_3,
    GPIO_PIN_4,     GPIO_PIN_5,  GPIO_PIN_6,  GPIO_PIN_7,
    GPIO_PIN_8,     GPIO_PIN_9,  GPIO_PIN_10, GPIO_PIN_11,
    GPIO_PIN_12,    GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15
} GPIO_Pin_t;

/* ============================================================
 *                       2) Pin Modes
 *   Each value is the ready 4-bit field (CNF[1:0] | MODE[1:0]).
 * ============================================================ */
typedef enum {
    /* ----- Input modes (MODE = 00) ----- */
    GPIO_MODE_INPUT_ANALOG     = 0x0,  /* 0000 */
    GPIO_MODE_INPUT_FLOATING   = 0x4,  /* 0100 */
    GPIO_MODE_INPUT_PULLUPDOWN = 0x8,  /* 1000 */

    /* ----- Output modes @2MHz (MODE = 10) ----- */
    GPIO_MODE_OUTPUT_PP        = 0x2,  /* 0010 push-pull        */
    GPIO_MODE_OUTPUT_OD        = 0x6,  /* 0110 open-drain       */
    GPIO_MODE_OUTPUT_AF_PP     = 0xA,  /* 1010 alt push-pull    */
    GPIO_MODE_OUTPUT_AF_OD     = 0xE   /* 1110 alt open-drain   */
} GPIO_Mode_t;

/* ============================================================
 *                  3) Pin State (for writes)
 * ============================================================ */
typedef enum {
    GPIO_PIN_LOW  = 0,
    GPIO_PIN_HIGH = 1
} GPIO_PinState_t;

/* ============================================================
 *        4) Fast macros (Set / Clear / Toggle / Check)
 *   Inline register access for clean and fast code.
 * ============================================================ */

/* atomic set/clear through BSRR */
#define GPIO_SET_PIN(PORT, PIN)      ((PORT)->BSRR = (1U << (PIN)))
#define GPIO_CLEAR_PIN(PORT, PIN)    ((PORT)->BSRR = (1U << ((PIN) + 16U)))

/* toggle through XOR on ODR */
#define GPIO_TOGGLE_PIN(PORT, PIN)   ((PORT)->ODR ^= (1U << (PIN)))

/* read / check */
#define GPIO_READ_PIN(PORT, PIN)     (((PORT)->IDR >> (PIN)) & 1U)
#define GPIO_CHECK_PIN(PORT, PIN)    (((PORT)->IDR & (1U << (PIN))) != 0U)

/* ============================================================
 *                    5) Function Prototypes
 * ============================================================ */

/**
 * @brief  Enable the clock of a GPIO port.
 * @param  GPIOx : GPIOA, GPIOB, GPIOC, GPIOD
 */
void GPIO_EnableClock(GPIO_TypeDef *GPIOx);

/**
 * @brief  Configure one pin (direction and type).
 * @param  GPIOx : the port
 * @param  pin   : pin number (GPIO_PIN_0 .. 15)
 * @param  mode  : a GPIO_Mode_t value
 */
void GPIO_InitPin(GPIO_TypeDef *GPIOx, GPIO_Pin_t pin, GPIO_Mode_t mode);

/**
 * @brief  Write HIGH or LOW on a pin (uses the safe BSRR inside).
 */
void GPIO_WritePin(GPIO_TypeDef *GPIOx, GPIO_Pin_t pin, GPIO_PinState_t state);

/**
 * @brief  Read the pin value.
 * @retval 0 or 1
 */
uint8_t GPIO_ReadPin(GPIO_TypeDef *GPIOx, GPIO_Pin_t pin);

/**
 * @brief  For GPIO_MODE_INPUT_PULLUPDOWN: choose pull direction.
 *         pull_up=1 -> pull-up (ODR=1), pull_up=0 -> pull-down (ODR=0).
 */
void GPIO_SetPull(GPIO_TypeDef *GPIOx, GPIO_Pin_t pin, uint8_t pull_up);

#endif /* STM32F103C6_GPIO_H_ */
