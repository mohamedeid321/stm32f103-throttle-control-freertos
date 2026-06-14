/**
 ******************************************************************************
 * @file    stm32f103c6_gpio.h
 * @brief   GPIO Driver for STM32F103C6  (Cortex-M3)
 * @author  Mohamed
 * @note    Register-level driver (no HAL) - Proteus compatible
 ******************************************************************************
 */

#ifndef STM32F103C6_GPIO_H_
#define STM32F103C6_GPIO_H_

#include "stm32f103_regs.h"   /* تعريفات الريجسترات اليدوية (بدون CMSIS) */
#include <stdint.h>

/* ============================================================
 *                       1) الـ Pin Numbers
 * ============================================================ */
typedef enum {
    GPIO_PIN_0 = 0, GPIO_PIN_1,  GPIO_PIN_2,  GPIO_PIN_3,
    GPIO_PIN_4,     GPIO_PIN_5,  GPIO_PIN_6,  GPIO_PIN_7,
    GPIO_PIN_8,     GPIO_PIN_9,  GPIO_PIN_10, GPIO_PIN_11,
    GPIO_PIN_12,    GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15
} GPIO_Pin_t;

/* ============================================================
 *                       2) الـ Pin Modes
 *   القيم دي هي الـ 4 bits (CNF[1:0] | MODE[1:0]) جاهزة
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
 *                  3) حالة الـ Pin (للكتابة)
 * ============================================================ */
typedef enum {
    GPIO_PIN_LOW  = 0,
    GPIO_PIN_HIGH = 1
} GPIO_PinState_t;

/* ============================================================
 *        4) أهم جزء: الـ Macros (Set / Clear / Toggle / Check)
 *   مكتوبة عشان الكود يبقى أنضف و أسرع (inline)
 * ============================================================ */

/* تحكم سريع و آمن (atomic) عبر BSRR */
#define GPIO_SET_PIN(PORT, PIN)      ((PORT)->BSRR = (1U << (PIN)))
#define GPIO_CLEAR_PIN(PORT, PIN)    ((PORT)->BSRR = (1U << ((PIN) + 16U)))

/* التقليب عبر XOR على ODR */
#define GPIO_TOGGLE_PIN(PORT, PIN)   ((PORT)->ODR ^= (1U << (PIN)))

/* القراءة و الفحص */
#define GPIO_READ_PIN(PORT, PIN)     (((PORT)->IDR >> (PIN)) & 1U)
#define GPIO_CHECK_PIN(PORT, PIN)    (((PORT)->IDR & (1U << (PIN))) != 0U)

/* ============================================================
 *                    5) الـ Function Prototypes
 * ============================================================ */

/**
 * @brief  تفعيل الـ clock لـ port معيّن
 * @param  GPIOx : GPIOA, GPIOB, GPIOC
 */
void GPIO_EnableClock(GPIO_TypeDef *GPIOx);

/**
 * @brief  تهيئة pin (الاتجاه و النوع)
 * @param  GPIOx : الـ port
 * @param  pin   : رقم الـ pin (GPIO_PIN_0 .. 15)
 * @param  mode  : الـ mode من GPIO_Mode_t
 */
void GPIO_InitPin(GPIO_TypeDef *GPIOx, GPIO_Pin_t pin, GPIO_Mode_t mode);

/**
 * @brief  كتابة قيمة على pin (دالة - للاستخدام العام)
 */
void GPIO_WritePin(GPIO_TypeDef *GPIOx, GPIO_Pin_t pin, GPIO_PinState_t state);

/**
 * @brief  قراءة قيمة pin (دالة)
 * @retval 0 أو 1
 */
uint8_t GPIO_ReadPin(GPIO_TypeDef *GPIOx, GPIO_Pin_t pin);

void GPIO_SetPull(GPIO_TypeDef *GPIOx, GPIO_Pin_t pin, uint8_t pull_up);

#endif /* STM32F103C6_GPIO_H_ */
