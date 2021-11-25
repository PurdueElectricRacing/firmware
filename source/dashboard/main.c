#include "stm32l432xx.h"
#include "common/phal_L4/can/can.h"
#include "apps.h"



int main (void)
{
    PHAL_initCAN(CAN1, false);

    return 0;
}