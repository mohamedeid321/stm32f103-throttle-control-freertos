/**
 ******************************************************************************
 * @file    stm32f103c6_uart.c
 * @brief   UART (USART1) Driver Implementation for STM32F103C6
 *          TX = PA9 (polling), RX = PA10 (interrupt-driven).
 ******************************************************************************
 */

#include "stm32f103c6_uart.h"

/* stored user callback for received bytes */
static UART_RxCallback_t uart_rx_cb = 0;

/* ============================================================
 *                    UART_Init
 * ============================================================ */
void UART_Init(uint32_t baudrate, UART_RxCallback_t rx_cb)
{
    /* ---- 1) clocks: GPIOA + USART1 (both APB2) ---- */
    RCC->APB2ENR |= (1U << 2);    /* GPIOA  */
    RCC->APB2ENR |= (1U << 14);   /* USART1 */

    /* ---- 2) pins ----
     *   PA9  (TX) = alternate-function push-pull
     *   PA10 (RX) = input floating
     */
    GPIO_InitPin(GPIOA, GPIO_PIN_9,  GPIO_MODE_OUTPUT_AF_PP);
    GPIO_InitPin(GPIOA, GPIO_PIN_10, GPIO_MODE_INPUT_FLOATING);

    /* ---- 3) baud rate ---- */
    USART1->BRR = UART_F_CPU / baudrate;

    /* ---- 4) control register ---- */
    USART1->CR1 = 0;
    USART1->CR1 |= (1U << 3);      /* TE: transmitter enable */
    USART1->CR1 |= (1U << 2);      /* RE: receiver enable    */

    /* ---- 5) RX interrupt (only if a callback is given) ---- */
    uart_rx_cb = rx_cb;
    if (rx_cb != 0) {
        USART1->CR1 |= (1U << 5);          /* RXNEIE: RX interrupt enable */
        NVIC_ENABLE_IRQ(USART1_IRQn);
    }

    /* ---- 6) enable the USART ---- */
    USART1->CR1 |= (1U << 13);     /* UE */
}

/* ============================================================
 *                    Transmit (polling)
 * ============================================================ */
void UART_SendChar(char c)
{
    while (!(USART1->SR & (1U << 7)));   /* wait TXE */
    USART1->DR = (uint32_t)(c & 0xFF);
}

void UART_SendString(const char *str)
{
    while (*str)
        UART_SendChar(*str++);
}

void UART_SendNumber(uint32_t num)
{
    char buf[11];
    int  i = 0;

    if (num == 0) { UART_SendChar('0'); return; }

    while (num > 0) {
        buf[i++] = (char)('0' + (num % 10));
        num /= 10;
    }
    while (i > 0)
        UART_SendChar(buf[--i]);
}

/* ============================================================
 *                    Receive (polling alternative)
 * ============================================================ */
uint8_t UART_ReceiveChar(void)
{
    while (!(USART1->SR & (1U << 5)));   /* wait RXNE */
    return (uint8_t)USART1->DR;
}

/* ============================================================
 *                    USART1 Interrupt Handler
 *   Fires when a byte arrives (RXNE). Reading DR clears the flag.
 *   The byte is passed to the user callback.
 * ============================================================ */
void USART1_IRQHandler(void)
{
    if (USART1->SR & (1U << 5))          /* RXNE: byte received? */
    {
        uint8_t byte = (uint8_t)USART1->DR;   /* read clears RXNE */
        if (uart_rx_cb)
            uart_rx_cb(byte);
    }
}
