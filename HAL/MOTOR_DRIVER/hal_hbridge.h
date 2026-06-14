/**
 ******************************************************************************
 * @file    hal_hbridge.h
 * @brief   H-Bridge DC-motor HAL driver (e.g. L298N) for STM32F103C6
 * @author  Mohamed
 * @layer   HAL (uses MCAL: GPIO + Timer PWM)
 *
 * @note    Wiring (one motor channel):
 *            IN1, IN2 -> two GPIO pins      (direction)
 *            EN       -> a timer PWM pin    (speed)
 *
 *          Direction truth table:
 *            IN1=1, IN2=0  -> forward
 *            IN1=0, IN2=1  -> reverse
 *            IN1=0, IN2=0  -> coast (free)
 *            IN1=1, IN2=1  -> brake (short)
 *
 *          Speed: PWM duty on EN. Use HBRIDGE_SetSpeedRaw() with the
 *          pot (0..1000) for full resolution.
 *
 *          Project default pins (from the pin map):
 *            IN1=PB10, IN2=PB11, EN=PB0 (TIM3 CH3).
 ******************************************************************************
 */

#ifndef HAL_HBRIDGE_H_
#define HAL_HBRIDGE_H_

#include "stm32f103c6_gpio.h"
#include "stm32f103c6_timer.h"
#include <stdint.h>

typedef enum {
    HBRIDGE_FORWARD = 0,
    HBRIDGE_REVERSE = 1
} HBridge_Direction_t;

/* configuration: direction pins + PWM timer/channel */
typedef struct {
    GPIO_TypeDef    *in1_port;     /* direction pin 1            */
    GPIO_Pin_t       in1_pin;
    GPIO_TypeDef    *in2_port;     /* direction pin 2            */
    GPIO_Pin_t       in2_pin;
    TIM_TypeDef     *pwm_timer;    /* timer driving the EN pin   */
    Timer_Channel_t  pwm_channel;  /* which channel (sets the pin)*/
    uint32_t         pwm_freq_hz;  /* e.g. 1000 (1 kHz)          */
} HBridge_Config_t;

/**
 * @brief  Sensible project defaults: IN1=PB10, IN2=PB11,
 *         TIM3 CH3 (PB0), 1 kHz PWM.
 */
HBridge_Config_t HBRIDGE_GetDefaultConfig(void);

/**
 * @brief  Initialize pins + PWM. Motor starts STOPPED (duty 0),
 *         direction = forward.
 */
void HBRIDGE_Init(const HBridge_Config_t *cfg);

/** @brief Set rotation direction (safe to call while running). */
void HBRIDGE_SetDirection(HBridge_Direction_t dir);

/** @brief Speed as percent 0..100. */
void HBRIDGE_SetSpeedPercent(uint8_t percent);

/**
 * @brief  Speed raw 0..1000 (full PWM resolution).
 *         Pairs directly with POT_ReadDuty1000().
 */
void HBRIDGE_SetSpeedRaw(uint16_t value);

/** @brief Coast stop: duty 0 + both IN pins low (free spin). */
void HBRIDGE_Stop(void);

/** @brief Active brake: both IN pins high (motor shorted). */
void HBRIDGE_Brake(void);

#endif /* HAL_HBRIDGE_H_ */
