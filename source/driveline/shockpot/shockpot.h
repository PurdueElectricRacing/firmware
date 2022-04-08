#ifndef __SHOCKPOT__
#define __SHOCKPOT__

#include "force.h"
#include <stdint.h>

typedef struct __attribute__((packed))
{
    // Do not modify this struct unless
    // you modify the ADC DMA config
    // in main.h to match
    uint16_t pot_left;
    uint16_t pot_right;
} raw_shock_pots_t;

volatile extern raw_shock_pots_t raw_shock_pots;

void shockpot1000Hz();

#endif