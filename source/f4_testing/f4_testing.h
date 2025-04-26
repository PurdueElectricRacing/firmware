#ifndef __F4_TESTING__
#define __F4_TESTING__

#include "common/bootloader/bootloader_common.h"
#include "common/common_defs/common_defs.h"
#include "common/psched/psched.h"
#include "common/phal_F4_F7/usart/usart.h"
#include "common/phal_F4_F7/gpio/gpio.h"
#include "common/phal_F4_F7/can/can.h"
#include "common/phal_F4_F7/rcc/rcc.h"
#include "common/phal_F4_F7/adc/adc.h"
#include "common/phal_F4_F7/spi/spi.h"
#include "common/phal_F4_F7/dma/dma.h"
#include "common/faults/faults.h"

// To add new tests create a separate file (see led_blink.c) and it to the enum here
#define TEST_LED_BLINK     0
#define TEST_FREERTOS_DEMO 1
#define BRAKE_TEMPS 2

// Change this define to set the test compiled
#define F4_TESTING_CHOSEN BRAKE_TEMPS


#endif // __F4_TESTING__
