#ifndef PEDALS_H_
#define PEDALS_H_

typedef struct
{
    float t1;
    float t2;
    float b1;
    float b2;
} pedals_positions_S;



void pedals_Init();

void pedals_DoADC();

void pedals_GetPositions(pedals_positions_S* positions);

#endif