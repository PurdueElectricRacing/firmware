#include "stm32l432xx.h"
#include "stm32l4xx_hal_can.h"

#include "apps.h"
#include "common/psched/psched.h"
#include "common/phal_L4/gpio/gpio.h"

GPIOInitConfig_t gpio_config[] = {
    GPIO_INIT_CANRX_PA11,
    GPIO_INIT_CANTX_PA12,
    GPIO_INIT_INPUT(GPIOA, 10, GPIO_INPUT_OPEN_DRAIN),
};

int main (void)
{

    PHAL_initGPIO(gpio_config, sizeof(gpio_config)/sizeof(GPIOInitConfig_t));

    while(1)
    {
        asm("bkpt");
    }

    return 0;
}

