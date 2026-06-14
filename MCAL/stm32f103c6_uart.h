/**
 ******************************************************************************
 * @file    stm32f103c6_uart.h
 * @brief   UART (USART1) Driver for STM32F103C6
 * @author  Mohamed
 * @note    Register-level driver (no HAL).
 *          TX = PA9 (polling).  RX = PA10 (interrupt-driven).
 *          On each received byte, a user callback is called with it.
 ******************************************************************************
 */

#ifndef STM32F103C6_UART_H_
#define STM32F103C6_UART_H_

#include "stm32f103_regs.h"
#include "stm32f103c6_gpio.h"
#include <stdint.h>

/* ============================================================
 *   CONFIG: clock feeding USART1 (APB2). 8 MHz Proteus / 72 real.
 * ============================================================ */
#ifndef UART_F_CPU
#define UART_F_CPU   8000000UL
#endif

/* ============================================================
 *   Receive callback type: called from the RX interrupt with
 *   the byte that just arrived.
 * ============================================================ */
typedef void (*UART_RxCallback_t)(uint8_t byte);

/* ============================================================
 *                  Function Prototypes
 * ============================================================ */

/**
 * @brief  Initialize USART1: TX on PA9, RX on PA10.
 *         Enables the RX interrupt; pass a callback to handle
 *         received bytes (or NULL to disable RX handling).
 * @param  baudrate  : e.g. 9600 or 115200
 * @param  rx_cb     : called on each received byte (or NULL)
 */
void UART_Init(uint32_t baudrate, UART_RxCallback_t rx_cb);

/* ---- Transmit (polling) ---- */
void UART_SendChar(char c);
void UART_SendString(const char *str);
void UART_SendNumber(uint32_t num);

/* ---- Receive (polling alternative, if you prefer it) ---- */
/**
 * @brief  Blocking receive of one byte (busy-waits for RXNE).
 *         Use this only if you did NOT set an RX callback.
 */
uint8_t UART_ReceiveChar(void);

#endif /* STM32F103C6_UART_H_ */
