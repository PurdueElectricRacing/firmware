#include "stm32l432xx.h"
#include "common/phal_L4/can/can.h"
#include "node_defs.h"


int main (void)
{
    
    
    PHAL_initCAN(CAN1, false);
    initCANFilter();

    ApplicationID_t app = COMPILED_APP_ID;

    if (app > APP_INVALID)
    {
        asm("bkpt");
    }

    while (1)
    {
        asm("nop");
    }

    return 0;
}