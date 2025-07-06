#ifndef _PHAL_G4_H
#define _PHAL_G4_H

#include <stdbool.h>
#include <stdint.h>

#if defined(STM32G474xx)
    #include "stm32g4xx.h"
    #include "stm32g474xx.h"
#else
    #error "PHAL_ARCH_G4 defined, but no supported STM32G4 device macro found"
#endif

#endif // _PHAL_G4_H
