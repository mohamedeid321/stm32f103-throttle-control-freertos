/**
 ******************************************************************************
 * @file    stm32f103c6_spi.c
 * @brief   SPI1 Master Driver Implementation for STM32F103C6
 ******************************************************************************
 */

#include "stm32f103c6_spi.h"

static SPI_RxCallback_t spi_rx_cb = 0;

/* ============================================================
 *                    SPI_GetDefaultConfig
 * ============================================================ */
SPI_Config_t SPI_GetDefaultConfig(void)
{
    SPI_Config_t cfg;
    cfg.mode        = SPI_MODE_0;
    cfg.prescaler   = SPI_PRESCALER_8;
    cfg.rx_callback = 0;            /* polling by default */
    return cfg;
}

/* ============================================================
 *                    SPI_Init
 *   SPI1 master: SCK=PA5, MISO=PA6, MOSI=PA7.
 *   NSS handled in software (SSM + SSI) so we don't need a pin.
 * ============================================================ */
void SPI_Init(const SPI_Config_t *cfg)
{
    /* ---- 1) clocks: GPIOA + SPI1 (both APB2) ---- */
    RCC->APB2ENR |= (1U << 2);     /* GPIOA */
    RCC->APB2ENR |= (1U << 12);    /* SPI1  */

    /* ---- 2) pins ----
     *   SCK (PA5) and MOSI (PA7) = alternate-function push-pull
     *   MISO (PA6) = input floating
     */
    GPIO_InitPin(GPIOA, GPIO_PIN_5, GPIO_MODE_OUTPUT_AF_PP);
    GPIO_InitPin(GPIOA, GPIO_PIN_7, GPIO_MODE_OUTPUT_AF_PP);
    GPIO_InitPin(GPIOA, GPIO_PIN_6, GPIO_MODE_INPUT_FLOATING);

    /* ---- 3) configure CR1 ---- */
    uint32_t cr1 = 0;

    /* mode (CPOL/CPHA): mode value bit0=CPHA, bit1=CPOL */
    if (cfg->mode & 0x1) cr1 |= SPI_CR1_CPHA;
    if (cfg->mode & 0x2) cr1 |= SPI_CR1_CPOL;

    cr1 |= SPI_CR1_MSTR;                       /* master */
    cr1 |= ((uint32_t)cfg->prescaler << 3);    /* BR bits 3:5 */
    cr1 |= SPI_CR1_SSM | SPI_CR1_SSI;          /* software NSS, stay master */

    SPI1->CR1 = cr1;

    /* ---- 4) interrupt mode? ---- */
    spi_rx_cb = cfg->rx_callback;
    if (cfg->rx_callback != 0) {
        SPI1->CR2 |= SPI_CR2_RXNEIE;           /* RX interrupt */
        NVIC_ENABLE_IRQ(SPI1_IRQn);
    }

    /* ---- 5) enable SPI ---- */
    SPI1->CR1 |= SPI_CR1_SPE;
}

/* ============================================================
 *                    SPI_Transfer (polling, full-duplex)
 *   Write a byte, wait for the exchange, read the byte that
 *   came back. SPI always sends and receives together.
 * ============================================================ */
uint8_t SPI_Transfer(uint8_t data)
{
    while (!(SPI1->SR & SPI_SR_TXE));    /* wait TX empty   */
    SPI1->DR = data;                      /* send            */

    while (!(SPI1->SR & SPI_SR_RXNE));   /* wait RX ready   */
    return (uint8_t)SPI1->DR;             /* read received   */
}

/* ============================================================
 *                    SPI_TransferIT (interrupt)
 *   Send a byte and return at once. The RX interrupt will
 *   deliver the received byte to the callback.
 * ============================================================ */
void SPI_TransferIT(uint8_t data)
{
    while (!(SPI1->SR & SPI_SR_TXE));    /* make sure we can write */
    SPI1->DR = data;                      /* send; RX IRQ handles result */
}

/* ============================================================
 *                    SPI1 Interrupt Handler
 *   Fires when a byte is received (RXNE). Reading DR clears it.
 * ============================================================ */
void SPI1_IRQHandler(void)
{
    if (SPI1->SR & SPI_SR_RXNE)
    {
        uint8_t rx = (uint8_t)SPI1->DR;   /* read clears RXNE */
        if (spi_rx_cb)
            spi_rx_cb(rx);
    }
}
