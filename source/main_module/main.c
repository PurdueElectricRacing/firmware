#include "stm32l432xx.h"
#include "stm32l4xx_hal_can.h"

#include "apps.h"
#include "psched.h"


int main (void)
{
    apps_Init();

    // Test out link to CMSIS
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM6EN;
    TIM6->PSC      = (SystemCoreClock / 1000);
    TIM6->ARR      = 1000;

    while(1)
    {
        asm("bkpt");
    }

    // Test out link to Common module
    
    return 0;
}