/**
 ******************************************************************************
 * @file    stm32f103c6_i2c.c
 * @brief   I2C1 Master Driver Implementation for STM32F103C6
 *
 *   All blocking waits are bounded by a TIMEOUT so a missing/!ACK
 *   device or a momentary bus glitch can never hang the program
 *   forever. On timeout, a STOP is issued and the call returns.
 ******************************************************************************
 */

#include "stm32f103c6_i2c.h"

/* how many spins each wait loop may take before giving up.
 * ~ tens of ms at 8 MHz; large enough for normal traffic. */
#ifndef I2C_TIMEOUT
#define I2C_TIMEOUT  200000UL
#endif

/* ============================================================
 *                    I2C_Init
 *   Master, standard mode 100 kHz. SCL=PB6, SDA=PB7.
 * ============================================================ */
void I2C_Init(void)
{
    /* ---- 1) clocks: GPIOB (APB2 bit3) + I2C1 (APB1 bit21) ---- */
    RCC->APB2ENR |= (1U << 3);     /* GPIOB */
    RCC->APB1ENR |= (1U << 21);    /* I2C1  */

    /* ---- 2) PB6 (SCL) + PB7 (SDA) as AF open-drain ---- */
    GPIO_InitPin(GPIOB, GPIO_PIN_6, GPIO_MODE_OUTPUT_AF_OD);
    GPIO_InitPin(GPIOB, GPIO_PIN_7, GPIO_MODE_OUTPUT_AF_OD);

    /* ---- 3) software bus reset sequence ----
     *   If another peripheral (ADC, DMA) toggled GPIOB before us,
     *   the I2C bus might be stuck BUSY. We do a software reset:
     *   set SWRST, clear it, then reconfigure. This also releases
     *   any slave that may have been mid-transaction.
     */
    I2C1->CR1 |=  (1U << 15);   /* SWRST = 1: reset the peripheral */
    for (volatile uint32_t i = 0; i < 1000; i++);
    I2C1->CR1 &= ~(1U << 15);   /* SWRST = 0: release reset        */

    /* ---- 4) make sure peripheral is off while configuring ---- */
    I2C1->CR1 &= ~I2C_CR1_PE;

    /* ---- 5) tell I2C the APB1 clock frequency (CR2 FREQ) ---- */
    I2C1->CR2 = I2C_APB1_MHZ;

    /* ---- 6) clock control for 100 kHz (standard mode) ---- */
    I2C1->CCR = (I2C_APB1_MHZ * 1000000U) / (2U * 100000U);

    /* ---- 7) max rise time = APB1_MHz + 1 ---- */
    I2C1->TRISE = I2C_APB1_MHZ + 1U;

    /* ---- 8) enable the peripheral ---- */
    I2C1->CR1 |= I2C_CR1_PE;

    /* ---- 9) short settling delay after enable ---- */
    for (volatile uint32_t i = 0; i < 5000; i++);
}

/* ============================================================
 *   Internal: wait for a SR1 flag with timeout.
 *   Returns 1 if the flag appeared, 0 on timeout.
 * ============================================================ */
static uint8_t I2C_WaitFlag(uint32_t flag)
{
    uint32_t t = I2C_TIMEOUT;
    while (!(I2C1->SR1 & flag)) {
        if (--t == 0)
            return 0;
    }
    return 1;
}

/* ============================================================
 *                    Low-level primitives
 *   Same signatures as before; now timeout-protected so they
 *   can never hang. On timeout they issue a STOP and return.
 * ============================================================ */

void I2C_Start(void)
{
    I2C1->CR1 |= I2C_CR1_ACK;              /* enable ACK            */
    I2C1->CR1 |= I2C_CR1_START;            /* generate START        */
    if (!I2C_WaitFlag(I2C_SR1_SB))         /* wait START sent (SB)  */
        I2C1->CR1 |= I2C_CR1_STOP;
}

void I2C_Stop(void)
{
    I2C1->CR1 |= I2C_CR1_STOP;             /* generate STOP */
}

/* send the 7-bit address + read/write bit, wait for ADDR, clear it */
void I2C_SendAddress(uint8_t address, uint8_t read)
{
    uint8_t byte = (uint8_t)((address << 1) | (read ? 1U : 0U));
    I2C1->DR = byte;

    /* wait ADDR; bail out (STOP) on timeout or acknowledge-failure */
    uint32_t t = I2C_TIMEOUT;
    while (1) {
        uint32_t sr1 = I2C1->SR1;
        if (sr1 & I2C_SR1_ADDR) break;            /* addressed OK */
        if (sr1 & (1U << 10)) {                   /* AF: no ACK   */
            I2C1->SR1 &= ~(1U << 10);
            I2C1->CR1 |= I2C_CR1_STOP;
            return;
        }
        if (--t == 0) {
            I2C1->CR1 |= I2C_CR1_STOP;
            return;
        }
    }

    /* clear ADDR by reading SR1 then SR2 */
    (void)I2C1->SR1;
    (void)I2C1->SR2;
}

void I2C_WriteByte(uint8_t data)
{
    if (!I2C_WaitFlag(I2C_SR1_TXE)) {      /* wait TX empty   */
        I2C1->CR1 |= I2C_CR1_STOP;
        return;
    }
    I2C1->DR = data;
    if (!I2C_WaitFlag(I2C_SR1_BTF)) {      /* wait byte done  */
        I2C1->CR1 |= I2C_CR1_STOP;
        return;
    }
}

uint8_t I2C_ReadByte_ACK(void)
{
    I2C1->CR1 |= I2C_CR1_ACK;              /* ACK after this byte */
    if (!I2C_WaitFlag(I2C_SR1_RXNE))       /* wait data           */
        return 0;
    return (uint8_t)I2C1->DR;
}

uint8_t I2C_ReadByte_NACK(void)
{
    I2C1->CR1 &= ~I2C_CR1_ACK;             /* NACK = last byte */
    if (!I2C_WaitFlag(I2C_SR1_RXNE))
        return 0;
    return (uint8_t)I2C1->DR;
}

/* ============================================================
 *                    High-level: write one register
 *   START -> addr(W) -> reg -> value -> STOP
 * ============================================================ */
void I2C_WriteReg(uint8_t dev_addr, uint8_t reg, uint8_t value)
{
    I2C_Start();
    I2C_SendAddress(dev_addr, 0);   /* write */
    I2C_WriteByte(reg);             /* register pointer */
    I2C_WriteByte(value);           /* the value        */
    I2C_Stop();
}

/* ============================================================
 *                    High-level: read one register
 * ============================================================ */
uint8_t I2C_ReadReg(uint8_t dev_addr, uint8_t reg)
{
    uint8_t value;

    I2C_Start();
    I2C_SendAddress(dev_addr, 0);   /* write the register pointer */
    I2C_WriteByte(reg);

    I2C_Start();                    /* repeated START */
    I2C_SendAddress(dev_addr, 1);   /* read */
    value = I2C_ReadByte_NACK();    /* single byte -> NACK */
    I2C_Stop();

    return value;
}

/* ============================================================
 *                    High-level: read multiple registers
 * ============================================================ */
void I2C_ReadRegs(uint8_t dev_addr, uint8_t reg, uint8_t *buffer, uint8_t count)
{
    I2C_Start();
    I2C_SendAddress(dev_addr, 0);
    I2C_WriteByte(reg);

    I2C_Start();
    I2C_SendAddress(dev_addr, 1);   /* read */

    for (uint8_t i = 0; i < count; i++)
    {
        if (i == count - 1)
            buffer[i] = I2C_ReadByte_NACK();   /* last byte: NACK */
        else
            buffer[i] = I2C_ReadByte_ACK();    /* more coming: ACK */
    }

    I2C_Stop();
}

/* ============================================================
 *          I2C_WriteBuffer : robust block write (timeouts)
 *
 *   Self-contained — does NOT call the other primitives, so a
 *   single failed step cannot leave the bus half-open for the
 *   next call. Mirrors the proven sequence used while bringing
 *   up the OLED. For consecutive data bytes we wait on TXE (not
 *   BTF) and check BTF only once at the end, which is the
 *   reliable pattern for long streams.
 * ============================================================ */
uint8_t I2C_WriteBuffer(uint8_t dev_addr, uint8_t ctrl,
                        const uint8_t *data, uint16_t len)
{
    uint32_t t;

    /* wait until bus is free (BUSY flag in SR2 bit 1) */
    t = I2C_TIMEOUT;
    while ((I2C1->SR2 & (1U << 1)) && --t);   /* BUSY: wait */
    if (t == 0) return 0;                      /* bus stuck  */

    /* START */
    I2C1->CR1 |= I2C_CR1_START;
    t = I2C_TIMEOUT;
    while (!(I2C1->SR1 & I2C_SR1_SB)) { if (--t == 0) return 0; }

    /* address + write bit */
    I2C1->DR = (uint8_t)(dev_addr << 1);
    t = I2C_TIMEOUT;
    while (1) {
        uint32_t sr1 = I2C1->SR1;
        if (sr1 & I2C_SR1_ADDR) { (void)I2C1->SR1; (void)I2C1->SR2; break; }
        if (sr1 & (1U << 10)) {                 /* AF: no ACK */
            I2C1->SR1 &= ~(1U << 10);
            I2C1->CR1 |= I2C_CR1_STOP;
            return 0;
        }
        if (--t == 0) { I2C1->CR1 |= I2C_CR1_STOP; return 0; }
    }

    /* optional control byte (0xFF means "none") */
    if (ctrl != 0xFF) {
        t = I2C_TIMEOUT;
        while (!(I2C1->SR1 & I2C_SR1_TXE)) { if (--t == 0) { I2C1->CR1 |= I2C_CR1_STOP; return 0; } }
        I2C1->DR = ctrl;
    }

    /* payload: write each byte as soon as TXE is set */
    for (uint16_t i = 0; i < len; i++) {
        t = I2C_TIMEOUT;
        while (!(I2C1->SR1 & I2C_SR1_TXE)) { if (--t == 0) { I2C1->CR1 |= I2C_CR1_STOP; return 0; } }
        I2C1->DR = data[i];
    }

    /* make sure the last byte fully shifted out (BTF) before STOP */
    t = I2C_TIMEOUT;
    while (!(I2C1->SR1 & I2C_SR1_BTF)) { if (--t == 0) { I2C1->CR1 |= I2C_CR1_STOP; return 0; } }

    I2C1->CR1 |= I2C_CR1_STOP;
    return 1;
}
