/**
 ******************************************************************************
 * @file    stm32f103c6_iwdg.c
 * @brief   Independent Watchdog (IWDG) Driver implementation
 ******************************************************************************
 */

#include "stm32f103c6_iwdg.h"

void IWDG_Init(uint8_t prescaler, uint16_t reload)
{
    /* 1) unlock PR and RLR for writing */
    IWDG->KR = IWDG_KEY_UNLOCK;

    /* 2) set prescaler (only bits 2:0 are used, 0..6 valid) */
    IWDG->PR = (uint32_t)(prescaler & 0x07U);

    /* wait until the prescaler write has taken effect */
    while (IWDG->SR & IWDG_SR_PVU);

    /* 3) set reload value (12-bit) */
    IWDG->RLR = (uint32_t)(reload & 0x0FFFU);

    /* wait until the reload write has taken effect */
    while (IWDG->SR & IWDG_SR_RVU);

    /* 4) reload the counter with the new value now */
    IWDG->KR = IWDG_KEY_RELOAD;

    /* 5) start the watchdog — CANNOT be stopped after this */
    IWDG->KR = IWDG_KEY_ENABLE;
}

void IWDG_Refresh(void)
{
    IWDG->KR = IWDG_KEY_RELOAD;
}
