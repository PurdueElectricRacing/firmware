#include "stm32l432xx.h"
#include "stm32l4xx_hal_can.h"

#include "apps.h"
#include "common/psched/psched.h"
#include "common/phal_L4/gpio/gpio.h"

GPIOConfig_t gpio_confg[] = {
    {.bank=GPIOA, .pin=10, .type=ALT_FUNC, .config={.af_num    = 6         }},
    {.bank=GPIOB, .pin=11, .type=OUTPUT,   .config={.push_pull = PULL_UP   }},
    {.bank=GPIOC, .pin=12, .type=INPUT,    .config={.ospeed    = HIGH_SPEED}},
};

int main (void)
{

    PHAL_initGPIO(gpio_confg, sizeof(gpio_confg)/sizeof(GPIOConfig_t));

    while(1)
    {
        asm("bkpt");
    }

    // Test out link to Common module

    return 0;
}

