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

// FreeRTOS
#define CAN_TX_QUEUE_LENGTH (64)     // Length of software queue for each CAN peripheral
#define CAN_RX_QUEUE_LENGTH (64)     // Length of software queue for received messages

#if defined(STM32G474xx)
typedef enum : uint8_t {
    CAN_PERIPHERAL1 = 0,
    CAN_PERIPHERAL2 = 1,
    CAN_PERIPHERAL3 = 2,
    CAN_NUM_PERIPHERALS = 3,
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
    CAN_PERIPHERAL1 = 0,
    CAN_PERIPHERAL2 = 1,
    CAN_NUM_PERIPHERALS = 2,
    CAN_PERIPHERAL_INVALID = 0xFF
} CAN_peripheral_t;

static inline CAN_peripheral_t BUS_TO_PERIPHERAL(CAN_TypeDef *bus) {
    if (bus == CAN1) return CAN_PERIPHERAL1;
    else if (bus == CAN2) return CAN_PERIPHERAL2;
    else return CAN_PERIPHERAL_INVALID;
}
#endif

typedef struct {
    uint32_t tx_of;      // queue overflow
    uint32_t tx_fail;    // timed outs
    uint32_t rx_overrun; // fifo overrun
} can_peripheral_stats_t;

typedef struct {
    uint32_t rx_of; // queue overflow
    can_peripheral_stats_t can_peripheral_stats[CAN_NUM_PERIPHERALS];
} can_stats_t;

extern can_stats_t can_stats;
extern volatile uint32_t last_can_rx_time_ms;

extern QueueHandle_t q_rx_can;
extern QueueHandle_t q_tx_can[CAN_NUM_PERIPHERALS];

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
static_assert(
    configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY < NVIC_RX_IRQ_PRIO,
    "Do not set an IRQ priority higher (numerically lower) than FreeRTOS to prevent corruption of kernel data structures"
);
static_assert(
    NVIC_RX_IRQ_PRIO < NVIC_TX_IRQ_PRIO,
    "RX priority should be higher (numerically lower) than TX"
);

#endif // CAN_COMMON_H
