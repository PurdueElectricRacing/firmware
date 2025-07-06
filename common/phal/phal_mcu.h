/**
 * @file phal_mcu.h
 * @brief MCU architecture selector for PHAL layer (auto-defines PHAL_ARCH_*)
 */

#ifndef _PHAL_COMMON_MCU_H
#define _PHAL_COMMON_MCU_H

#include <stdbool.h>
#include <stdint.h>

// Architecture autodetect

#if defined(STM32L432xx) || defined(STM32L471xx) || defined(STM32L496xx)
  #define PHAL_ARCH_L4
#elif defined(STM32F407xx) || defined(STM32F732xx)
  #define PHAL_ARCH_F4_F7
#elif defined(STM32G474xx)
  #define PHAL_ARCH_G4
#else
  #error "Unable to infer PHAL_ARCH from STM32 device macro. Please define one manually if needed."
#endif

#endif // _PHAL_COMMON_MCU_H
