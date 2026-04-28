/**
 * @file watchdog.c
 * @brief Basic watchdog implementation
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "watchdog.h"

#if defined(STM32F407xx)
#include "stm32f4xx.h"
#elif defined(STM32G474xx)
#include "stm32g4xx.h"
#else
#error "unsupported watchdog arch"
#endif

void WDG_init() {
    IWDG->KR = 0x5555;
    IWDG->PR = (IWDG_PR_PR_1 | IWDG_PR_PR_0);
    IWDG->RLR = WATCHDOG_TIMEOUT_MS;

    // pause the watchdog during debugging
#if defined(STM32F407xx)
    DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_IWDG_STOP;
#elif defined(STM32G474xx)
    DBGMCU->APB1FZR1 |= DBGMCU_APB1FZR1_DBG_IWDG_STOP;
#endif

    IWDG->KR = 0xAAAA; // reset
    IWDG->KR = 0xCCCC; // start
}

void WDG_pet(void) {
    IWDG->KR = 0xAAAA;
}