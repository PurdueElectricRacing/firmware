/**
 * @file usb.h
 * @brief Architecture-selecting USB HAL include.
 * @author Ronak Jain (jain717@purdue.edu)
 */

#ifndef _PHAL_COMMON_USB_H
#define _PHAL_COMMON_USB_H

#include "common/phal/phal_mcu.h"

#if defined(PHAL_ARCH_G4)
#define PHAL_USB_HEADER "common/phal_G4/usb/usb.h"
#else
#error "Unsupported PHAL architecture. Please define a known STM32xx macro."
#endif

#include PHAL_USB_HEADER

#endif // _PHAL_COMMON_USB_H
