#include "stm32l432xx.h"

#include "apps.h"

int main (void)
{
    int i;
    apps_Init();

    while(i++ <= 100)
    {
        asm("nop");
        apps_Tick(1.0, 1.0);
    }

    // Test out link to CMSIS
    RCC->AHB2RSTR &= !RCC_AHB2RSTR_GPIOARST;

    return 0;
}