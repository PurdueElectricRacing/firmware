#ifndef CAN_COMMON_H
#define CAN_COMMON_H

#include <stdint.h>
#include "common/phal/can.h"
#include "common/can_library/generated/can_types.h"
#include "common/queue/queue.h"

typedef struct {
  uint32_t tx_of;      // queue overflow
  uint32_t tx_fail;    // timed out
  uint32_t rx_overrun; // fifo overrun
} can_peripheral_stats_t;

#define CAN_TX_MAILBOX_CNT (3)
#define CAN_TX_TIMEOUT_MS (15)
#define CAN_TX_BLOCK_TIMEOUT (30 * 16000)

#define CAN_MAILBOX_HIGH_PRIO 0
#define CAN_MAILBOX_MED_PRIO  1
#define CAN_MAILBOX_LOW_PRIO  2

#ifndef CAN1_IDX
#define CAN1_IDX 0
#endif
#ifndef CAN2_IDX
#define CAN2_IDX 1
#endif

#define GET_PERIPH_IDX(bus) ((bus == CAN1) ? CAN1_IDX : CAN2_IDX)

typedef struct {
    uint32_t rx_of;      // queue overflow
    can_peripheral_stats_t can_peripheral_stats[2];
} can_stats_t;

extern can_stats_t can_stats;
extern q_handle_t q_tx_can[][CAN_TX_MAILBOX_CNT];

void CAN_enqueue_tx(CanMsgTypeDef_t *msg);

#include "common/can_library/generated/can_router.h"

void CAN_tx_update();
void CAN_rx_update();
void CAN_handle_irq(CAN_TypeDef *bus, uint8_t fifo);
bool CAN_library_init();

#endif
