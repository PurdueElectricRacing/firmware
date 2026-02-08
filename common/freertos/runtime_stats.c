#include "runtime_stats.h"

void configure_timer(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t get_counter_value(void)
{
    return DWT->CYCCNT;
}


