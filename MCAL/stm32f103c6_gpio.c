/**
 ******************************************************************************
 * @file    stm32f103c6_gpio.c
 * @brief   GPIO Driver Implementation for STM32F103C6
 ******************************************************************************
 */

#include "stm32f103c6_gpio.h"

/* ============================================================
 *                    GPIO_EnableClock
 *   Enables the clock of the requested port in RCC->APB2ENR.
 *   All GPIO ports on the F1 family live on the APB2 bus.
 * ============================================================ */
void GPIO_EnableClock(GPIO_TypeDef *GPIOx)
{
    if (GPIOx == GPIOA)      RCC->APB2ENR |= (1U << 2);
    else if (GPIOx == GPIOB) RCC->APB2ENR |= (1U << 3);
    else if (GPIOx == GPIOC) RCC->APB2ENR |= (1U << 4);
    else if (GPIOx == GPIOD) RCC->APB2ENR |= (1U << 5);
}

/* ============================================================
 *                    GPIO_InitPin
 *   Sets the 4 configuration bits of the pin in CRL or CRH:
 *   - pins 0..7  -> CRL
 *   - pins 8..15 -> CRH
 *   - bit position = (pin % 8) * 4
 * ============================================================ */
void GPIO_InitPin(GPIO_TypeDef *GPIOx, GPIO_Pin_t pin, GPIO_Mode_t mode)
{
    /* make sure the clock is on (convenience for the user) */
    GPIO_EnableClock(GPIOx);

    if (pin < 8)
    {
        uint32_t shift = (uint32_t)pin * 4U;
        GPIOx->CRL &= ~(0xFU << shift);           /* clear the 4 bits */
        GPIOx->CRL |=  ((uint32_t)mode << shift); /* set the mode     */
    }
    else
    {
        uint32_t shift = ((uint32_t)pin - 8U) * 4U;
        GPIOx->CRH &= ~(0xFU << shift);
        GPIOx->CRH |=  ((uint32_t)mode << shift);
    }
}

/* ============================================================
 *                    GPIO_WritePin
 *   Writes HIGH or LOW using the atomic BSRR register.
 * ============================================================ */
void GPIO_WritePin(GPIO_TypeDef *GPIOx, GPIO_Pin_t pin, GPIO_PinState_t state)
{
    if (state == GPIO_PIN_HIGH)
        GPIOx->BSRR = (1U << pin);          /* set   */
    else
        GPIOx->BSRR = (1U << (pin + 16U));  /* reset */
}

/* ============================================================
 *                    GPIO_ReadPin
 *   Returns the pin level (0 or 1) from the IDR register.
 * ============================================================ */
uint8_t GPIO_ReadPin(GPIO_TypeDef *GPIOx, GPIO_Pin_t pin)
{
    return (uint8_t)((GPIOx->IDR >> pin) & 1U);
}

/* ============================================================
 *                    GPIO_SetPull
 *   In input pull-up/down mode the ODR bit selects the pull:
 *   ODR=1 -> pull-up, ODR=0 -> pull-down.
 * ============================================================ */
void GPIO_SetPull(GPIO_TypeDef *GPIOx, GPIO_Pin_t pin, uint8_t pull_up)
{
    if (pull_up)
        GPIOx->ODR |=  (1U << pin);   /* pull-up   */
    else
        GPIOx->ODR &= ~(1U << pin);   /* pull-down */
}
