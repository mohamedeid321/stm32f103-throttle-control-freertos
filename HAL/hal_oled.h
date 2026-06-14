/**
 ******************************************************************************
 * @file    hal_oled.h
 * @brief   0.96" OLED (I2C) text driver for STM32F103C6
 * @author  Mohamed
 * @layer   HAL (uses MCAL: I2C only)
 *
 * @note    Supports the two common 0.96"/1.3" controllers:
 *            - SSD1306 (DEFAULT)
 *            - SH1106  (build with:  #define OLED_CONTROLLER_SH1106)
 *          Both are driven in PAGE addressing mode so the same code
 *          works for the two chips (SH1106 only differs by a 2-column
 *          offset and has no horizontal mode anyway).
 *
 *          SCREEN LAYOUT (text mode):
 *            128x64 pixels = 8 rows (pages) x 21 columns of 6x8 chars
 *            row: 0..7   col: 0..20
 *
 *          WIRING (4-pin module): VCC, GND, SCL=PB6, SDA=PB7
 *          (pull-ups usually on the module itself).
 *
 *          Example:
 *            OLED_Init();
 *            OLED_PrintAt(0, 0, "Temp:");
 *            OLED_SetCursor(0, 6);
 *            OLED_PrintNumber(25);
 ******************************************************************************
 */

#ifndef HAL_OLED_H_
#define HAL_OLED_H_

#include "stm32f103c6_i2c.h"
#include <stdint.h>

/* ---- controller selection (SSD1306 is the default) ---- */
/* #define OLED_CONTROLLER_SH1106 */

/* 7-bit I2C address (0x3C on almost all modules, some use 0x3D) */
#ifndef OLED_I2C_ADDR
#define OLED_I2C_ADDR   0x3C
#endif

#define OLED_ROWS   8     /* text rows    (pages)        */
#define OLED_COLS   21    /* text columns (128/6 = 21)   */

/* ============================================================
 *                  Function Prototypes
 * ============================================================ */

/**
 * @brief  Initialize the display controller (SSD1306 or SH1106).
 *         I2C_Init() MUST be called BEFORE this function.
 *         Clears the screen on exit.
 *
 *   Correct call order:
 *     LM35_Init(...);   // ADC/DMA first
 *     I2C_Init();       // then I2C
 *     OLED_Init();      // then OLED
 */
void OLED_Init(void);

/** @brief Clear the whole screen (all pixels off). */
void OLED_Clear(void);

/** @brief Move the text cursor. row 0..7, col 0..20. */
void OLED_SetCursor(uint8_t row, uint8_t col);

/** @brief Print one character at the cursor (cursor advances). */
void OLED_PrintChar(char c);

/** @brief Print a string at the cursor. */
void OLED_PrintString(const char *str);

/** @brief Print an unsigned number at the cursor. */
void OLED_PrintNumber(uint32_t num);

/**
 * @brief  Convenience: print a string at a given position, then PAD
 *         the rest of the line with spaces (great for dashboards —
 *         old text is wiped without flicker).
 */
void OLED_PrintLine(uint8_t row, const char *str);

/** @brief Print a string at a given row/col (no padding). */
void OLED_PrintAt(uint8_t row, uint8_t col, const char *str);

/** @brief Set brightness 0..255. */
void OLED_SetContrast(uint8_t value);

/**
 * @brief  Invert the display. This is the only "colour-like" control a
 *         monochrome OLED has: it swaps lit and unlit pixels.
 * @param  on : 1 = inverted (background lit, text dark),
 *              0 = normal   (background dark, text lit).
 */
void OLED_Invert(uint8_t on);

#endif /* HAL_OLED_H_ */
