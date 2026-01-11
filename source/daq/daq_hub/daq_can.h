#ifndef __DAQ_CAN_H__
#define __DAQ_CAN_H__

#include <stdint.h>
#include "DAQ.h"

// 0 = CAN1, 1 = CAN2,
#define BUS_ID_CAN1 0
#define BUS_ID_CAN2 1

#define CAN_BUS_COUNT      2

void initCANParse();
#endif // __DAQ_CAN_H__
