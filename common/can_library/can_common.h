#include <stdint.h>

typedef struct {
    uint8_t bus;
    bool is_extended_id;
    uint32_t unmasked_id;
    uint64_t data_BE;
} can_msg_t;

typedef struct {
  uint32_t tx_of;      // queue overflow
  uint32_t tx_fail;    // timed out
  uint32_t rx_overrun; // fifo overrun
} can_peripheral_stats_t;

#define CAN_TX_MAILBOX_CNT (3)
#define CAN_TX_TIMEOUT_MS (15)
#define CAN_TX_BLOCK_TIMEOUT       (30 * 16000)

#define CAN1_IDX 0
#define CAN2_IDX 1

typedef struct {
    uint32_t rx_of;      // queue overflow
    can_peripheral_stats_t can_peripheral_stats[NUM_CAN_PERIPHERALS];
} can_stats_t;

void CAN_enqueue_tx();
void CAN_tx_update();
bool CAN_library_init();