/**
 * @file can_common.c
 * @brief Common functions and data structures used in every node in the CAN library
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 * @author Luke Oxley (lcoxley@purdue.edu)
 */

#include "common/can_library/can_common.h"

#include "common/can_library/generated/can_router.h"
#include "common/queue/queue.h"

// common data structures
can_data_t can_data;
can_stats_t can_stats;
volatile uint32_t last_can_rx_time_ms;

#ifndef CAN_QUEUE_SIZE
#define CAN_QUEUE_SIZE 24
#endif

#if defined(STM32F407xx) || defined(STM32F732xx)

// todo mailbox based implementation here
queue_t q_tx_can[NUM_CAN_PERIPHERALS][CAN_TX_MAILBOX_CNT];
QUEUE_INIT(q_rx_can, sizeof(CanMsgTypeDef_t), CAN_QUEUE_SIZE);
uint32_t can_mbx_last_send_time[NUM_CAN_PERIPHERALS][CAN_TX_MAILBOX_CNT];

// Statically allocate TX queues for CAN1 and CAN2
#ifdef USE_CAN1
QUEUE_INIT(q_tx_can1_m0, sizeof(CanMsgTypeDef_t), CAN_QUEUE_SIZE);
QUEUE_INIT(q_tx_can1_m1, sizeof(CanMsgTypeDef_t), CAN_QUEUE_SIZE);
QUEUE_INIT(q_tx_can1_m2, sizeof(CanMsgTypeDef_t), CAN_QUEUE_SIZE);
#endif

#ifdef USE_CAN2
QUEUE_INIT(q_tx_can2_m0, sizeof(CanMsgTypeDef_t), CAN_QUEUE_SIZE);
QUEUE_INIT(q_tx_can2_m1, sizeof(CanMsgTypeDef_t), CAN_QUEUE_SIZE);
QUEUE_INIT(q_tx_can2_m2, sizeof(CanMsgTypeDef_t), CAN_QUEUE_SIZE);
#endif

void CAN_enqueue_tx(CanMsgTypeDef_t *msg) {
    uint8_t mailbox;
    uint8_t periph_idx = GET_PERIPH_IDX(msg->Bus);

    if (msg->IDE != 0) {
        // Extended ID: Use HLP bits (26-28) to determine priority mailbox
        switch ((msg->ExtId >> 26) & 0b111) {
            case 0:
            case 1:
                mailbox = CAN_MAILBOX_HIGH_PRIO;
                break;
            case 2:
            case 3:
                mailbox = CAN_MAILBOX_MED_PRIO;
                break;
            default:
                mailbox = CAN_MAILBOX_LOW_PRIO;
                break;
        }
    } else {
        // Standard ID: Default to high priority
        mailbox = CAN_MAILBOX_HIGH_PRIO;
    }

    if (queue_push(&q_tx_can[periph_idx][mailbox], msg) != QUEUE_SUCCESS) {
        can_stats.can_peripheral_stats[periph_idx].tx_of++;
    }
}

void CAN_tx_update() {
    CanMsgTypeDef_t tx_msg;
    for (uint8_t i = 0; i < CAN_TX_MAILBOX_CNT; ++i) {
#ifdef USE_CAN1
        // Handle CAN1
        if (PHAL_txMailboxFree(CAN1, i)) {
            if (queue_pop(&q_tx_can[CAN1_IDX][i], &tx_msg) == QUEUE_SUCCESS) {
                PHAL_txCANMessage(&tx_msg, i);
                can_mbx_last_send_time[CAN1_IDX][i] = OS_TICKS;
            }
        } else if (OS_TICKS - can_mbx_last_send_time[CAN1_IDX][i] > CAN_TX_TIMEOUT_MS) {
            PHAL_txCANAbort(CAN1, i);
            can_stats.can_peripheral_stats[CAN1_IDX].tx_fail++;
        }
#endif

#ifdef USE_CAN2
        // Handle CAN2
        if (PHAL_txMailboxFree(CAN2, i)) {
            if (queue_pop(&q_tx_can[CAN2_IDX][i], &tx_msg) == QUEUE_SUCCESS) {
                PHAL_txCANMessage(&tx_msg, i);
                can_mbx_last_send_time[CAN2_IDX][i] = OS_TICKS;
            }
        } else if (OS_TICKS - can_mbx_last_send_time[CAN2_IDX][i] > CAN_TX_TIMEOUT_MS) {
            PHAL_txCANAbort(CAN2, i);
            can_stats.can_peripheral_stats[CAN2_IDX].tx_fail++;
        }
#endif
    }
}

void CAN_handle_irq(CAN_TypeDef *bus, uint8_t fifo) {
    CanMsgTypeDef_t rx_msg;
    while (PHAL_rxCANMessage(bus, fifo, &rx_msg)) {
        if (queue_push(&q_rx_can, &rx_msg) != QUEUE_SUCCESS) {
            can_stats.rx_of++;
        }
    }
}

void CAN_rx_update() {
    CanMsgTypeDef_t rx_msg;
    while (queue_pop(&q_rx_can, &rx_msg) == QUEUE_SUCCESS) {
        last_can_rx_time_ms = OS_TICKS;
        uint8_t periph_idx  = GET_PERIPH_IDX(rx_msg.Bus);
        CAN_rx_dispatcher(rx_msg.IDE == 0 ? rx_msg.StdId : rx_msg.ExtId,
                          rx_msg.Data,
                          rx_msg.DLC,
                          periph_idx);
    }
}

// ! Define these in your main.c for now !
#ifdef USE_CAN1
void __attribute__((weak, used)) CAN1_RX0_IRQHandler() {
    CAN_handle_irq(CAN1, 0);
}

void __attribute__((weak, used)) CAN1_RX1_IRQHandler() {
    CAN_handle_irq(CAN1, 1);
}
#endif

#ifdef USE_CAN2
void __attribute__((weak, used)) CAN2_RX0_IRQHandler() {
    CAN_handle_irq(CAN2, 0);
}

void __attribute__((weak, used)) CAN2_RX1_IRQHandler() {
    CAN_handle_irq(CAN2, 1);
}
#endif

[[gnu::always_inline]]
static inline bool CAN_prepare_filter_config(CAN_TypeDef *can) {
    can->MCR |= CAN_MCR_INRQ; // Enter back into INIT state (required for changing scale)
    uint32_t timeout = 0;
    while (!(can->MSR & CAN_MSR_INAK) && ++timeout < PHAL_CAN_INIT_TIMEOUT)
        ;
    if (timeout == PHAL_CAN_INIT_TIMEOUT)
        return false;

    can->FMR |= CAN_FMR_FINIT; // Enter init mode for filter banks

    return true;
}

[[gnu::always_inline]]
static inline bool CAN_exit_filter_config(CAN_TypeDef *can) {
    can->FMR &= ~CAN_FMR_FINIT; // Enable Filters (exit filter init mode)

    // Enter back into NORMAL mode
    can->MCR &= ~CAN_MCR_INRQ;
    uint32_t timeout = 0;
    while ((can->MSR & CAN_MSR_INAK) && ++timeout < PHAL_CAN_INIT_TIMEOUT)
        ;

    return timeout != PHAL_CAN_INIT_TIMEOUT;
}

bool CAN_library_init() {
#ifdef USE_CAN1
    q_tx_can[CAN1_IDX][0] = q_tx_can1_m0;
    q_tx_can[CAN1_IDX][1] = q_tx_can1_m1;
    q_tx_can[CAN1_IDX][2] = q_tx_can1_m2;
    can_mbx_last_send_time[CAN1_IDX][0] = 0;
    can_mbx_last_send_time[CAN1_IDX][1] = 0;
    can_mbx_last_send_time[CAN1_IDX][2] = 0;
#endif

#ifdef USE_CAN2
    q_tx_can[CAN2_IDX][0] = q_tx_can2_m0;
    q_tx_can[CAN2_IDX][1] = q_tx_can2_m1;
    q_tx_can[CAN2_IDX][2] = q_tx_can2_m2;
    can_mbx_last_send_time[CAN2_IDX][0] = 0;
    can_mbx_last_send_time[CAN2_IDX][1] = 0;
    can_mbx_last_send_time[CAN2_IDX][2] = 0;
#endif

    can_stats = (can_stats_t) {0};
    CAN_data_init();

#if defined(USE_CAN1) || defined(USE_CAN2)
    // Ensure CAN1 clock is enabled (CAN1 owns all bxCAN filters)
    RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;
    if (!CAN_prepare_filter_config(CAN1)) {
        return false;
    }

    bxcan_set_filters(); // from NODE.h

    if (!CAN_exit_filter_config(CAN1)) {
        return false;
    }
#endif

    return true;
}

#elif defined(STM32G474xx)

// G4/FDCAN implementation - uses TX FIFO (no mailboxes)
// FDCAN has 3 TX FIFO slots handled by hardware, so we use a single software queue per peripheral

#ifndef NUM_CAN_PERIPHERALS
#if defined(USE_FDCAN3)
#define NUM_CAN_PERIPHERALS 3
#elif defined(USE_FDCAN2)
#define NUM_CAN_PERIPHERALS 2
#elif defined(USE_FDCAN1)
#define NUM_CAN_PERIPHERALS 1
#else
#define NUM_CAN_PERIPHERALS 1
#endif
#endif

queue_t q_tx_can[NUM_CAN_PERIPHERALS];
QUEUE_INIT(q_rx_can, sizeof(CanMsgTypeDef_t), CAN_QUEUE_SIZE);

// Statically allocate TX queues for FDCAN1, FDCAN2, and FDCAN3
#ifdef USE_FDCAN1
QUEUE_INIT(q_tx_can1, sizeof(CanMsgTypeDef_t), CAN_QUEUE_SIZE);
#endif

#ifdef USE_FDCAN2
QUEUE_INIT(q_tx_can2, sizeof(CanMsgTypeDef_t), CAN_QUEUE_SIZE);
#endif

#ifdef USE_FDCAN3
QUEUE_INIT(q_tx_can3, sizeof(CanMsgTypeDef_t), CAN_QUEUE_SIZE);
#endif

void CAN_enqueue_tx(CanMsgTypeDef_t *msg) {
    uint8_t periph_idx = GET_PERIPH_IDX(msg->Bus);

    if (queue_push(&q_tx_can[periph_idx], msg) != QUEUE_SUCCESS) {
        can_stats.can_peripheral_stats[periph_idx].tx_of++;
    }
}

void CAN_tx_update() {
    CanMsgTypeDef_t tx_msg;

#ifdef USE_FDCAN1
    while (PHAL_FDCAN_txFifoFree(FDCAN1) && queue_pop(&q_tx_can[CAN1_IDX], &tx_msg) == QUEUE_SUCCESS) {
        PHAL_FDCAN_send(&tx_msg);
    }
#endif

#ifdef USE_FDCAN2
    while (PHAL_FDCAN_txFifoFree(FDCAN2) && queue_pop(&q_tx_can[CAN2_IDX], &tx_msg) == QUEUE_SUCCESS) {
        PHAL_FDCAN_send(&tx_msg);
    }
#endif

#ifdef USE_FDCAN3
    while (PHAL_FDCAN_txFifoFree(FDCAN3) && queue_pop(&q_tx_can[CAN3_IDX], &tx_msg) == QUEUE_SUCCESS) {
        PHAL_FDCAN_send(&tx_msg);
    }
#endif
}

void CAN_rx_update() {
    CanMsgTypeDef_t rx_msg;
    while (queue_pop(&q_rx_can, &rx_msg) == QUEUE_SUCCESS) {
        last_can_rx_time_ms = OS_TICKS;
        uint8_t periph_idx  = GET_PERIPH_IDX(rx_msg.Bus);
        CAN_rx_dispatcher(rx_msg.IDE == 0 ? rx_msg.StdId : rx_msg.ExtId,
                          rx_msg.Data,
                          rx_msg.DLC,
                          periph_idx);
    }
}

// FDCAN RX callback - enqueues received messages to the RX queue
// This overrides the weak definition in fdcan.c
void PHAL_FDCAN_rxCallback(CanMsgTypeDef_t *msg) {
    if (queue_push(&q_rx_can, msg) != QUEUE_SUCCESS) {
        can_stats.rx_of++;
    }
}

bool CAN_library_init() {
    // Initialize TX queues (one per peripheral)
#ifdef USE_FDCAN1
    q_tx_can[CAN1_IDX] = q_tx_can1;
#endif

#ifdef USE_FDCAN2
    q_tx_can[CAN2_IDX] = q_tx_can2;
#endif

#ifdef USE_FDCAN3
    q_tx_can[CAN3_IDX] = q_tx_can3;
#endif

    // Clear stats
    can_stats = (can_stats_t) {0};

    // Initialize CAN data from generated code
    CAN_data_init();

#ifdef USE_FDCAN1
    FDCAN1_set_filters();
#endif

#ifdef USE_FDCAN2
    FDCAN2_set_filters();
#endif

#ifdef USE_FDCAN3
    FDCAN3_set_filters();
#endif

    return true;
}

#else
#error "Unsupported architecture"
#endif
