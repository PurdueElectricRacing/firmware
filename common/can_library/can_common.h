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
#define CAN_TX_QUEUE_LENGTH (64)     // Length of software queue for each CAN peripheral
#define CAN_RX_QUEUE_LENGTH (64)     // Length of software queue for received messages

#define CAN_TX_MAILBOX_CNT   (3)
#define CAN_TX_TIMEOUT_MS    (15)
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

extern QueueHandle_t q_rx_can;
extern QueueHandle_t q_tx_can[];

void CAN_enqueue_tx(CanMsgTypeDef_t *msg);

#include "common/can_library/generated/can_router.h"

void CAN_tx_update();
void CAN_rx_update();
bool CAN_init();
bool CAN_enable_IRQs();

#define DEFINE_CAN_TASKS() \
    DEFINE_TASK(CAN_rx_update, 0, osPriorityHigh, STACK_2048); \
    DEFINE_TASK(CAN_tx_update, 0, osPriorityHigh, STACK_2048);

#define START_CAN_TASKS() \
    START_TASK(CAN_rx_update); \
    START_TASK(CAN_tx_update); \
    CAN_enable_IRQs();

#define NVIC_RX_IRQ_PRIO (6)
#define NVIC_TX_IRQ_PRIO (7)

#endif // CAN_COMMON_H
