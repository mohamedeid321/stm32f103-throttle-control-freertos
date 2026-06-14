/**
 ******************************************************************************
 * @file    FreeRTOSConfig.h
 * @brief   FreeRTOS configuration for STM32F103C6 (Blue Pill)
 *          Register-level project (no HAL/CMSIS).
 *
 *   Clock: 8 MHz (HSI default — no PLL configured).
 *   If you later run at 72 MHz, change configCPU_CLOCK_HZ accordingly.
 ******************************************************************************
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* ============================================================
 *   Core
 * ============================================================ */
#define configUSE_PREEMPTION                    1
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configCPU_CLOCK_HZ                      ( 8000000UL )
#define configTICK_RATE_HZ                      ( 1000 )      /* 1 ms tick  */
#define configMAX_PRIORITIES                    5
#define configMINIMAL_STACK_SIZE                128           /* words      */
#define configTOTAL_HEAP_SIZE                   ( 4 * 1024 )  /* 4 KB heap  */
#define configMAX_TASK_NAME_LEN                 12
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1

/* ============================================================
 *   Features
 * ============================================================ */
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             0
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_QUEUE_SETS                    0
#define configUSE_TASK_NOTIFICATIONS            1
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               ( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH                5
#define configTIMER_TASK_STACK_DEPTH            128

/* ============================================================
 *   Cortex-M3: interrupt priorities
 *   MUST be set for FreeRTOS to handle SVC / SysTick / PendSV.
 *   All FreeRTOS-managed interrupts must have priority >=
 *   configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY.
 * ============================================================ */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5

#define configKERNEL_INTERRUPT_PRIORITY         \
    ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << 4 )

#define configMAX_SYSCALL_INTERRUPT_PRIORITY    \
    ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << 4 )

/* ============================================================
 *   Debug / runtime checks (keep off for production)
 * ============================================================ */
#define configCHECK_FOR_STACK_OVERFLOW          2
#define configUSE_MALLOC_FAILED_HOOK            1
#define configASSERT( x ) if( ( x ) == 0 ) { __asm("BKPT"); }

/* ============================================================
 *   API inclusions
 * ============================================================ */
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xTaskGetCurrentTaskHandle       1

/* ============================================================
 *   Map FreeRTOS interrupt handlers to CMSIS-style names
 *   (the startup file uses these exact names)
 * ============================================================ */
#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
