#include "runtime_stats.h"

void configureTimer(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk; 
}

uint32_t getCounterValue(void)
{
    return DWT->CYCCNT;
}


