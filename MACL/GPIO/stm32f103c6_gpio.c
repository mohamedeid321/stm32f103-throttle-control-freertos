/**
 ******************************************************************************
 * @file    stm32f103c6_gpio.c
 * @brief   GPIO Driver Implementation for STM32F103C6
 ******************************************************************************
 */

#include "stm32f103c6_gpio.h"

/* ============================================================
 *                    GPIO_EnableClock
 *   بيفعّل الـ clock للـ port المطلوب من ريجستر RCC->APB2ENR
 *   كل الـ GPIO ports في F1 موجودة على ناقل APB2
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
 *   بيضبط الـ 4 bits بتاعة الـ pin في CRL أو CRH
 *
 *   - لو الـ pin من 0–7  -> نستخدم CRL
 *   - لو الـ pin من 8–15 -> نستخدم CRH
 *   - موقع الـ bits = (pin % 8) * 4
 * ============================================================ */
void GPIO_InitPin(GPIO_TypeDef *GPIOx, GPIO_Pin_t pin, GPIO_Mode_t mode)
{
    /* تأكد إن الـ clock متفعّل (راحة بال للمستخدم) */
    GPIO_EnableClock(GPIOx);

    if (pin < 8)
    {
        uint32_t shift = (uint32_t)pin * 4U;
        GPIOx->CRL &= ~(0xFU << shift);          /* امسح الـ 4 bits  */
        GPIOx->CRL |=  ((uint32_t)mode << shift); /* اضبط الـ mode    */
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
 *   بتكتب HIGH أو LOW على الـ pin (بتستخدم BSRR الآمن داخليًا)
 * ============================================================ */
void GPIO_WritePin(GPIO_TypeDef *GPIOx, GPIO_Pin_t pin, GPIO_PinState_t state)
{
    if (state == GPIO_PIN_HIGH)
        GPIOx->BSRR = (1U << pin);          /* Set   */
    else
        GPIOx->BSRR = (1U << (pin + 16U));  /* Reset */
}

/* ============================================================
 *                    GPIO_ReadPin
 *   بترجّع قيمة الـ pin (0 أو 1) من ريجستر IDR
 * ============================================================ */
uint8_t GPIO_ReadPin(GPIO_TypeDef *GPIOx, GPIO_Pin_t pin)
{
    return (uint8_t)((GPIOx->IDR >> pin) & 1U);
}



void GPIO_SetPull(GPIO_TypeDef *GPIOx, GPIO_Pin_t pin, uint8_t pull_up)
{
    if (pull_up)
        GPIOx->ODR |=  (1U << pin);   // ODR=1 → Pull-UP
    else
        GPIOx->ODR &= ~(1U << pin);   // ODR=0 → Pull-DOWN
}
