#ifndef _CHARGE_H_
#define _CHARGE_H_

// Includes
#ifdef STM32L496xx
#include "stm32l496xx.h"
#elif STM32L432xx
#include "stm32l432xx.h"
#else
#error "Please define a STM32 arch"
#endif

#include "common_defs.h"
#include "bms.h"

#endif