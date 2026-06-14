/**
 ******************************************************************************
 * @file    hal_button.h
 * @brief   Push-button HAL driver (EXTI + debounce) for STM32F103C6
 * @author  Mohamed
 * @layer   HAL (uses MCAL: GPIO + EXTI)
 *
 * @note    Each button:
 *            - pin configured as input with internal PULL-UP
 *              (wire the button between the pin and GND)
 *            - EXTI on the FALLING edge (press pulls the pin low)
 *            - software DEBOUNCE: after the edge we wait ~2 ms and
 *              verify the pin is still low before accepting the press
 *
 *          The verify-delay runs inside the interrupt; it is short and
 *          presses are rare, so this is an acceptable simple design.
 *
 *          LIMITATION (hardware): each button must be on a DIFFERENT
 *          EXTI line number (the line = the pin number, whatever the
 *          port). E.g. PB3, PB4, PB5 -> lines 3, 4, 5: OK.
 *          PA3 and PB3 together -> both are line 3: NOT possible.
 *
 *          Example:
 *            void on_start(void) { ... }
 *            Button_Init(GPIOB, GPIO_PIN_3, on_start);
 ******************************************************************************
 */

#ifndef HAL_BUTTON_H_
#define HAL_BUTTON_H_

#include "stm32f103c6_gpio.h"
#include "stm32f103c6_EXTI_DRIVER.h"
#include <stdint.h>

/* called (from interrupt context) on a debounced press */
typedef void (*Button_Callback_t)(void);

/**
 * @brief  Initialize one button on port/pin with a press callback.
 *         Pin becomes input pull-up; EXTI falling edge; debounced.
 * @param  port     : GPIOA / GPIOB / GPIOC
 * @param  pin      : pin number (also the EXTI line, must be unique)
 * @param  callback : called once per clean press
 */
void Button_Init(GPIO_TypeDef *port, GPIO_Pin_t pin,
                 Button_Callback_t callback);

/**
 * @brief  Read the CURRENT level of the button.
 * @return 1 = pressed (pin low), 0 = released.
 */
uint8_t Button_IsPressed(GPIO_TypeDef *port, GPIO_Pin_t pin);

#endif /* HAL_BUTTON_H_ */
