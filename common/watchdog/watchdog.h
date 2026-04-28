/**
 * @file watchdog.h
 * @brief Basic watchdog implementation
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <stdint.h>
#include "common/freertos/freertos.h"

void WDG_init(void);
void WDG_pet(void);

static constexpr uint32_t WATCHDOG_TIMEOUT_MS = 1000;
static constexpr uint32_t WATCHDOG_PET_FREQUENCY = 100;

#define DEFINE_WATCHDOG_TASK() DEFINE_TASK(WDG_pet, WATCHDOG_PET_FREQUENCY, osPriorityLow, STACK_256)

#define START_WATCHDOG_TASK() \
    do { \
        WDG_init(); \
        START_TASK(WDG_pet); \
    } while (0);

#endif // WATCHDOG_H