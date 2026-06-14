/**
 ******************************************************************************
 * @file    stm32f103c6_rtc.h
 * @brief   Real-Time Clock (RTC) Driver for STM32F103C6
 * @author  Mohamed
 * @note    Register-level driver (no HAL).
 *
 *          The STM32F103 RTC is a 32-bit counter of SECONDS, clocked by
 *          a 32.768 kHz crystal (LSE) and divided down to 1 Hz. It keeps
 *          running on the VBAT battery even when the main power is off.
 *
 *          This driver stores time as "seconds since midnight + a day
 *          count", and provides easy Time/Date structures so you don't
 *          deal with the raw second counter yourself.
 ******************************************************************************
 */

#ifndef STM32F103C6_RTC_H_
#define STM32F103C6_RTC_H_

#include "stm32f103_regs.h"
#include <stdint.h>

/* ============================================================
 *                  Time / Date structures
 * ============================================================ */
typedef struct {
    uint8_t hours;     /* 0..23 */
    uint8_t minutes;   /* 0..59 */
    uint8_t seconds;   /* 0..59 */
} RTC_Time_t;

typedef struct {
    uint8_t  day;      /* 1..31 */
    uint8_t  month;    /* 1..12 */
    uint16_t year;     /* e.g. 2026 */
} RTC_Date_t;

/* alarm callback (called from the RTC interrupt) */
typedef void (*RTC_AlarmCallback_t)(void);

/* ============================================================
 *                  Function Prototypes
 * ============================================================ */

/**
 * @brief  Initialize the RTC: enable LSE 32.768 kHz, set 1 Hz tick.
 *         Safe to call on every boot — it only re-initializes the
 *         counter the first time (so the time survives resets while
 *         VBAT is present).
 */
void RTC_Init(void);

/** @brief Set the current time (hours/minutes/seconds). */
void RTC_SetTime(const RTC_Time_t *time);

/** @brief Read the current time. */
void RTC_GetTime(RTC_Time_t *time);

/** @brief Set the current date (day/month/year). */
void RTC_SetDate(const RTC_Date_t *date);

/** @brief Read the current date. */
void RTC_GetDate(RTC_Date_t *date);

/**
 * @brief  Set an alarm at a given time of day. When the time is
 *         reached, the callback is called (from the RTC interrupt).
 */
void RTC_SetAlarm(const RTC_Time_t *time, RTC_AlarmCallback_t callback);

#endif /* STM32F103C6_RTC_H_ */
