/**
 ******************************************************************************
 * @file    stm32f103c6_rtc.c
 * @brief   RTC Driver Implementation for STM32F103C6
 *
 *   Design: the hardware counter holds the number of SECONDS since an
 *   epoch (we use 2000-01-01 00:00:00). We convert between that count
 *   and human Time/Date with simple integer math (no float).
 ******************************************************************************
 */

#include "stm32f103c6_rtc.h"

#define RTC_EPOCH_YEAR   2000

static RTC_AlarmCallback_t rtc_alarm_cb = 0;

/* ============================================================
 *          Low-level: enter / exit configuration mode
 *   You must wait for RTOFF, set CNF, write, clear CNF, wait RTOFF.
 * ============================================================ */
static void RTC_EnterConfig(void)
{
    while (!(RTC->CRL & RTC_CRL_RTOFF));   /* wait until ready */
    RTC->CRL |= RTC_CRL_CNF;               /* enter config mode */
}

static void RTC_ExitConfig(void)
{
    RTC->CRL &= ~RTC_CRL_CNF;              /* exit config mode */
    while (!(RTC->CRL & RTC_CRL_RTOFF));   /* wait write done  */
}

/* ============================================================
 *          Low-level: read / write the 32-bit counter
 * ============================================================ */
static uint32_t RTC_ReadCounter(void)
{
    return ((uint32_t)RTC->CNTH << 16) | (RTC->CNTL & 0xFFFF);
}

static void RTC_WriteCounter(uint32_t value)
{
    RTC_EnterConfig();
    RTC->CNTH = (value >> 16) & 0xFFFF;
    RTC->CNTL = value & 0xFFFF;
    RTC_ExitConfig();
}

/* ============================================================
 *          Helpers: leap year + days in month
 * ============================================================ */
static uint8_t RTC_IsLeap(uint16_t year)
{
    return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) ? 1 : 0;
}

static uint8_t RTC_DaysInMonth(uint8_t month, uint16_t year)
{
    static const uint8_t d[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (month == 2 && RTC_IsLeap(year)) return 29;
    return d[month - 1];
}

/* ============================================================
 *   Convert a full date+time to total seconds since the epoch.
 * ============================================================ */
static uint32_t RTC_ToSeconds(const RTC_Date_t *date, const RTC_Time_t *time)
{
    uint32_t days = 0;

    for (uint16_t y = RTC_EPOCH_YEAR; y < date->year; y++)
        days += RTC_IsLeap(y) ? 366 : 365;

    for (uint8_t m = 1; m < date->month; m++)
        days += RTC_DaysInMonth(m, date->year);

    days += (date->day - 1);

    return days * 86400UL
         + (uint32_t)time->hours   * 3600UL
         + (uint32_t)time->minutes * 60UL
         + time->seconds;
}

/* ============================================================
 *   Convert total seconds back to date + time.
 * ============================================================ */
static void RTC_FromSeconds(uint32_t total, RTC_Date_t *date, RTC_Time_t *time)
{
    uint32_t days = total / 86400UL;
    uint32_t rem  = total % 86400UL;

    time->hours   = (uint8_t)(rem / 3600UL);
    time->minutes = (uint8_t)((rem % 3600UL) / 60UL);
    time->seconds = (uint8_t)(rem % 60UL);

    uint16_t year = RTC_EPOCH_YEAR;
    while (1) {
        uint16_t dy = RTC_IsLeap(year) ? 366 : 365;
        if (days < dy) break;
        days -= dy;
        year++;
    }
    date->year = year;

    uint8_t month = 1;
    while (1) {
        uint8_t dm = RTC_DaysInMonth(month, year);
        if (days < dm) break;
        days -= dm;
        month++;
    }
    date->month = month;
    date->day   = (uint8_t)(days + 1);
}

/* ============================================================
 *                    RTC_Init
 * ============================================================ */
void RTC_Init(void)
{
    /* ---- 1) enable PWR + BKP clocks (APB1) ---- */
    RCC->APB1ENR |= (1U << 28);   /* PWR */
    RCC->APB1ENR |= (1U << 27);   /* BKP */

    /* ---- 2) allow access to the backup domain ---- */
    PWR->CR |= (1U << 8);         /* DBP: disable backup write protection */

    /* ---- 3) if RTC already running (battery kept it), skip re-init ----
     *   RTCEN is bit 15 of RCC->BDCR. If set, the clock survived.
     */
    if (!(RCC->BDCR & (1U << 15)))
    {
        /* start the 32.768 kHz external crystal (LSE) */
        RCC->BDCR |= (1U << 0);              /* LSEON */
        while (!(RCC->BDCR & (1U << 1)));     /* wait LSERDY */

        /* select LSE as the RTC clock source + enable RTC */
        RCC->BDCR |= (1U << 8);              /* RTCSEL = 01 (LSE) */
        RCC->BDCR |= (1U << 15);             /* RTCEN */

        /* wait registers synchronized */
        RTC->CRL &= ~RTC_CRL_RSF;
        while (!(RTC->CRL & RTC_CRL_RSF));

        /* prescaler: 32768-1 = 32767 -> 1 Hz tick */
        RTC_EnterConfig();
        RTC->PRLH = 0;
        RTC->PRLL = 32767;
        RTC_ExitConfig();
    }
}

/* ============================================================
 *                    Time get / set
 *   We keep the date part by reading the whole counter, replacing
 *   only the time-of-day, and writing back.
 * ============================================================ */
void RTC_SetTime(const RTC_Time_t *time)
{
    RTC_Date_t date;
    RTC_Time_t dummy;
    RTC_FromSeconds(RTC_ReadCounter(), &date, &dummy);   /* keep date */
    RTC_WriteCounter(RTC_ToSeconds(&date, time));
}

void RTC_GetTime(RTC_Time_t *time)
{
    RTC_Date_t date;
    RTC_FromSeconds(RTC_ReadCounter(), &date, time);
}

void RTC_SetDate(const RTC_Date_t *date)
{
    RTC_Date_t dummy;
    RTC_Time_t time;
    RTC_FromSeconds(RTC_ReadCounter(), &dummy, &time);   /* keep time */
    RTC_WriteCounter(RTC_ToSeconds(date, &time));
}

void RTC_GetDate(RTC_Date_t *date)
{
    RTC_Time_t time;
    RTC_FromSeconds(RTC_ReadCounter(), date, &time);
}

/* ============================================================
 *                    Alarm
 *   The alarm register holds an absolute second count. We set it
 *   to today's (or tomorrow's) target time and enable the interrupt.
 * ============================================================ */
void RTC_SetAlarm(const RTC_Time_t *time, RTC_AlarmCallback_t callback)
{
    rtc_alarm_cb = callback;

    /* build the alarm's absolute second count using today's date */
    RTC_Date_t date;
    RTC_Time_t now;
    uint32_t current = RTC_ReadCounter();
    RTC_FromSeconds(current, &date, &now);

    uint32_t alarm = RTC_ToSeconds(&date, time);
    if (alarm <= current)            /* already passed today -> tomorrow */
        alarm += 86400UL;

    /* write the alarm register */
    RTC_EnterConfig();
    RTC->ALRH = (alarm >> 16) & 0xFFFF;
    RTC->ALRL = alarm & 0xFFFF;
    RTC_ExitConfig();

    /* enable the alarm interrupt */
    RTC->CRH |= RTC_CRH_ALRIE;
    NVIC_ENABLE_IRQ(RTC_IRQn);
}

/* ============================================================
 *                    RTC Interrupt Handler
 * ============================================================ */
void RTC_IRQHandler(void)
{
    if (RTC->CRL & RTC_CRL_ALRF)        /* alarm flag */
    {
        RTC->CRL &= ~RTC_CRL_ALRF;      /* clear it */
        if (rtc_alarm_cb)
            rtc_alarm_cb();
    }
}
