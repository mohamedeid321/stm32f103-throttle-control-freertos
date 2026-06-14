/**
 ******************************************************************************
 * @file    stm32f103c6_dma.h
 * @brief   Generic DMA Driver for STM32F103C6 (DMA1, channels 1..7)
 * @author  Mohamed
 * @note    Register-level driver (no HAL). Works with any peripheral:
 *          you give it the transfer type, addresses, count, and sizes,
 *          and it configures the channel. Optional completion callback.
 *
 *          Typical use with the ADC:
 *            DMA_Config_t d = DMA_GetDefaultConfig();
 *            d.channel       = DMA_CHANNEL_1;          // ADC1 uses ch1
 *            d.direction     = DMA_PERIPH_TO_MEMORY;
 *            d.peripheral_addr = (uint32_t)&ADC1->DR;
 *            d.memory_addr     = (uint32_t)adc_buffer;
 *            d.count           = 3;
 *            d.periph_size   = DMA_SIZE_16BIT;
 *            d.memory_size   = DMA_SIZE_16BIT;
 *            d.mem_increment = 1;
 *            d.circular      = 1;
 *            DMA_Init(&d);
 *            DMA_Start(DMA_CHANNEL_1);
 ******************************************************************************
 */

#ifndef STM32F103C6_DMA_H_
#define STM32F103C6_DMA_H_

#include "stm32f103_regs.h"
#include <stdint.h>

/* ============================================================
 *                  Channel selection (DMA1: 1..7)
 *   Note: each peripheral is hard-wired to a specific channel.
 *   e.g. ADC1 -> ch1, USART1_TX -> ch4, USART1_RX -> ch5,
 *        SPI1_RX -> ch2, SPI1_TX -> ch3.
 * ============================================================ */
typedef enum {
    DMA_CHANNEL_1 = 1,
    DMA_CHANNEL_2 = 2,
    DMA_CHANNEL_3 = 3,
    DMA_CHANNEL_4 = 4,
    DMA_CHANNEL_5 = 5,
    DMA_CHANNEL_6 = 6,
    DMA_CHANNEL_7 = 7
} DMA_Channel_t;

/* ============================================================
 *                  Transfer direction
 * ============================================================ */
typedef enum {
    DMA_PERIPH_TO_MEMORY = 0,   /* read from peripheral (DIR=0)  */
    DMA_MEMORY_TO_PERIPH = 1,   /* write to peripheral (DIR=1)   */
    DMA_MEMORY_TO_MEMORY = 2    /* memory copy (MEM2MEM=1)       */
} DMA_Direction_t;

/* ============================================================
 *                  Data size (PSIZE / MSIZE)
 * ============================================================ */
typedef enum {
    DMA_SIZE_8BIT  = 0,   /* 00 */
    DMA_SIZE_16BIT = 1,   /* 01 */
    DMA_SIZE_32BIT = 2    /* 10 */
} DMA_Size_t;

/* ============================================================
 *                  Priority level (PL)
 * ============================================================ */
typedef enum {
    DMA_PRIORITY_LOW       = 0,
    DMA_PRIORITY_MEDIUM    = 1,
    DMA_PRIORITY_HIGH      = 2,
    DMA_PRIORITY_VERY_HIGH = 3
} DMA_Priority_t;

/* completion callback (called when transfer completes, if interrupt on) */
typedef void (*DMA_Callback_t)(void);

/* ============================================================
 *                  Configuration structure
 * ============================================================ */
typedef struct {
    DMA_Channel_t   channel;          /* which DMA channel (1..7)        */
    DMA_Direction_t direction;        /* transfer direction              */
    uint32_t        peripheral_addr;  /* peripheral register address     */
    uint32_t        memory_addr;      /* memory buffer address           */
    uint16_t        count;            /* number of items to transfer     */
    DMA_Size_t      periph_size;      /* peripheral data size            */
    DMA_Size_t      memory_size;      /* memory data size                */
    uint8_t         periph_increment; /* 1 = increment peripheral addr   */
    uint8_t         mem_increment;    /* 1 = increment memory addr       */
    uint8_t         circular;         /* 1 = circular (auto-reload)      */
    DMA_Priority_t  priority;         /* channel priority                */
    DMA_Callback_t  callback;         /* called on complete (NULL = none) */
} DMA_Config_t;

/* ============================================================
 *                  Function Prototypes
 * ============================================================ */

/**
 * @brief  Sensible defaults: channel 1, periph->memory, 8-bit,
 *         memory increment on, not circular, low priority, no callback.
 */
DMA_Config_t DMA_GetDefaultConfig(void);

/**
 * @brief  Configure a DMA channel from the config (does not start it
 *         unless count>0 and you call DMA_Start). Enables the clock,
 *         sets addresses, count, sizes, increments, circular, priority,
 *         and (if a callback is given) the transfer-complete interrupt.
 */
void DMA_Init(const DMA_Config_t *cfg);

/** @brief Enable (start) the channel. */
void DMA_Start(DMA_Channel_t channel);

/** @brief Disable (stop) the channel. */
void DMA_Stop(DMA_Channel_t channel);

/**
 * @brief  Reload the number of items and restart (useful for one-shot
 *         transfers you want to trigger again).
 */
void DMA_SetCount(DMA_Channel_t channel, uint16_t count);

/** @brief Returns 1 if the transfer-complete flag is set for the channel. */
uint8_t DMA_IsComplete(DMA_Channel_t channel);

/** @brief Clear the transfer-complete flag for the channel. */
void DMA_ClearComplete(DMA_Channel_t channel);

#endif /* STM32F103C6_DMA_H_ */
