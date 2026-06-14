/**
 ******************************************************************************
 * @file    hal_led.h
 * @brief   LED HAL driver for STM32F103C6
 * @author  Mohamed
 * @layer   HAL (uses MCAL: GPIO only)
 *
 *   USAGE:
 *     // one line: define + init together
 *     LED_t motor   = LED_Create(GPIOB, GPIO_PIN_12);      // active-high
 *     LED_t onboard = LED_Create_Low(GPIOC, GPIO_PIN_13);  // active-low
 *
 *     // use by name
 *     LED_ON(motor);
 *     LED_OFF(onboard);
 *     LED_TOGGLE(motor);
 *     if (LED_IS_ON(motor)) { ... }
 ******************************************************************************
 */

#ifndef HAL_LED_H_
#define HAL_LED_H_

#include "stm32f103c6_gpio.h"
#include <stdint.h>

/* ---- polarity ---- */
typedef enum {
    LED_ACTIVE_HIGH = 0,
    LED_ACTIVE_LOW  = 1
} LED_Polarity_t;

/* ---- LED object ---- */
typedef struct {
    GPIO_TypeDef  *port;
    GPIO_Pin_t     pin;
    LED_Polarity_t polarity;
} LED_t;

/* ============================================================
 *   Create: define + init in one line
 * ============================================================ */

/* internal function — do not call directly */
LED_t LED_Init_Internal(GPIO_TypeDef *port, GPIO_Pin_t pin,
                        LED_Polarity_t polarity);

/* active-high (default for external LEDs) */
#define LED_Create(port, pin) \
    LED_Init_Internal((port), (pin), LED_ACTIVE_HIGH)

/* active-low (e.g. Blue Pill on-board LED on PC13) */
#define LED_Create_Low(port, pin) \
    LED_Init_Internal((port), (pin), LED_ACTIVE_LOW)

/* ============================================================
 *   Use by name
 * ============================================================ */
#define LED_ON(led)     LED_On    (&(led))
#define LED_OFF(led)    LED_Off   (&(led))
#define LED_TOGGLE(led) LED_Toggle(&(led))
#define LED_IS_ON(led)  LED_IsOn  (&(led))

/* ============================================================
 *   Functions (used by the macros above)
 * ============================================================ */
void    LED_On    (const LED_t *led);
void    LED_Off   (const LED_t *led);
void    LED_Toggle(const LED_t *led);
uint8_t LED_IsOn  (const LED_t *led);

#endif /* HAL_LED_H_ */
