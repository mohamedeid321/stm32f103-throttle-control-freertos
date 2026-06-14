/**
 ******************************************************************************
 * @file    stm32f103c6_i2c.h
 * @brief   I2C (I2C1) Master Driver for STM32F103C6
 * @author  Mohamed
 * @note    Register-level driver (no HAL). Master mode, 7-bit addresses.
 *          I2C1 pins: SCL = PB6, SDA = PB7 (alternate-function open-drain).
 *          Remember external pull-up resistors (~4.7k) on SDA and SCL.
 *
 *          High-level helpers (the usual way you talk to a sensor):
 *            - I2C_WriteReg : write one byte to a register
 *            - I2C_ReadReg  : read one byte from a register
 *            - I2C_ReadRegs : read multiple bytes from a register
 ******************************************************************************
 */

#ifndef STM32F103C6_I2C_H_
#define STM32F103C6_I2C_H_

#include "stm32f103_regs.h"
#include "stm32f103c6_gpio.h"
#include <stdint.h>

/* APB1 clock in MHz (used for I2C timing). 8 Proteus / 36 real (max APB1). */
#ifndef I2C_APB1_MHZ
#define I2C_APB1_MHZ   8U
#endif

/* ============================================================
 *                  Function Prototypes
 * ============================================================ */

/**
 * @brief  Initialize I2C1 as master at standard mode (100 kHz).
 *         Sets PB6/PB7 as alternate-function open-drain.
 */
void I2C_Init(void);

/* ---- Low-level primitives (if you need raw control) ---- */
void    I2C_Start(void);
void    I2C_Stop(void);
void    I2C_WriteByte(uint8_t data);
uint8_t I2C_ReadByte_ACK(void);    /* read + send ACK (more bytes coming) */
uint8_t I2C_ReadByte_NACK(void);   /* read + send NACK (last byte)        */
void    I2C_SendAddress(uint8_t address, uint8_t read);  /* read=1 to read */

/* ---- High-level register access (typical sensor usage) ---- */

/**
 * @brief  Write one byte to a register of a slave device.
 * @param  dev_addr : 7-bit slave address
 * @param  reg      : register address inside the slave
 * @param  value    : byte to write
 */
void I2C_WriteReg(uint8_t dev_addr, uint8_t reg, uint8_t value);

/**
 * @brief  Read one byte from a register of a slave device.
 */
uint8_t I2C_ReadReg(uint8_t dev_addr, uint8_t reg);

/**
 * @brief  Read several consecutive bytes starting at a register.
 * @param  buffer : where to store the bytes
 * @param  count  : how many bytes to read
 */
void I2C_ReadRegs(uint8_t dev_addr, uint8_t reg, uint8_t *buffer, uint8_t count);

/**
 * @brief  Robust, self-contained block write with timeouts at EVERY
 *         step (START, address, each byte, STOP). Sends:
 *           START -> addr(W) -> ctrl -> data[0..len-1] -> STOP
 *         This is the safe primitive for displays (OLED command/data
 *         streams) where a stuck flag must never hang the program.
 * @param  dev_addr : 7-bit slave address
 * @param  ctrl     : first byte after the address (e.g. 0x00=cmd,
 *                    0x40=data for SSD1306). Pass 0xFF to skip it.
 * @param  data     : payload bytes (may be NULL if len==0)
 * @param  len      : number of payload bytes
 * @return 1 on success, 0 if any step timed out.
 */
uint8_t I2C_WriteBuffer(uint8_t dev_addr, uint8_t ctrl,
                        const uint8_t *data, uint16_t len);

#endif /* STM32F103C6_I2C_H_ */
