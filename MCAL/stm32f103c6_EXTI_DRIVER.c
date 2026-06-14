/**
 ******************************************************************************
 * @file    stm32f103c6_exti.c
 * @brief   EXTI Driver Implementation for STM32F103C6
 ******************************************************************************
 */

#include "stm32f103c6_EXTI_DRIVER.h"

/* ============================================================
 *              Callback storage (one slot per line)
 *
 *   We have 16 EXTI lines, so we keep an array of 16 function
 *   pointers. When a line fires, the ISR looks up its callback
 *   here and calls it. Initialized to NULL (no callback yet).
 * ============================================================ */
static EXTI_Callback_t exti_callbacks[16] = {0};

/* ============================================================
 *                    EXTI_Init
 *   Configures one EXTI line end-to-end (see header for details).
 * ============================================================ */
void EXTI_Init(EXTI_Port_t port, GPIO_Pin_t pin,
               EXTI_Trigger_t trigger, EXTI_Callback_t callback)
{
    /* ---- 1) Store the user callback for this line ---- */
    exti_callbacks[pin] = callback;

    /* ---- 2) Enable the AFIO clock (needed for EXTICR) ---- */
    RCC_AFIO_CLOCK_EN();

    /* ---- 3) Map the EXTI line to the chosen port ----
     *   Each EXTICR register holds 4 lines (4 bits each).
     *   - register index = pin / 4
     *   - bit offset     = (pin % 4) * 4
     */
    uint32_t reg_index = (uint32_t)pin / 4U;
    uint32_t shift     = ((uint32_t)pin % 4U) * 4U;

    AFIO->EXTICR[reg_index] &= ~(0xFU << shift);              /* clear nibble */
    AFIO->EXTICR[reg_index] |=  ((uint32_t)port << shift);    /* set port     */

    /* ---- 4) Select the trigger edge ----
     *   RTSR = rising, FTSR = falling. "Both" sets both bits.
     */
    if (trigger == EXTI_TRIGGER_RISING || trigger == EXTI_TRIGGER_BOTH)
        EXTI->RTSR |= (1U << pin);
    else
        EXTI->RTSR &= ~(1U << pin);

    if (trigger == EXTI_TRIGGER_FALLING || trigger == EXTI_TRIGGER_BOTH)
        EXTI->FTSR |= (1U << pin);
    else
        EXTI->FTSR &= ~(1U << pin);

    /* ---- 5) Unmask the interrupt on this line ---- */
    EXTI->IMR |= (1U << pin);

    /* ---- 6) Enable the matching IRQ in the NVIC ----
     *   Lines 0..4 each have their own IRQ.
     *   Lines 5..9 share one IRQ, lines 10..15 share another.
     */
    /* ---- 6) Set priority (FreeRTOS-safe) then enable in NVIC ----
     *   Priority must be NUMERICALLY >= configLIBRARY_MAX_SYSCALL_
     *   INTERRUPT_PRIORITY (5) so the handler may safely use
     *   FreeRTOS "FromISR" APIs. We use 6 (just below that floor,
     *   i.e. lower priority than the kernel) for all EXTI lines.
     *   Without this, the default priority is 0 (highest possible),
     *   which is UNSAFE with FreeRTOS and can cause missed/garbled
     *   interrupts.
     */
    if      (pin <= 4)  { NVIC_SetPriority(EXTI0_IRQn + pin, 6); NVIC_ENABLE_IRQ(EXTI0_IRQn + pin); }
    else if (pin <= 9)  { NVIC_SetPriority(EXTI9_5_IRQn, 6);     NVIC_ENABLE_IRQ(EXTI9_5_IRQn); }
    else                { NVIC_SetPriority(EXTI15_10_IRQn, 6);   NVIC_ENABLE_IRQ(EXTI15_10_IRQn); }
}

/* ============================================================
 *                    EXTI_Disable
 *   Masks the line so it stops generating interrupts.
 * ============================================================ */
void EXTI_Disable(GPIO_Pin_t pin)
{
    EXTI->IMR &= ~(1U << pin);     /* mask the interrupt   */
    exti_callbacks[pin] = 0;       /* drop the callback    */
}

/* ============================================================
 *              Internal helper: handle one line
 *
 *   Checks if the line is pending, calls its callback,
 *   then clears the pending flag (write 1 to EXTI->PR).
 *   Clearing the flag is MANDATORY, otherwise the ISR
 *   would fire again immediately.
 * ============================================================ */
static void EXTI_HandleLine(uint8_t line)
{
    if (EXTI->PR & (1U << line))            /* did this line fire? */
    {
        if (exti_callbacks[line] != 0)      /* is there a callback? */
            exti_callbacks[line]();         /* run it               */

        EXTI->PR = (1U << line);            /* clear pending (write 1) */
    }
}

/* ============================================================
 *                    Interrupt Handlers (ISRs)
 *
 *   These names MUST match the vector table in the startup file.
 *   - EXTI0..4   : one handler each
 *   - EXTI9_5    : shared by lines 5..9   -> check each one
 *   - EXTI15_10  : shared by lines 10..15 -> check each one
 * ============================================================ */

void EXTI0_IRQHandler(void) { EXTI_HandleLine(0); }
void EXTI1_IRQHandler(void) { EXTI_HandleLine(1); }
void EXTI2_IRQHandler(void) { EXTI_HandleLine(2); }
void EXTI3_IRQHandler(void) { EXTI_HandleLine(3); }
void EXTI4_IRQHandler(void) { EXTI_HandleLine(4); }

/* Shared handler for lines 5..9 : we don't know which line fired,
 * so we check all of them. */
void EXTI9_5_IRQHandler(void)
{
    for (uint8_t line = 5; line <= 9; line++)
        EXTI_HandleLine(line);
}

/* Shared handler for lines 10..15 */
void EXTI15_10_IRQHandler(void)
{
    for (uint8_t line = 10; line <= 15; line++)
        EXTI_HandleLine(line);
}
