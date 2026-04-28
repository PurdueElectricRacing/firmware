#ifndef CAN_COMMON_H
#define CAN_COMMON_H

/**
 * @file can_common.h
 * @brief Common functions and data structures used in every node in the CAN library
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include <stdint.h>

#include "common/freertos/freertos.h"
#include "common/phal/can.h"

#if defined(STM32G474xx)
typedef enum : uint8_t {
    CAN_PERIPHERAL1        = 0,
    CAN_PERIPHERAL2        = 1,
    CAN_PERIPHERAL3        = 2,
    CAN_NUM_PERIPHERALS    = 3,
    CAN_PERIPHERAL_INVALID = 0xFF
} CAN_peripheral_t;

static inline CAN_peripheral_t BUS_TO_PERIPHERAL(FDCAN_GlobalTypeDef *bus) {
    if (bus == FDCAN1) return CAN_PERIPHERAL1;
    else if (bus == FDCAN2) return CAN_PERIPHERAL2;
    else if (bus == FDCAN3) return CAN_PERIPHERAL3;
    else return CAN_PERIPHERAL_INVALID;
}
#else
typedef enum : uint8_t {
    CAN_PERIPHERAL1        = 0,
    CAN_PERIPHERAL2        = 1,
    CAN_NUM_PERIPHERALS    = 2,
    CAN_PERIPHERAL_INVALID = 0xFF
} CAN_peripheral_t;

static inline CAN_peripheral_t BUS_TO_PERIPHERAL(CAN_TypeDef *bus) {
    if (bus == CAN1) return CAN_PERIPHERAL1;
    else if (bus == CAN2) return CAN_PERIPHERAL2;
    else return CAN_PERIPHERAL_INVALID;
}
#endif

typedef struct {
    uint32_t rx_overflow; // software queue overflow
    uint32_t tx_overflow; // software queue overflow
    uint32_t tx_enqueue_count;
    uint32_t tx_task_wake_count;
    uint32_t tx_sent_count;
    uint32_t tx_callback_count;
    // todo: track hardware stats
    // todo: per-peripheral stats
} can_stats_t;

extern volatile can_stats_t can_stats;
extern volatile uint32_t last_can_rx_time_ms;
// todo: last tx time

extern QueueHandle_t can_rx_queue;
extern QueueHandle_t can_tx_queues[CAN_NUM_PERIPHERALS];

void CAN_enqueue_tx(CanMsgTypeDef_t *msg);
void CAN_tx_update(void);
void CAN_rx_update(void);
bool CAN_init(void);
void CAN_enable_IRQs(void);
void CAN_rx_init(void);
void CAN_tx_init(void);

#define DEFINE_CAN_TASKS() \
    DEFINE_TASK(CAN_rx_update, 0, osPriorityHigh, STACK_2048); \
    DEFINE_TASK(CAN_tx_update, 0, osPriorityHigh, STACK_1024);

#define START_CAN_TASKS() \
    START_TASK(CAN_rx_update); \
    START_TASK(CAN_tx_update); \
    CAN_enable_IRQs();

#define CAN_TX_QUEUE_LENGTH (32) // Length of software queue for each CAN peripheral
#define CAN_RX_QUEUE_LENGTH (32) // Length of software queue for received messages

#define NVIC_RX_IRQ_PRIO (6)
#define NVIC_TX_IRQ_PRIO (7)
static_assert(
    configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY < NVIC_RX_IRQ_PRIO,
    "Do not set an IRQ priority higher (numerically lower) than FreeRTOS to prevent corruption of kernel data structures"
);
static_assert(
    NVIC_RX_IRQ_PRIO < NVIC_TX_IRQ_PRIO,
    "RX priority should be higher (numerically lower) than TX"
);

#endif // CAN_COMMON_H