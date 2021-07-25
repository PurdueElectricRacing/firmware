#include "apps.h"


int main (void)
{
    int i;

    while(i++ <= 100)
    {
        asm("nop");
    }
    asm("nop");
}