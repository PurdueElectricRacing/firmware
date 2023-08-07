#include "common/phal_F4/rcc/rcc.h"
#include "common/phal_F4/gpio/gpio.h"
#include "common/psched/psched.h"

GPIOInitConfig_t gpio_config[] = {
  GPIO_INIT_OUTPUT(GPIOD, 12, GPIO_OUTPUT_LOW_SPEED),
};

#define TargetCoreClockrateHz 16000000
ClockRateConfig_t clock_config = {
    .system_source              =SYSTEM_CLOCK_SRC_HSI,
    .pll_src                    =PLL_SRC_MSI,
    .msi_output_rate_target_hz  =16000000,
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

void HardFault_Handler();
void ledBlink();

int main()
{
    if(0 != PHAL_configureClockRates(&clock_config))
    {
        HardFault_Handler();
    }
    if(!PHAL_initGPIO(gpio_config, sizeof(gpio_config)/sizeof(GPIOInitConfig_t)))
    {
        HardFault_Handler();
    }
        /* Task Creation */
    schedInit(APB1ClockRateHz);
    // taskCreate(usartTXTest, 1000);
    taskCreate(ledBlink, 500);
    //Fault Stuff
    // taskCreate(adcConvert, 50);
    // taskCreate(daqPeriodic, DAQ_UPDATE_PERIOD);
    // taskCreate(canSendTest, 50);
    // taskCreate(memFg, MEM_FG_TIME);
    // taskCreate(wheelSpeedsPeriodic, 15);
    // taskCreate(myCounterTest, 50);
    // taskCreateBackground(canTxUpdate);
    // taskCreateBackground(canRxUpdate);
    // taskCreateBackground(memBg);


    // signify end of initialization
    // PHAL_writeGPIO(LED_GREEN_GPIO_Port, LED_GREEN_Pin, 0);
    schedStart();
    return 0;
}

void ledBlink()
{
    static uint8_t status;
    if (status)
    {
        PHAL_writeGPIO(GPIOD, 12, 0);
        status = 0;
    }
    else
    {
        PHAL_writeGPIO(GPIOD, 12, 1);
        status = 1;
    }
}
void HardFault_Handler()
{
    while(1)
    {
        __asm__("nop");
    }
}