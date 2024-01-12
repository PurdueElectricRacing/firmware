/* System Includes */
#include "stm32l432xx.h"
#include "common/bootloader/bootloader_common.h"
#include "common/psched/psched.h"
#include "common/phal_L4/can/can.h"
#include "common/phal_L4/rcc/rcc.h"
#include "common/phal_L4/gpio/gpio.h"
#include "common/phal_L4/adc/adc.h"
#include "common/phal_L4/usart/usart.h"
#include "common/phal_L4/tim/tim.h"
#include "common/phal_L4/dma/dma.h"
#include "common/phal_L4/eeprom_spi/eeprom_spi.h"
#include <math.h>

/* Module Includes */
#include "main.h"
#include "can_parse.h"
#include "daq.h"
#include "common/modules/wheel_speeds/wheel_speeds.h"

#include "common/faults/faults.h"


GPIOInitConfig_t gpio_config[] = {
//TODO: LED port
//TODO: Button port
//TODO: CAN Ports
};

#define TargetCoreClockrateHz 80000000
ClockRateConfig_t clock_config = {
    .system_source              =SYSTEM_CLOCK_SRC_PLL,
    .pll_src                    =PLL_SRC_MSI,
    .msi_output_rate_target_hz  =16000000,
    .vco_output_rate_target_hz  =160000000,
    .system_clock_target_hz     =TargetCoreClockrateHz,
    .ahb_clock_target_hz        =(TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz       =(TargetCoreClockrateHz / (4)),
    .apb2_clock_target_hz       =(TargetCoreClockrateHz / (4)),
};

/* Locals for Clock Rates */
extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

//TODO: queue definitions

/* Function Prototypes */
void HardFault_Handler();
//Insert function prototypes here


int main (void)
{
    //TODO: init queues

    /* HAL Initilization */
    if(0 != PHAL_configureClockRates(&clock_config))
    {
        HardFault_Handler();
    }
    if(!PHAL_initGPIO(gpio_config, sizeof(gpio_config)/sizeof(GPIOInitConfig_t)))
    {
        HardFault_Handler();
    }


    //TODO: init CAN

    //TODO: configure button interrupt

    /* Task Creation */
    schedInit(APB1ClockRateHz);
        //TODO: LED task
        //TODO: CAN background tasks

    schedStart();

    return 0;
}

void HardFault_Handler()
{
    while(1)
    {
        __asm__("nop");
    }
}