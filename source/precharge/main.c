#include "stm32l496xx.h"
#include "common/phal_L4/can/can.h"



int main (void)
{
    PHAL_initCAN(CAN2, false);

    return 0;
}