#include "common/phal_F4_F7/rcc/rcc.h"
#include "common/phal_F4_F7/gpio/gpio.h"
#include "common/phal_F4_F7/adc/adc.h"
#include "common/phal_F4_F7/dma/dma.h"
#include "common/phal_F4_F7/spi/spi.h"
#include "common/psched/psched.h"

#include "main.h"

GPIOInitConfig_t gpio_config[] = {
    //TODO: LED port
    //TODO: Button port
    //TODO: CAN Ports

};

#define TargetCoreClockrateHz 16000000
ClockRateConfig_t clock_config = {
    .system_source              =SYSTEM_CLOCK_SRC_HSI,
    .vco_output_rate_target_hz  =160000000,
    .system_clock_target_hz     =TargetCoreClockrateHz,
    .ahb_clock_target_hz        =(TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz       =(TargetCoreClockrateHz / (1)),
    .apb2_clock_target_hz       =(TargetCoreClockrateHz / (1)),
};

extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;
//TODO: queue definitions


void HardFault_Handler();
//Insert function prototypes here

int main()
{
    //TODO: init queues
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
        /* Schedule Periodic tasks here */
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