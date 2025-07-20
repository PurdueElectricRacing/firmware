#ifndef _PHAL_F4_H
#define _PHAL_F4_H

#include <stdbool.h>
#include <stdint.h>

#if defined(STM32F407xx)
#include "stm32f407xx.h"
#include "stm32f4xx.h"
#elif defined(STM32F732xx)
#include "stm32f732xx.h"
#include "stm32f7xx.h"
#else
#error "PHAL_ARCH_F4_F7 defined, but no supported STM32F4/F7 device macro found"
#endif

#endif // _PHAL_F4_H
