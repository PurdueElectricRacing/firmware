#include <assert.h>
#include "per_rtos.h"

void rtosRunTasks()
{
    asm("nop");
}


void inline rtosHandleTick()
{
    // Implement this on a per-component basis or something
}