#ifndef CAN_COMMON_H
#define CAN_COMMON_H

/**
 * @file can_common.h
 * @brief Common functions and data structures used in every node in the CAN library
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 * @author Ronak Jain (jain717@purdue.edu)
 */

#include <stdint.h>

#include "common/freertos/freertos.h"
#include "common/phal/can.h"

typedef struct {
    uint32_t tx_of;      // queue overflow
    uint32_t tx_fail;    // timed out
    uint32_t rx_overrun; // fifo overrun
} can_peripheral_stats_t;

// FreeRTOS
#define CAN_TX_BACKPRESSURE_MS (2) // Wait up to 2ms if FDCAN TX FIFO is full before dropping message
#define CAN_TX_QUEUE_LENGTH (64)     // Length of software queue for each CAN peripheral
#define CAN_RX_QUEUE_LENGTH (64)     // Length of software queue for received messages

#define CAN_TX_MAILBOX_CNT   (3)
#define CAN_TX_TIMEOUT_MS    (15)
#define CAN_TX_BLOCK_TIMEOUT (30 * 16000)

#define CAN_MAILBOX_HIGH_PRIO 0
#define CAN_MAILBOX_MED_PRIO  1
#define CAN_MAILBOX_LOW_PRIO  2

/* Standard CAN flag and mask definitions */
#define CAN_EFF_FLAG 0x80000000U /* EFF/SFF is set in the MSB */
#define CAN_RTR_FLAG 0x40000000U /* remote transmission request */
#define CAN_ERR_FLAG 0x20000000U /* error message frame */

/* valid bits in CAN ID for frame formats */
#define CAN_SFF_MASK 0x000007FFU /* standard frame format (SFF) */
#define CAN_EFF_MASK 0x1FFFFFFFU /* extended frame format (EFF) */
#define CAN_ERR_MASK 0x1FFFFFFFU /* omit EFF, RTR, ERR flags */

#ifndef CAN1_IDX
#define CAN1_IDX 0
#endif
#ifndef CAN2_IDX
#define CAN2_IDX 1
#endif
#ifndef CAN3_IDX
#define CAN3_IDX 2
#endif

#if defined(STM32G474xx)
// G4 uses FDCAN peripheral
#define GET_PERIPH_IDX(bus) ((bus == FDCAN1) ? CAN1_IDX : ((bus == FDCAN2) ? CAN2_IDX : CAN3_IDX))
#else
// F4/F7/L4 use bxCAN peripheral
#define GET_PERIPH_IDX(bus) ((bus == CAN1) ? CAN1_IDX : CAN2_IDX)
#endif

#if defined(STM32G474xx)
#define NUM_CAN_PERIPHERALS_MAX 3
#else
#define NUM_CAN_PERIPHERALS_MAX 2
#endif

typedef struct {
    uint32_t rx_of; // queue overflow
    can_peripheral_stats_t can_peripheral_stats[NUM_CAN_PERIPHERALS_MAX];
} can_stats_t;

extern can_stats_t can_stats;
extern volatile uint32_t last_can_rx_time_ms;


#if defined(STM32F407xx) 
// bxCAN uses 3 mailboxes per peripheral
extern QueueHandle_t q_tx_can[][CAN_TX_MAILBOX_CNT];
#elif defined(STM32G474xx)
// G4/FDCAN uses a single TX queue per peripheral (no mailboxes)
extern QueueHandle_t q_tx_can[];
#else
#error "Unsupported architecture"
#endif
extern QueueHandle_t q_rx_can;

void CAN_enqueue_tx(CanMsgTypeDef_t *msg);

#include "common/can_library/generated/can_router.h"

void CAN_tx_update();
void CAN_rx_update();
bool CAN_library_init();

#if defined(STM32G474xx)
// G4/FDCAN IRQ handling is done in fdcan.c via PHAL_FDCAN_rxCallback
#else
void CAN_handle_irq(CAN_TypeDef *bus, uint8_t fifo);
#endif

#endif // CAN_COMMON_H
