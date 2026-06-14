/**
 ******************************************************************************
 * @file    stm32f103_regs.h
 * @brief   Manual register definitions for STM32F103C6
 *          (alternative to CMSIS / stm32f1xx.h)
 *
 *          Includes: GPIO, RCC, AFIO, EXTI and NVIC
 *
 * @note    Use this when CMSIS is not available in the project.
 *          Addresses are taken from the Reference Manual (RM0008)
 *          and the device datasheet.
 ******************************************************************************
 */

#ifndef STM32F103_REGS_H_
#define STM32F103_REGS_H_

#include <stdint.h>

/* ============================================================
 *  Shortcut: __IO means volatile
 *  (the register can change outside of the program flow)
 * ============================================================ */
#define __IO   volatile

/* ============================================================
 *                  1) Peripheral Base Addresses
 *  (from the STM32F103 memory map)
 * ============================================================ */
#define PERIPH_BASE        (0x40000000UL)

#define APB2PERIPH_BASE    (PERIPH_BASE + 0x10000UL)   /* 0x40010000 */
#define AHBPERIPH_BASE     (PERIPH_BASE + 0x20000UL)   /* 0x40020000 */

#define RCC_BASE           (AHBPERIPH_BASE + 0x1000UL) /* 0x40021000 */

#define AFIO_BASE          (APB2PERIPH_BASE + 0x0000UL)/* 0x40010000 */
#define EXTI_BASE          (APB2PERIPH_BASE + 0x0400UL)/* 0x40010400 */

#define GPIOA_BASE         (APB2PERIPH_BASE + 0x0800UL)/* 0x40010800 */
#define GPIOB_BASE         (APB2PERIPH_BASE + 0x0C00UL)/* 0x40010C00 */
#define GPIOC_BASE         (APB2PERIPH_BASE + 0x1000UL)/* 0x40011000 */
#define GPIOD_BASE         (APB2PERIPH_BASE + 0x1400UL)/* 0x40011400 */

#define APB1PERIPH_BASE    (PERIPH_BASE + 0x0000UL)   /* 0x40000000 */

/* General Purpose Timers (on APB1) */
#define TIM2_BASE          (APB1PERIPH_BASE + 0x0000UL)/* 0x40000000 */
#define TIM3_BASE          (APB1PERIPH_BASE + 0x0400UL)/* 0x40000400 */
#define TIM4_BASE          (APB1PERIPH_BASE + 0x0800UL)/* 0x40000800 */

/* ADC1 (on APB2) */
#define ADC1_BASE          (APB2PERIPH_BASE + 0x2400UL)/* 0x40012400 */

/* USART1 (on APB2) */
#define USART1_BASE        (APB2PERIPH_BASE + 0x3800UL)/* 0x40013800 */

/* DMA1 (on AHB) */
#define DMA1_BASE          (AHBPERIPH_BASE + 0x0000UL) /* 0x40020000 */

/* SPI1 (on APB2) */
#define SPI1_BASE          (APB2PERIPH_BASE + 0x3000UL)/* 0x40013000 */

/* I2C1 (on APB1) */
#define I2C1_BASE          (APB1PERIPH_BASE + 0x5400UL)/* 0x40005400 */

/* RTC, PWR, BKP (on APB1) */
#define RTC_BASE           (APB1PERIPH_BASE + 0x2800UL)/* 0x40002800 */
#define PWR_BASE           (APB1PERIPH_BASE + 0x7000UL)/* 0x40007000 */
#define BKP_BASE           (APB1PERIPH_BASE + 0x6C00UL)/* 0x40006C00 */

/* NVIC registers (inside the Cortex-M3 System Control Space) */
#define NVIC_ISER_BASE     (0xE000E100UL)  /* Interrupt Set-Enable Registers */

/* ============================================================
 *                  2) GPIO Register Structure
 *  This layout matches the Reference Manual exactly (important!)
 * ============================================================ */
typedef struct {
    __IO uint32_t CRL;    /* 0x00: Config Register Low   (pins 0-7)  */
    __IO uint32_t CRH;    /* 0x04: Config Register High  (pins 8-15) */
    __IO uint32_t IDR;    /* 0x08: Input Data Register               */
    __IO uint32_t ODR;    /* 0x0C: Output Data Register              */
    __IO uint32_t BSRR;   /* 0x10: Bit Set/Reset Register            */
    __IO uint32_t BRR;    /* 0x14: Bit Reset Register                */
    __IO uint32_t LCKR;   /* 0x18: Lock Register                     */
} GPIO_TypeDef;

/* ============================================================
 *                  3) RCC Register Structure
 *  (we only need APB2ENR, but we define up to it)
 * ============================================================ */
typedef struct {
    __IO uint32_t CR;        /* 0x00 */
    __IO uint32_t CFGR;      /* 0x04 */
    __IO uint32_t CIR;       /* 0x08 */
    __IO uint32_t APB2RSTR;  /* 0x0C */
    __IO uint32_t APB1RSTR;  /* 0x10 */
    __IO uint32_t AHBENR;    /* 0x14 */
    __IO uint32_t APB2ENR;   /* 0x18: used to enable GPIO/AFIO clocks */
    __IO uint32_t APB1ENR;   /* 0x1C */
    __IO uint32_t BDCR;      /* 0x20 */
    __IO uint32_t CSR;       /* 0x24 */
} RCC_TypeDef;

/* ============================================================
 *                  4) AFIO Register Structure
 *   EXTICR[0..3] choose which port maps to each EXTI line.
 * ============================================================ */
typedef struct {
    __IO uint32_t EVCR;
    __IO uint32_t MAPR;
    __IO uint32_t EXTICR[4];  /* EXTICR1..4 : port selection per line */
    __IO uint32_t MAPR2;
} AFIO_TypeDef;

/* ============================================================
 *                  5) EXTI Register Structure
 * ============================================================ */
typedef struct {
    __IO uint32_t IMR;    /* 0x00: Interrupt Mask Register     */
    __IO uint32_t EMR;    /* 0x04: Event Mask Register         */
    __IO uint32_t RTSR;   /* 0x08: Rising Trigger Selection    */
    __IO uint32_t FTSR;   /* 0x0C: Falling Trigger Selection   */
    __IO uint32_t SWIER;  /* 0x10: Software Interrupt Event    */
    __IO uint32_t PR;     /* 0x14: Pending Register (write 1 to clear) */
} EXTI_TypeDef;

/* ============================================================
 *                  5b) TIM Register Structure (General Purpose)
 *   Layout for TIM2/3/4. Order matches the Reference Manual.
 * ============================================================ */
typedef struct {
    __IO uint32_t CR1;    /* 0x00: Control Register 1            */
    __IO uint32_t CR2;    /* 0x04: Control Register 2            */
    __IO uint32_t SMCR;   /* 0x08: Slave Mode Control           */
    __IO uint32_t DIER;   /* 0x0C: DMA/Interrupt Enable         */
    __IO uint32_t SR;     /* 0x10: Status Register              */
    __IO uint32_t EGR;    /* 0x14: Event Generation             */
    __IO uint32_t CCMR1;  /* 0x18: Capture/Compare Mode 1       */
    __IO uint32_t CCMR2;  /* 0x1C: Capture/Compare Mode 2       */
    __IO uint32_t CCER;   /* 0x20: Capture/Compare Enable       */
    __IO uint32_t CNT;    /* 0x24: Counter                      */
    __IO uint32_t PSC;    /* 0x28: Prescaler                    */
    __IO uint32_t ARR;    /* 0x2C: Auto-Reload Register         */
    __IO uint32_t RCR;    /* 0x30: Repetition Counter           */
    __IO uint32_t CCR1;   /* 0x34: Capture/Compare 1            */
    __IO uint32_t CCR2;   /* 0x38: Capture/Compare 2            */
    __IO uint32_t CCR3;   /* 0x3C: Capture/Compare 3            */
    __IO uint32_t CCR4;   /* 0x40: Capture/Compare 4            */
} TIM_TypeDef;

/* ============================================================
 *                  5c) ADC Register Structure
 *   Layout for ADC1. Order matches the Reference Manual.
 * ============================================================ */
typedef struct {
    __IO uint32_t SR;      /* 0x00: Status Register            */
    __IO uint32_t CR1;     /* 0x04: Control Register 1         */
    __IO uint32_t CR2;     /* 0x08: Control Register 2         */
    __IO uint32_t SMPR1;   /* 0x0C: Sample Time 1 (ch 10-17)   */
    __IO uint32_t SMPR2;   /* 0x10: Sample Time 2 (ch 0-9)     */
    __IO uint32_t JOFR1;   /* 0x14 */
    __IO uint32_t JOFR2;   /* 0x18 */
    __IO uint32_t JOFR3;   /* 0x1C */
    __IO uint32_t JOFR4;   /* 0x20 */
    __IO uint32_t HTR;     /* 0x24 */
    __IO uint32_t LTR;     /* 0x28 */
    __IO uint32_t SQR1;    /* 0x2C: Sequence 1 (+ length L)    */
    __IO uint32_t SQR2;    /* 0x30: Sequence 2                 */
    __IO uint32_t SQR3;    /* 0x34: Sequence 3 (first channels)*/
    __IO uint32_t JSQR;    /* 0x38 */
    __IO uint32_t JDR1;    /* 0x3C */
    __IO uint32_t JDR2;    /* 0x40 */
    __IO uint32_t JDR3;    /* 0x44 */
    __IO uint32_t JDR4;    /* 0x48 */
    __IO uint32_t DR;      /* 0x4C: Data Register (result)     */
} ADC_TypeDef;

/* ============================================================
 *                  5d) USART Register Structure
 * ============================================================ */
typedef struct {
    __IO uint32_t SR;     /* 0x00: Status Register            */
    __IO uint32_t DR;     /* 0x04: Data Register              */
    __IO uint32_t BRR;    /* 0x08: Baud Rate Register         */
    __IO uint32_t CR1;    /* 0x0C: Control Register 1         */
    __IO uint32_t CR2;    /* 0x10: Control Register 2         */
    __IO uint32_t CR3;    /* 0x14: Control Register 3         */
    __IO uint32_t GTPR;   /* 0x18: Guard time & prescaler     */
} USART_TypeDef;

/* ---- USART_SR (Status Register) bit positions ---- */
#define USART_SR_RXNE   (1U << 5)   /* read data register not empty */
#define USART_SR_TC     (1U << 6)   /* transmission complete        */
#define USART_SR_TXE    (1U << 7)   /* transmit data register empty */

/* ---- USART_CR1 (Control Register 1) bit positions ---- */
#define USART_CR1_RE      (1U << 2)   /* receiver enable            */
#define USART_CR1_TE      (1U << 3)   /* transmitter enable         */
#define USART_CR1_RXNEIE  (1U << 5)   /* RXNE interrupt enable      */
#define USART_CR1_TXEIE   (1U << 7)   /* TXE interrupt enable       */
#define USART_CR1_UE      (1U << 13)  /* USART enable               */

/* ============================================================
 *                  5e) DMA Register Structure
 *   DMA has a global part + 7 channels. ADC1 uses DMA1 channel 1.
 *   We model the global regs + one channel block.
 * ============================================================ */
typedef struct {
    __IO uint32_t CCR;      /* channel configuration            */
    __IO uint32_t CNDTR;    /* number of data to transfer       */
    __IO uint32_t CPAR;     /* peripheral address               */
    __IO uint32_t CMAR;     /* memory address                   */
} DMA_Channel_TypeDef;

typedef struct {
    __IO uint32_t ISR;      /* 0x00: interrupt status           */
    __IO uint32_t IFCR;     /* 0x04: interrupt flag clear       */
} DMA_TypeDef;

/* DMA channels: channel 1 starts at base + 0x08, each is 0x14 apart */
#define DMA1_Channel1_BASE  (DMA1_BASE + 0x0008UL)
#define DMA1_Channel2_BASE  (DMA1_BASE + 0x001CUL)
#define DMA1_Channel3_BASE  (DMA1_BASE + 0x0030UL)
#define DMA1_Channel4_BASE  (DMA1_BASE + 0x0044UL)
#define DMA1_Channel5_BASE  (DMA1_BASE + 0x0058UL)
#define DMA1_Channel6_BASE  (DMA1_BASE + 0x006CUL)
#define DMA1_Channel7_BASE  (DMA1_BASE + 0x0080UL)

/* ============================================================
 *                  5f) SPI Register Structure
 * ============================================================ */
typedef struct {
    __IO uint32_t CR1;     /* 0x00: Control Register 1   */
    __IO uint32_t CR2;     /* 0x04: Control Register 2   */
    __IO uint32_t SR;      /* 0x08: Status Register      */
    __IO uint32_t DR;      /* 0x0C: Data Register        */
    __IO uint32_t CRCPR;   /* 0x10 */
    __IO uint32_t RXCRCR;  /* 0x14 */
    __IO uint32_t TXCRCR;  /* 0x18 */
} SPI_TypeDef;

/* ---- SPI_CR1 bits ---- */
#define SPI_CR1_CPHA     (1U << 0)
#define SPI_CR1_CPOL     (1U << 1)
#define SPI_CR1_MSTR     (1U << 2)   /* master mode            */
#define SPI_CR1_SPE      (1U << 6)   /* SPI enable             */
#define SPI_CR1_SSI      (1U << 8)   /* internal slave select  */
#define SPI_CR1_SSM      (1U << 9)   /* software slave manage  */

/* ---- SPI_CR2 bits ---- */
#define SPI_CR2_RXNEIE   (1U << 6)   /* RX interrupt enable    */
#define SPI_CR2_TXEIE    (1U << 7)   /* TX interrupt enable    */

/* ---- SPI_SR bits ---- */
#define SPI_SR_RXNE      (1U << 0)   /* receive buffer not empty */
#define SPI_SR_TXE       (1U << 1)   /* transmit buffer empty    */
#define SPI_SR_BSY       (1U << 7)   /* busy flag                */

/* ============================================================
 *                  5g) I2C Register Structure
 * ============================================================ */
typedef struct {
    __IO uint32_t CR1;     /* 0x00: Control Register 1   */
    __IO uint32_t CR2;     /* 0x04: Control Register 2   */
    __IO uint32_t OAR1;    /* 0x08: Own Address 1        */
    __IO uint32_t OAR2;    /* 0x0C: Own Address 2        */
    __IO uint32_t DR;      /* 0x10: Data Register        */
    __IO uint32_t SR1;     /* 0x14: Status Register 1    */
    __IO uint32_t SR2;     /* 0x18: Status Register 2    */
    __IO uint32_t CCR;     /* 0x1C: Clock Control        */
    __IO uint32_t TRISE;   /* 0x20: Rise Time            */
} I2C_TypeDef;

/* ---- I2C_CR1 bits ---- */
#define I2C_CR1_PE       (1U << 0)    /* peripheral enable */
#define I2C_CR1_START    (1U << 8)    /* generate START    */
#define I2C_CR1_STOP     (1U << 9)    /* generate STOP     */
#define I2C_CR1_ACK      (1U << 10)   /* acknowledge enable */

/* ---- I2C_SR1 bits ---- */
#define I2C_SR1_SB       (1U << 0)    /* start bit sent      */
#define I2C_SR1_ADDR     (1U << 1)    /* address sent/matched */
#define I2C_SR1_BTF      (1U << 2)    /* byte transfer finished */
#define I2C_SR1_RXNE     (1U << 6)    /* data register not empty */
#define I2C_SR1_TXE      (1U << 7)    /* data register empty */

/* ---- I2C_SR2 bits ---- */
#define I2C_SR2_BUSY     (1U << 1)    /* bus busy */

/* ============================================================
 *                  5h) RTC / PWR Register Structures
 * ============================================================ */
typedef struct {
    __IO uint32_t CRH;    /* 0x00: Control High (interrupt enables) */
    __IO uint32_t CRL;    /* 0x04: Control Low (flags, config mode) */
    __IO uint32_t PRLH;   /* 0x08: Prescaler reload High            */
    __IO uint32_t PRLL;   /* 0x0C: Prescaler reload Low             */
    __IO uint32_t DIVH;   /* 0x10: Divider High (read-only)         */
    __IO uint32_t DIVL;   /* 0x14: Divider Low                      */
    __IO uint32_t CNTH;   /* 0x18: Counter High                     */
    __IO uint32_t CNTL;   /* 0x1C: Counter Low                      */
    __IO uint32_t ALRH;   /* 0x20: Alarm High                       */
    __IO uint32_t ALRL;   /* 0x24: Alarm Low                        */
} RTC_TypeDef;

typedef struct {
    __IO uint32_t CR;     /* 0x00: Power Control Register   */
    __IO uint32_t CSR;    /* 0x04: Power Control/Status     */
} PWR_TypeDef;

/* ---- RTC_CRL bits ---- */
#define RTC_CRL_SECF    (1U << 0)   /* second flag            */
#define RTC_CRL_ALRF    (1U << 1)   /* alarm flag             */
#define RTC_CRL_RSF     (1U << 3)   /* registers synchronized */
#define RTC_CRL_CNF     (1U << 4)   /* configuration mode     */
#define RTC_CRL_RTOFF   (1U << 5)   /* RTC operation off (ready) */

/* ---- RTC_CRH bits ---- */
#define RTC_CRH_SECIE   (1U << 0)   /* second interrupt enable */
#define RTC_CRH_ALRIE   (1U << 1)   /* alarm interrupt enable  */

/* ============================================================
 *           6) Ready-to-use peripheral pointers
 *  Now you can use GPIOA, GPIOC, RCC, AFIO, EXTI ... directly
 * ============================================================ */
#define RCC    ((RCC_TypeDef *)  RCC_BASE)
#define GPIOA  ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB  ((GPIO_TypeDef *) GPIOB_BASE)
#define GPIOC  ((GPIO_TypeDef *) GPIOC_BASE)
#define GPIOD  ((GPIO_TypeDef *) GPIOD_BASE)
#define AFIO   ((AFIO_TypeDef *) AFIO_BASE)
#define EXTI   ((EXTI_TypeDef *) EXTI_BASE)
#define TIM2   ((TIM_TypeDef *)  TIM2_BASE)
#define TIM3   ((TIM_TypeDef *)  TIM3_BASE)
#define TIM4   ((TIM_TypeDef *)  TIM4_BASE)
#define ADC1   ((ADC_TypeDef *)  ADC1_BASE)
#define USART1 ((USART_TypeDef *) USART1_BASE)
#define SPI1   ((SPI_TypeDef *) SPI1_BASE)
#define I2C1   ((I2C_TypeDef *) I2C1_BASE)
#define RTC    ((RTC_TypeDef *) RTC_BASE)
#define PWR    ((PWR_TypeDef *) PWR_BASE)
#define DMA1          ((DMA_TypeDef *) DMA1_BASE)
#define DMA1_Channel1 ((DMA_Channel_TypeDef *) DMA1_Channel1_BASE)
#define DMA1_Channel2 ((DMA_Channel_TypeDef *) DMA1_Channel2_BASE)
#define DMA1_Channel3 ((DMA_Channel_TypeDef *) DMA1_Channel3_BASE)
#define DMA1_Channel4 ((DMA_Channel_TypeDef *) DMA1_Channel4_BASE)
#define DMA1_Channel5 ((DMA_Channel_TypeDef *) DMA1_Channel5_BASE)
#define DMA1_Channel6 ((DMA_Channel_TypeDef *) DMA1_Channel6_BASE)
#define DMA1_Channel7 ((DMA_Channel_TypeDef *) DMA1_Channel7_BASE)

/* NVIC ISER as a simple array (ISER[0] covers IRQ 0..31) */
#define NVIC_ISER  ((__IO uint32_t *) NVIC_ISER_BASE)

/* ============================================================
 *                  7) EXTI IRQ Numbers (for the Blue Pill)
 *   These match the vector table positions.
 * ============================================================ */
#define EXTI0_IRQn      6
#define EXTI1_IRQn      7
#define EXTI2_IRQn      8
#define EXTI3_IRQn      9
#define EXTI4_IRQn      10
#define EXTI9_5_IRQn    23   /* shared: lines 5..9   */
#define EXTI15_10_IRQn  40   /* shared: lines 10..15 */

#define TIM2_IRQn       28
#define TIM3_IRQn       29
#define TIM4_IRQn       30
#define USART1_IRQn     37
#define SPI1_IRQn       35

/* RTC global interrupt */
#define RTC_IRQn        3

/* DMA1 channel IRQ numbers */
#define DMA1_Channel1_IRQn   11
#define DMA1_Channel2_IRQn   12
#define DMA1_Channel3_IRQn   13
#define DMA1_Channel4_IRQn   14
#define DMA1_Channel5_IRQn   15
#define DMA1_Channel6_IRQn   16
#define DMA1_Channel7_IRQn   17
/* ============================================================
 *                  8) Helper Macros
 * ============================================================ */

/* Enable an IRQ in the NVIC */
#define NVIC_ENABLE_IRQ(IRQn)   (NVIC_ISER[(IRQn) >> 5] = (1U << ((IRQn) & 0x1F)))

/* ---- NVIC Interrupt Priority Registers (IPR) ----
 *   8-bit priority field per IRQ, byte-addressable.
 *   Cortex-M3 implements the TOP 4 bits of each byte (bits 7:4);
 *   the lower 4 bits are ignored (read as 0). So a priority value
 *   must be shifted left by 4 -> use NVIC_SetPriority() below.
 */
#define NVIC_IPR_BASE  (0xE000E400UL)
#define NVIC_IPR       ((volatile uint8_t *) NVIC_IPR_BASE)

/**
 * @brief  Set the priority of an interrupt.
 * @param  IRQn     : interrupt number (e.g. EXTI0_IRQn)
 * @param  priority : 0 (highest) .. 15 (lowest), Cortex-M3 has 4
 *                     priority bits -> values 0..15 map to the
 *                     top 4 bits of the priority byte.
 *
 * @note   MUST be >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY's
 *         "priority" value (i.e. numerically >=) for any interrupt
 *         whose handler calls FreeRTOS "FromISR" APIs. A LOWER
 *         number means HIGHER priority in Cortex-M (0 = highest).
 */
#define NVIC_SetPriority(IRQn, priority) \
    (NVIC_IPR[(IRQn)] = (uint8_t)((priority) << 4))

/* Enable the AFIO clock (APB2ENR bit 0) */
#define RCC_AFIO_CLOCK_EN()     (RCC->APB2ENR |= (1U << 0))

/* Enable the USART1 clock (APB2ENR bit 14) */
#define RCC_USART1_CLOCK_EN()   (RCC->APB2ENR |= (1U << 14))

#endif /* STM32F103_REGS_H_ */
