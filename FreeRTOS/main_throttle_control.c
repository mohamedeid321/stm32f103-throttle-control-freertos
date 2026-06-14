/**
 ******************************************************************************
 * @file    main_throttle_control.c
 * @brief   THROTTLE CONTROL — FreeRTOS project.
 *
 *   A push button (EXTI) toggles the motor START/STOP. While STOPPED,
 *   the motor is fully off and the potentiometer is ignored. While
 *   RUNNING, the potentiometer controls motor speed live.
 *
 *   TASKS:
 *     Task_Blink    (128, prio1) : heartbeat PC13
 *     Task_ADC      (256, prio3) : reads LM35 (motor temp) + POT
 *     Task_Display  (256, prio2) : OLED "THROTTLE CONTROL" dashboard
 *     Task_LEDs     (128, prio2) : 3 status LEDs
 *     Task_Motor    (128, prio2) : applies speed only when RUNNING
 *     Task_Watchdog (128, prio1) : feeds the IWDG (see below)
 *
 *   BUTTON (EXTI):
 *     PB5, falling edge, debounced (PB3/PB4 avoided - JTAG pins).
 *     Each press toggles motor state:
 *       STOP (default at boot) <-> RUN
 *
 *   LED LOGIC:
 *     PB12 (temp_led) : ON  when motor temperature > 28.0 C
 *     PB13 (duty_led) : blinks every 1s when POT duty > 50% AND running
 *     PB14 (error_led): ON  when error flag set
 *
 *   OLED LAYOUT:
 *     row0: THROTTLE CONTROL
 *     row1: ----------------
 *     row3: State: RUN / STOP
 *     row4: Speed: xx %
 *     row5: Temp:  xx.x C
 *
 *   WIRING:
 *     LM35 OUT -> PA1 (ADC CH1)   - motor temperature
 *     POT  OUT -> PA0 (ADC CH0)   - throttle
 *     Button   -> PB5 (to GND, internal pull-up)
 *     H-Bridge : IN1=PB10, IN2=PB11, EN(PWM)=PB0 (TIM3 CH3)
 *     LEDs     : PB12, PB13, PB14
 *     OLED     : I2C1 (PB6/PB7)
 *
 *   WATCHDOG (IWDG) — "all tasks must report alive":
 *     Every task sets its OWN bit in g_task_alive_flags once per
 *     loop iteration. A dedicated Task_Watchdog checks the flags
 *     every WATCHDOG_CHECK_PERIOD_MS:
 *       - if ALL bits are set  -> IWDG_Refresh() and clear all bits
 *       - if ANY bit is missing -> do nothing (skip the refresh)
 *     If even ONE task hangs, its bit stops being set, the refresh
 *     stops happening, and after the IWDG timeout (~2s) the MCU
 *     resets automatically — recovering from any single-task hang.
 ******************************************************************************
 */

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "hal_lm35.h"
#include "hal_pot.h"
#include "hal_analog.h"
#include "hal_oled.h"
#include "hal_led.h"
#include "hal_button.h"
#include "hal_hbridge.h"
#include "stm32f103c6_i2c.h"
#include "stm32f103c6_gpio.h"
#include "stm32f103c6_iwdg.h"

#define TEMP_THRESHOLD_CX10   280   /* 28.0 C */
#define DUTY_THRESHOLD_PCT     50   /* 50 %   */

/* ============================================================
 *   Watchdog: per-task "alive" bits
 * ============================================================ */
#define WD_BIT_BLINK     (1U << 0)
#define WD_BIT_ADC       (1U << 1)
#define WD_BIT_DISPLAY   (1U << 2)
#define WD_BIT_LEDS      (1U << 3)
#define WD_BIT_MOTOR     (1U << 4)
#define WD_ALL_TASKS     (WD_BIT_BLINK | WD_BIT_ADC | WD_BIT_DISPLAY | \
                          WD_BIT_LEDS  | WD_BIT_MOTOR)

/* must be >= the slowest task period (500ms) so every task gets a
 * chance to set its bit between two consecutive watchdog checks */
#define WATCHDOG_CHECK_PERIOD_MS   600

/* IWDG timeout ~2.0s : (4 * 2^PR) * RLR / 40000
 *   PR=4 -> divider=64 ; RLR=1250 -> 64*1250/40000 = 2.0s          */
#define IWDG_PRESCALER   4
#define IWDG_RELOAD      1250

static volatile uint8_t g_task_alive_flags = 0;

/* ---- shared data ---- */
static volatile uint16_t g_temp_cx10 = 0;   /* motor temperature x10 */
static volatile uint8_t  g_duty_pct  = 0;   /* throttle from POT     */
static volatile uint8_t  g_error     = 0;
static volatile uint8_t  g_running   = 0;   /* 0=STOP (default), 1=RUN */
static SemaphoreHandle_t g_mutex;

/* ============================================================
 *   Button callback (EXTI, runs in interrupt context)
 *   Toggles g_running. Kept short — no I2C/OLED here.
 * ============================================================ */
static void Button_OnPress(void)
{
    g_running = (uint8_t)!g_running;
}

/* ---- Task_Blink (128, prio1) ---- */
static void Task_Blink(void *pv)
{
    (void)pv;
    for (;;)
    {
        GPIO_TOGGLE_PIN(GPIOC, GPIO_PIN_13);

        g_task_alive_flags |= WD_BIT_BLINK;   /* "I'm alive" */

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/* ---- Task_ADC (256, prio3) ---- */
static void Task_ADC(void *pv)
{
    (void)pv;
    TickType_t last = xTaskGetTickCount();

    for (;;)
    {
        uint16_t t = LM35_ReadTempCx10();   /* motor temperature */
        uint8_t  d = POT_ReadPercent();     /* throttle position */

        if (xSemaphoreTake(g_mutex, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            g_temp_cx10 = t;
            g_duty_pct  = d;
            xSemaphoreGive(g_mutex);
        }

        g_task_alive_flags |= WD_BIT_ADC;   /* "I'm alive" */

        vTaskDelayUntil(&last, pdMS_TO_TICKS(200));
    }
}

/* ---- Task_Display (256, prio2) ---- */
static void Task_Display(void *pv)
{
    (void)pv;

    OLED_PrintAt(0, 0, "THROTTLE CONTROL");
    OLED_PrintAt(1, 0, "----------------");

    TickType_t last = xTaskGetTickCount();

    for (;;)
    {
        uint16_t t = 0;
        uint8_t  d = 0;
        uint8_t  e = 0;
        uint8_t  r = 0;

        if (xSemaphoreTake(g_mutex, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            t = g_temp_cx10;
            d = g_duty_pct;
            e = g_error;
            r = g_running;
            xSemaphoreGive(g_mutex);
        }

        /* State row */
        OLED_SetCursor(3, 0);
        OLED_PrintString("State: ");
        OLED_PrintString(r ? "RUN " : "STOP");
        OLED_PrintString("      ");

        /* Speed row — 0 if stopped, even if POT is turned */
        OLED_SetCursor(4, 0);
        OLED_PrintString("Speed: ");
        OLED_PrintNumber(r ? d : 0);
        OLED_PrintString(" %   ");

        /* Temp row (motor temperature) */
        OLED_SetCursor(5, 0);
        OLED_PrintString("Temp:  ");
        OLED_PrintNumber(t / 10);
        OLED_PrintChar('.');
        OLED_PrintNumber(t % 10);
        OLED_PrintString(" C   ");

        /* error row */
        if (e)
            OLED_PrintLine(7, "!! ERROR !!");
        else
            OLED_PrintLine(7, "           ");

        g_task_alive_flags |= WD_BIT_DISPLAY;   /* "I'm alive" */

        vTaskDelayUntil(&last, pdMS_TO_TICKS(500));
    }
}

/* ---- Task_LEDs (128, prio2) ---- */
static void Task_LEDs(void *pv)
{
    (void)pv;

    LED_t temp_led  = LED_Create(GPIOB, GPIO_PIN_12);
    LED_t duty_led  = LED_Create(GPIOB, GPIO_PIN_13);
    LED_t error_led = LED_Create(GPIOB, GPIO_PIN_14);

    TickType_t last = xTaskGetTickCount();

    for (;;)
    {
        uint16_t t = 0;
        uint8_t  d = 0;
        uint8_t  e = 0;
        uint8_t  r = 0;

        if (xSemaphoreTake(g_mutex, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            t = g_temp_cx10;
            d = g_duty_pct;
            e = g_error;
            r = g_running;
            xSemaphoreGive(g_mutex);
        }

        /* PB12: motor temperature */
        if (t > TEMP_THRESHOLD_CX10) LED_ON(temp_led); else LED_OFF(temp_led);

        /* PB13: throttle > 50% AND motor running -> blink 1s */
        if (r && d > DUTY_THRESHOLD_PCT)
            LED_TOGGLE(duty_led);
        else
            LED_OFF(duty_led);

        /* PB14: error */
        if (e) LED_ON(error_led); else LED_OFF(error_led);

        g_task_alive_flags |= WD_BIT_LEDS;   /* "I'm alive" */

        vTaskDelayUntil(&last, pdMS_TO_TICKS(500));
    }
}

/* ============================================================
 *   Task_Motor (128, prio2)
 *   Applies POT speed ONLY when running; fully stops otherwise.
 * ============================================================ */
static void Task_Motor(void *pv)
{
    (void)pv;

    HBridge_Config_t mcfg = HBRIDGE_GetDefaultConfig();
    HBRIDGE_Init(&mcfg);
    HBRIDGE_SetDirection(HBRIDGE_FORWARD);
    HBRIDGE_SetSpeedPercent(0);   /* start stopped */

    TickType_t last = xTaskGetTickCount();

    for (;;)
    {
        uint8_t d = 0;
        uint8_t r = 0;

        if (xSemaphoreTake(g_mutex, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            d = g_duty_pct;
            r = g_running;
            xSemaphoreGive(g_mutex);
        }

        if (r)
            HBRIDGE_SetSpeedPercent(d);   /* RUN: follow the pot */
        else
            HBRIDGE_SetSpeedPercent(0);   /* STOP: fully off     */

        g_task_alive_flags |= WD_BIT_MOTOR;   /* "I'm alive" */

        vTaskDelayUntil(&last, pdMS_TO_TICKS(100));
    }
}

/* ============================================================
 *   Task_Watchdog (128, prio1)
 *
 *   Every WATCHDOG_CHECK_PERIOD_MS, checks whether ALL other tasks
 *   have set their "alive" bit since the last check:
 *     - all bits set   -> IWDG_Refresh() (system is healthy),
 *                          then clear all bits for the next round
 *     - any bit missing -> do nothing. If this repeats for ~2s
 *                          straight, the IWDG resets the MCU.
 * ============================================================ */
static void Task_Watchdog(void *pv)
{
    (void)pv;
    TickType_t last = xTaskGetTickCount();

    for (;;)
    {
        if ((g_task_alive_flags & WD_ALL_TASKS) == WD_ALL_TASKS)
        {
            IWDG_Refresh();
            g_task_alive_flags = 0;   /* start the next round */
        }
        /* else: at least one task missed its turn -> skip the
         * refresh. If the hung task never recovers, the IWDG
         * timeout will reset the whole system.                 */

        vTaskDelayUntil(&last, pdMS_TO_TICKS(WATCHDOG_CHECK_PERIOD_MS));
    }
}

/* ---- hooks ---- */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask; (void)pcTaskName;
    GPIO_EnableClock(GPIOC);
    GPIO_InitPin(GPIOC, GPIO_PIN_13, GPIO_MODE_OUTPUT_PP);
    for(;;) {
        GPIO_TOGGLE_PIN(GPIOC, GPIO_PIN_13);
        for (volatile uint32_t i=0;i<100000;i++);
    }
}
void vApplicationMallocFailedHook(void)
{
    GPIO_EnableClock(GPIOC);
    GPIO_InitPin(GPIOC, GPIO_PIN_13, GPIO_MODE_OUTPUT_PP);
    for(;;) {
        GPIO_TOGGLE_PIN(GPIOC, GPIO_PIN_13);
        for (volatile uint32_t i=0;i<30000;i++);
    }
}

/* ---- main ---- */
int main(void)
{
    GPIO_EnableClock(GPIOC);
    GPIO_InitPin(GPIOC, GPIO_PIN_13, GPIO_MODE_OUTPUT_PP);

    /* 1) ADC first */
    POT_Init(0);
    LM35_Init(1);
    Analog_Start();

    /* 2) I2C + OLED */
    I2C_Init();
    OLED_Init();
    OLED_PrintAt(0, 0, "Starting...");

    /* 3) Button (EXTI) — PB5, toggles g_running on press */
    Button_Init(GPIOB, GPIO_PIN_5, Button_OnPress);

    /* 4) mutex */
    g_mutex = xSemaphoreCreateMutex();
    if (g_mutex == NULL) { for(;;); }

    /* 5) tasks */
    BaseType_t ok = pdPASS;
    ok &= xTaskCreate(Task_Blink,    "Blink",    128, NULL, 1, NULL);
    ok &= xTaskCreate(Task_ADC,      "ADC",      256, NULL, 3, NULL);
    ok &= xTaskCreate(Task_Display,  "Display",  256, NULL, 2, NULL);
    ok &= xTaskCreate(Task_LEDs,     "LEDs",     128, NULL, 2, NULL);
    ok &= xTaskCreate(Task_Motor,    "Motor",    128, NULL, 2, NULL);
    ok &= xTaskCreate(Task_Watchdog, "Watchdog", 128, NULL, 1, NULL);

    if (ok != pdPASS) {
        OLED_PrintAt(1, 0, "Task create FAIL");
        for(;;);
    }

    /* 6) start the IWDG LAST, just before the scheduler.
     *    NOTE: once started it CANNOT be stopped — only a reset
     *    clears it. Timeout ~2.0s (PR=4, RLR=1250).
     */
    IWDG_Init(IWDG_PRESCALER, IWDG_RELOAD);

    vTaskStartScheduler();
    for (;;);
}
