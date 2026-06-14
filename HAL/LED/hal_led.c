/**
 ******************************************************************************
 * @file    hal_led.c
 * @brief   LED HAL driver implementation
 ******************************************************************************
 */

#include "hal_led.h"

/* ============================================================
 *   LED_Init_Internal
 *   Called by LED_Create / LED_Create_Low macros.
 *   Configures the GPIO pin and sets the LED to OFF.
 *   Returns a ready-to-use LED_t struct.
 * ============================================================ */
LED_t LED_Init_Internal(GPIO_TypeDef *port, GPIO_Pin_t pin,
                        LED_Polarity_t polarity)
{
    LED_t led;
    led.port     = port;
    led.pin      = pin;
    led.polarity = polarity;

    GPIO_InitPin(port, pin, GPIO_MODE_OUTPUT_PP);

    /* start with LED off */
    if (polarity == LED_ACTIVE_HIGH)
        GPIO_CLEAR_PIN(port, pin);
    else
        GPIO_SET_PIN(port, pin);

    return led;
}

/* ============================================================
 *   Runtime controls
 * ============================================================ */
void LED_On(const LED_t *led)
{
    if (led->polarity == LED_ACTIVE_HIGH)
        GPIO_SET_PIN  (led->port, led->pin);
    else
        GPIO_CLEAR_PIN(led->port, led->pin);
}

void LED_Off(const LED_t *led)
{
    if (led->polarity == LED_ACTIVE_HIGH)
        GPIO_CLEAR_PIN(led->port, led->pin);
    else
        GPIO_SET_PIN  (led->port, led->pin);
}

void LED_Toggle(const LED_t *led)
{
    GPIO_TOGGLE_PIN(led->port, led->pin);
}

uint8_t LED_IsOn(const LED_t *led)
{
    uint8_t level = GPIO_ReadPin(led->port, led->pin);
    return (led->polarity == LED_ACTIVE_HIGH) ? level : (uint8_t)!level;
}
