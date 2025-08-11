#ifndef _PHAL_COMMON_CAN_H
#define _PHAL_COMMON_CAN_H

#include "common/phal/phal_mcu.h"

#if defined(PHAL_ARCH_L4)
#define PHAL_CAN_HEADER "common/phal_L4/can/can.h"
#elif defined(PHAL_ARCH_F4_F7)
#define PHAL_CAN_HEADER "common/phal_F4_F7/can/can.h"
#elif defined(PHAL_ARCH_G4)
#define PHAL_CAN_HEADER "common/phal_G4/fdcan/fdcan.h"
#else
#error "Unsupported PHAL architecture. Please define a known STM32xx macro."
#endif

#include PHAL_CAN_HEADER

#endif // _PHAL_COMMON_CAN_H
