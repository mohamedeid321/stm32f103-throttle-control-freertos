/**
 ******************************************************************************
 * @file    hal_button.c
 * @brief   Push-button HAL driver implementation
 *
 *  How the pieces fit:
 *    EXTI line fires -> our per-line trampoline -> Button_HandleLine()
 *    -> debounce check -> user callback.
 *
 *  The EXTI driver's callback has no argument, so we register a tiny
 *  "trampoline" function per line that calls the shared handler with
 *  the line number.
 ******************************************************************************
 */

#include "hal_button.h"

/* per-line registry */
static GPIO_TypeDef      *btn_port[16] = {0};
static Button_Callback_t  btn_cb[16]   = {0};

/* ~2 ms busy wait at 8 MHz (tune BUTTON_DEBOUNCE_LOOPS if needed) */
#ifndef BUTTON_DEBOUNCE_LOOPS
#define BUTTON_DEBOUNCE_LOOPS  3200U
#endif

/* ============================================================
 *          Shared handler: debounce + dispatch
 * ============================================================ */
static void Button_HandleLine(uint8_t line)
{
    if (btn_port[line] == 0)
        return;

    /* debounce: short wait, then the pin must STILL be low */
    for (volatile uint32_t i = 0; i < BUTTON_DEBOUNCE_LOOPS; i++);

    if (GPIO_ReadPin(btn_port[line], (GPIO_Pin_t)line) == 0)   /* still pressed */
    {
        if (btn_cb[line])
            btn_cb[line]();
    }
}

/* ============================================================
 *          Trampolines: one tiny function per EXTI line
 * ============================================================ */
#define BTN_TRAMP(n) static void Button_Tramp##n(void) { Button_HandleLine(n); }
BTN_TRAMP(0)  BTN_TRAMP(1)  BTN_TRAMP(2)  BTN_TRAMP(3)
BTN_TRAMP(4)  BTN_TRAMP(5)  BTN_TRAMP(6)  BTN_TRAMP(7)
BTN_TRAMP(8)  BTN_TRAMP(9)  BTN_TRAMP(10) BTN_TRAMP(11)
BTN_TRAMP(12) BTN_TRAMP(13) BTN_TRAMP(14) BTN_TRAMP(15)

static const EXTI_Callback_t btn_tramp[16] = {
    Button_Tramp0,  Button_Tramp1,  Button_Tramp2,  Button_Tramp3,
    Button_Tramp4,  Button_Tramp5,  Button_Tramp6,  Button_Tramp7,
    Button_Tramp8,  Button_Tramp9,  Button_Tramp10, Button_Tramp11,
    Button_Tramp12, Button_Tramp13, Button_Tramp14, Button_Tramp15
};

/* map a GPIO_TypeDef* to the EXTI_Port_t value AFIO expects */
static EXTI_Port_t Button_PortToExti(GPIO_TypeDef *port)
{
    if (port == GPIOA) return EXTI_PORT_A;
    if (port == GPIOB) return EXTI_PORT_B;
    if (port == GPIOC) return EXTI_PORT_C;
    return EXTI_PORT_D;
}

/* ============================================================
 *                    Button_Init
 * ============================================================ */
void Button_Init(GPIO_TypeDef *port, GPIO_Pin_t pin,
                 Button_Callback_t callback)
{
    /* 1) input with internal pull-up (button to GND) */
    GPIO_InitPin(port, pin, GPIO_MODE_INPUT_PULLUPDOWN);
    GPIO_SetPull(port, pin, 1);              /* 1 = pull-up */

    /* 2) registry for the shared handler */
    btn_port[pin] = port;
    btn_cb[pin]   = callback;

    /* 3) EXTI on falling edge -> our trampoline for this line */
    EXTI_Init(Button_PortToExti(port), pin,
              EXTI_TRIGGER_FALLING, btn_tramp[pin]);
}

uint8_t Button_IsPressed(GPIO_TypeDef *port, GPIO_Pin_t pin)
{
    return (GPIO_ReadPin(port, pin) == 0) ? 1U : 0U;
}
