/**
 ******************************************************************************
 * @file    stm32f103c6_iwdg.h
 * @brief   Independent Watchdog (IWDG) Driver for STM32F103C6
 * @author  Mohamed
 * @layer   MCAL
 *
 * @note    The IWDG is clocked by the LSI (~40 kHz internal RC), fully
 *          independent of the main system clock (HSE/HSI/PLL). It is
 *          designed as a last-resort safety net: once started it
 *          CANNOT be disabled — only a system reset clears it.
 *
 *          Timeout formula:
 *            timeout_s = (4 * 2^prescaler) * reload / 40000
 *
 *          If IWDG_Refresh() is not called before the timeout expires,
 *          the IWDG forces a full system reset (equivalent to pressing
 *          the reset button).
 *
 *          DEBUGGING NOTE: the IWDG keeps counting even while the CPU
 *          is halted by the debugger (unless DBGMCU_CR is configured
 *          to freeze it). If you are single-stepping through code and
 *          the board keeps resetting, that's the IWDG firing — this is
 *          expected and not a bug.
 ******************************************************************************
 */

#ifndef STM32F103C6_IWDG_H_
#define STM32F103C6_IWDG_H_

#include "stm32f103_regs.h"
#include <stdint.h>

/**
 * @brief  Start the IWDG with a given prescaler and reload value.
 *
 * @param  prescaler : 0..6, selects divider = 4 * 2^prescaler
 *                      (0->4, 1->8, 2->16, 3->32, 4->64, 5->128, 6->256)
 * @param  reload    : 0..0xFFF (12-bit), the down-counter start value
 *
 * @note   Example for a ~2 second timeout:
 *           prescaler = 4  (divider = 64)
 *           reload    = 1250
 *           timeout   = 64 * 1250 / 40000 = 2.0 s
 *
 * @warning Once called, the watchdog runs FOREVER until a reset.
 *          Call IWDG_Refresh() periodically (faster than the
 *          timeout) or the MCU will reset.
 */
void IWDG_Init(uint8_t prescaler, uint16_t reload);

/**
 * @brief  Refresh ("feed") the watchdog — reloads the down-counter
 *         to the value set in IWDG_Init(), preventing a reset.
 *         Call this faster than the configured timeout.
 */
void IWDG_Refresh(void);

#endif /* STM32F103C6_IWDG_H_ */
