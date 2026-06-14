/**
 ******************************************************************************
 * @file    stm32f103c6_spi.h
 * @brief   SPI (SPI1) Master Driver for STM32F103C6
 * @author  Mohamed
 * @note    Register-level driver (no HAL). Master mode.
 *          SPI1 pins: SCK=PA5, MISO=PA6, MOSI=PA7, NSS(SW)=any GPIO.
 *
 *          Two ways to transfer:
 *            - POLLING    : SPI_Transfer() blocks until done (simple).
 *            - INTERRUPT  : SPI_TransferIT() returns immediately and a
 *                           callback fires when a byte is received.
 ******************************************************************************
 */

#ifndef STM32F103C6_SPI_H_
#define STM32F103C6_SPI_H_

#include "stm32f103_regs.h"
#include "stm32f103c6_gpio.h"
#include <stdint.h>

/* ============================================================
 *                  SPI Mode (CPOL/CPHA)
 * ============================================================ */
typedef enum {
    SPI_MODE_0 = 0,   /* CPOL=0, CPHA=0 */
    SPI_MODE_1 = 1,   /* CPOL=0, CPHA=1 */
    SPI_MODE_2 = 2,   /* CPOL=1, CPHA=0 */
    SPI_MODE_3 = 3    /* CPOL=1, CPHA=1 */
} SPI_Mode_t;

/* ============================================================
 *                  Baud rate prescaler (BR bits)
 *   SPI clock = fPCLK / prescaler.
 * ============================================================ */
typedef enum {
    SPI_PRESCALER_2   = 0,
    SPI_PRESCALER_4   = 1,
    SPI_PRESCALER_8   = 2,
    SPI_PRESCALER_16  = 3,
    SPI_PRESCALER_32  = 4,
    SPI_PRESCALER_64  = 5,
    SPI_PRESCALER_128 = 6,
    SPI_PRESCALER_256 = 7
} SPI_Prescaler_t;

/* RX callback for interrupt mode */
typedef void (*SPI_RxCallback_t)(uint8_t received);

/* ============================================================
 *                  Configuration structure
 * ============================================================ */
typedef struct {
    SPI_Mode_t       mode;        /* SPI mode 0..3            */
    SPI_Prescaler_t  prescaler;   /* clock speed              */
    SPI_RxCallback_t rx_callback; /* for interrupt mode (NULL = polling) */
} SPI_Config_t;

/* ============================================================
 *                  Function Prototypes
 * ============================================================ */

/**
 * @brief  Default config: mode 0, prescaler 8, polling (no callback).
 */
SPI_Config_t SPI_GetDefaultConfig(void);

/**
 * @brief  Initialize SPI1 as master (SCK=PA5, MISO=PA6, MOSI=PA7).
 *         If cfg->rx_callback is set, interrupt mode is enabled.
 */
void SPI_Init(const SPI_Config_t *cfg);

/**
 * @brief  POLLING transfer: send one byte and return the byte
 *         received at the same time (SPI is full-duplex).
 */
uint8_t SPI_Transfer(uint8_t data);

/**
 * @brief  INTERRUPT transfer: send one byte, return immediately.
 *         The configured callback fires when the byte is received.
 */
void SPI_TransferIT(uint8_t data);

#endif /* STM32F103C6_SPI_H_ */
