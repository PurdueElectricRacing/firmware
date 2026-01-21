#ifndef __DAQ_CAN_H__
#define __DAQ_CAN_H__

#include <stdint.h>

#include "common/phal/can.h"
#include "common/can_library/can_common.h"

// 0 = CAN1, 1 = CAN2,
#define BUS_ID_CAN1 0
#define BUS_ID_CAN2 1

#define CAN_BUS_COUNT      2

extern can_stats_t can_stats;

void initCANParse();
void canTxSendToBack(CanMsgTypeDef_t* msg);
void CAN_tx_update(void);

#endif // __DAQ_CAN_H__