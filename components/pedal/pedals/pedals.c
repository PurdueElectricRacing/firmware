#include "pedals.h"
#include <string.h>


static pedals_positions_S pedal_positions;

void pedals_Init()
{
    pedal_positions.t1 = 0;
    pedal_positions.t2 = 0;
    pedal_positions.b1 = 0;
    pedal_positions.b2 = 0;
}

void pedals_DoADC()
{
    pedal_positions.t1 = 0.5;
    pedal_positions.t2 = 0.4;
    pedal_positions.b1 = 0.3;
    pedal_positions.b2 = 0.2;
}

void pedals_GetPositions(pedals_positions_S* positions)
{
    memcpy(&pedal_positions, positions, sizeof(pedals_positions_S));
}