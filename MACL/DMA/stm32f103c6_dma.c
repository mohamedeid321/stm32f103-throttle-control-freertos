/**
 ******************************************************************************
 * @file    stm32f103c6_dma.c
 * @brief   Generic DMA Driver Implementation for STM32F103C6
 ******************************************************************************
 */

#include "stm32f103c6_dma.h"

/* one callback slot per channel (index 0 unused, channels are 1..7) */
static DMA_Callback_t dma_cb[8] = {0};

/* ============================================================
 *          Map a channel number to its register block
 * ============================================================ */
static DMA_Channel_TypeDef* DMA_GetChannel(DMA_Channel_t ch)
{
    switch (ch) {
        case DMA_CHANNEL_1: return DMA1_Channel1;
        case DMA_CHANNEL_2: return DMA1_Channel2;
        case DMA_CHANNEL_3: return DMA1_Channel3;
        case DMA_CHANNEL_4: return DMA1_Channel4;
        case DMA_CHANNEL_5: return DMA1_Channel5;
        case DMA_CHANNEL_6: return DMA1_Channel6;
        case DMA_CHANNEL_7: return DMA1_Channel7;
    }
    return DMA1_Channel1;
}

/* map a channel to its NVIC IRQ number */
static uint32_t DMA_GetIRQn(DMA_Channel_t ch)
{
    return (uint32_t)(DMA1_Channel1_IRQn + (ch - 1));
}

/* ============================================================
 *                    DMA_GetDefaultConfig
 * ============================================================ */
DMA_Config_t DMA_GetDefaultConfig(void)
{
    DMA_Config_t cfg;
    cfg.channel          = DMA_CHANNEL_1;
    cfg.direction        = DMA_PERIPH_TO_MEMORY;
    cfg.peripheral_addr  = 0;
    cfg.memory_addr      = 0;
    cfg.count            = 0;
    cfg.periph_size      = DMA_SIZE_8BIT;
    cfg.memory_size      = DMA_SIZE_8BIT;
    cfg.periph_increment = 0;
    cfg.mem_increment    = 1;
    cfg.circular         = 0;
    cfg.priority         = DMA_PRIORITY_LOW;
    cfg.callback         = 0;
    return cfg;
}

/* ============================================================
 *                    DMA_Init
 * ============================================================ */
void DMA_Init(const DMA_Config_t *cfg)
{
    DMA_Channel_TypeDef *ch = DMA_GetChannel(cfg->channel);

    /* ---- 1) enable DMA1 clock (AHBENR bit 0) ---- */
    RCC->AHBENR |= (1U << 0);

    /* ---- 2) disable channel while configuring ---- */
    ch->CCR &= ~(1U << 0);          /* EN = 0 */

    /* ---- 3) addresses ----
     *   For MEM2MEM, the "peripheral" address acts as the source.
     */
    ch->CPAR = cfg->peripheral_addr;
    ch->CMAR = cfg->memory_addr;

    /* ---- 4) number of items ---- */
    ch->CNDTR = cfg->count;

    /* ---- 5) build CCR ---- */
    uint32_t ccr = 0;

    /* direction */
    if (cfg->direction == DMA_MEMORY_TO_PERIPH)
        ccr |= (1U << 4);                 /* DIR = 1 (read from memory) */
    else if (cfg->direction == DMA_MEMORY_TO_MEMORY)
        ccr |= (1U << 14);                /* MEM2MEM = 1 */

    /* increments */
    if (cfg->periph_increment) ccr |= (1U << 6);   /* PINC */
    if (cfg->mem_increment)    ccr |= (1U << 7);   /* MINC */

    /* sizes */
    ccr |= ((uint32_t)cfg->periph_size << 8);      /* PSIZE bits 8:9   */
    ccr |= ((uint32_t)cfg->memory_size << 10);     /* MSIZE bits 10:11 */

    /* priority */
    ccr |= ((uint32_t)cfg->priority << 12);        /* PL bits 12:13 */

    /* circular */
    if (cfg->circular) ccr |= (1U << 5);           /* CIRC */

    /* transfer-complete interrupt if a callback is given */
    dma_cb[cfg->channel] = cfg->callback;
    if (cfg->callback != 0) {
        ccr |= (1U << 1);                          /* TCIE */
        NVIC_ENABLE_IRQ(DMA_GetIRQn(cfg->channel));
    }

    ch->CCR = ccr;                                 /* write, EN still 0 */
}

/* ============================================================
 *                    Start / Stop
 * ============================================================ */
void DMA_Start(DMA_Channel_t channel)
{
    DMA_GetChannel(channel)->CCR |= (1U << 0);     /* EN = 1 */
}

void DMA_Stop(DMA_Channel_t channel)
{
    DMA_GetChannel(channel)->CCR &= ~(1U << 0);    /* EN = 0 */
}

/* ============================================================
 *                    DMA_SetCount (reload + restart)
 *   The channel must be disabled to change CNDTR.
 * ============================================================ */
void DMA_SetCount(DMA_Channel_t channel, uint16_t count)
{
    DMA_Channel_TypeDef *ch = DMA_GetChannel(channel);
    ch->CCR   &= ~(1U << 0);     /* disable */
    ch->CNDTR  = count;
    ch->CCR   |= (1U << 0);      /* re-enable */
}

/* ============================================================
 *          Status flags (in DMA->ISR; 4 bits per channel)
 *   For channel n: bits start at (n-1)*4.
 *   bit0=GIF, bit1=TCIF, bit2=HTIF, bit3=TEIF.
 * ============================================================ */
uint8_t DMA_IsComplete(DMA_Channel_t channel)
{
    uint32_t tcif = (1U << (((channel - 1) * 4) + 1));   /* TCIF */
    return (DMA1->ISR & tcif) ? 1U : 0U;
}

void DMA_ClearComplete(DMA_Channel_t channel)
{
    uint32_t ctcif = (1U << (((channel - 1) * 4) + 1));  /* CTCIF in IFCR */
    DMA1->IFCR = ctcif;
}

/* ============================================================
 *                    Interrupt Handlers
 *   Each channel has its own vector. On transfer complete,
 *   clear the flag and call the stored callback.
 * ============================================================ */
static void DMA_HandleIRQ(DMA_Channel_t channel)
{
    uint32_t tcif = (1U << (((channel - 1) * 4) + 1));
    if (DMA1->ISR & tcif) {
        DMA1->IFCR = tcif;                 /* clear the flag */
        if (dma_cb[channel])
            dma_cb[channel]();
    }
}

void DMA1_Channel1_IRQHandler(void) { DMA_HandleIRQ(DMA_CHANNEL_1); }
void DMA1_Channel2_IRQHandler(void) { DMA_HandleIRQ(DMA_CHANNEL_2); }
void DMA1_Channel3_IRQHandler(void) { DMA_HandleIRQ(DMA_CHANNEL_3); }
void DMA1_Channel4_IRQHandler(void) { DMA_HandleIRQ(DMA_CHANNEL_4); }
void DMA1_Channel5_IRQHandler(void) { DMA_HandleIRQ(DMA_CHANNEL_5); }
void DMA1_Channel6_IRQHandler(void) { DMA_HandleIRQ(DMA_CHANNEL_6); }
void DMA1_Channel7_IRQHandler(void) { DMA_HandleIRQ(DMA_CHANNEL_7); }
