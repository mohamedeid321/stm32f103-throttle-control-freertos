/**
 ******************************************************************************
 * @file    hal_hbridge.c
 * @brief   H-Bridge HAL driver implementation
 ******************************************************************************
 */

#include "hal_hbridge.h"

/* single-motor module: store the config internally */
static HBridge_Config_t hb;

HBridge_Config_t HBRIDGE_GetDefaultConfig(void)
{
    HBridge_Config_t cfg;
    cfg.in1_port    = GPIOB;
    cfg.in1_pin     = GPIO_PIN_10;
    cfg.in2_port    = GPIOB;
    cfg.in2_pin     = GPIO_PIN_11;
    cfg.pwm_timer   = TIM3;
    cfg.pwm_channel = TIMER_CHANNEL_3;   /* TIM3 CH3 = PB0 */
    cfg.pwm_freq_hz = 1000;
    return cfg;
}

void HBRIDGE_Init(const HBridge_Config_t *cfg)
{
    hb = *cfg;

    /* direction pins as outputs, both low (coast) */
    GPIO_InitPin(hb.in1_port, hb.in1_pin, GPIO_MODE_OUTPUT_PP);
    GPIO_InitPin(hb.in2_port, hb.in2_pin, GPIO_MODE_OUTPUT_PP);
    GPIO_CLEAR_PIN(hb.in1_port, hb.in1_pin);
    GPIO_CLEAR_PIN(hb.in2_port, hb.in2_pin);

    /* PWM time-base + channel, starting at duty 0 (stopped) */
    Timer_InitBaseFreq(hb.pwm_timer, hb.pwm_freq_hz);
    Timer_ConfigChannelPWM(hb.pwm_timer, hb.pwm_channel, 0);
    Timer_Start(hb.pwm_timer);

    /* default direction so SetSpeed alone can move the motor */
    HBRIDGE_SetDirection(HBRIDGE_FORWARD);
}

void HBRIDGE_SetDirection(HBridge_Direction_t dir)
{
    if (dir == HBRIDGE_FORWARD) {
        GPIO_SET_PIN  (hb.in1_port, hb.in1_pin);
        GPIO_CLEAR_PIN(hb.in2_port, hb.in2_pin);
    } else {
        GPIO_CLEAR_PIN(hb.in1_port, hb.in1_pin);
        GPIO_SET_PIN  (hb.in2_port, hb.in2_pin);
    }
}

void HBRIDGE_SetSpeedPercent(uint8_t percent)
{
    if (percent > 100) percent = 100;
    Timer_SetDuty(hb.pwm_timer, hb.pwm_channel, percent);
}

void HBRIDGE_SetSpeedRaw(uint16_t value)
{
    Timer_SetDutyRaw(hb.pwm_timer, hb.pwm_channel, value);
}

void HBRIDGE_Stop(void)
{
    Timer_SetDutyRaw(hb.pwm_timer, hb.pwm_channel, 0);
    GPIO_CLEAR_PIN(hb.in1_port, hb.in1_pin);
    GPIO_CLEAR_PIN(hb.in2_port, hb.in2_pin);
}

void HBRIDGE_Brake(void)
{
    Timer_SetDutyRaw(hb.pwm_timer, hb.pwm_channel, 0);
    GPIO_SET_PIN(hb.in1_port, hb.in1_pin);
    GPIO_SET_PIN(hb.in2_port, hb.in2_pin);
}
