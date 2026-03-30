/**
 * @file can_common.c
 * @brief Common functions and data structures used in every node in the CAN library
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 * @author Ronak Jain (jain717@purdue.edu)
 */

#include "common/can_library/can_common.h"

#include "common/can_library/generated/can_router.h"

// common data structures
volatile can_data_t can_data;
volatile can_stats_t can_stats;
volatile uint32_t last_can_rx_time_ms;

extern osThreadId_t CAN_rx_update_handle;
extern osThreadId_t CAN_tx_update_handle;

QueueHandle_t can_tx_queues[CAN_NUM_PERIPHERALS];
DEFINE_QUEUE(can_rx_queue, CanMsgTypeDef_t, CAN_RX_QUEUE_LENGTH);

// Shared rx update implementation
void CAN_rx_update() {
    CanMsgTypeDef_t rx_msg;

    // Block until a message is received
    if (xQueueReceive(can_rx_queue, &rx_msg, portMAX_DELAY) == pdPASS) {
        last_can_rx_time_ms = OS_TICKS;
        CAN_peripheral_t peripheral = BUS_TO_PERIPHERAL(rx_msg.Bus);
        CAN_rx_dispatcher(
            rx_msg.IDE == 0 ? rx_msg.StdId : rx_msg.ExtId,
            rx_msg.Data,
            rx_msg.DLC,
            peripheral
        );
    }
}

// Shared tx enqueue implementation
void CAN_enqueue_tx(CanMsgTypeDef_t *msg) {
    CAN_peripheral_t peripheral = BUS_TO_PERIPHERAL(msg->Bus);

    // Immediately drop the message and bump overflow counter if the queue is full
    if (xQueueSendToBack(can_tx_queues[peripheral], msg, 0) != pdPASS) {
        can_stats.tx_overflow++;
        return;
    }

    // Wake the TX task to attempt an immediate send
    if (CAN_tx_update_handle != NULL) {
        xTaskNotifyGive(CAN_tx_update_handle);
    }
}

// Shared tx isr implementation
[[gnu::always_inline]]
static inline void CAN_wake_tx_from_ISR() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (CAN_tx_update_handle != NULL) {
        vTaskNotifyGiveFromISR(CAN_tx_update_handle, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

#if defined(STM32F407xx)

#ifdef USE_CAN1
DEFINE_QUEUE(can1_tx_queue, CanMsgTypeDef_t, CAN_TX_QUEUE_LENGTH);
#endif

#ifdef USE_CAN2
DEFINE_QUEUE(can2_tx_queue, CanMsgTypeDef_t, CAN_TX_QUEUE_LENGTH);
#endif

void CAN_tx_update() {
    // Block until a notification is received
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // Feed the peripheral TX FIFOs until all queues are empty or FIFOs are full
    bool is_tx_success;
    do { // Loop to handle the case where a FIFO slot opens during processing
        is_tx_success = false;
        CanMsgTypeDef_t tx_msg;
        uint8_t free_index;
        QueueHandle_t tx_queue;

#ifdef USE_CAN1
        tx_queue = can_tx_queues[BUS_TO_PERIPHERAL(CAN1)];
        while (PHAL_getFreeTxMailbox(CAN1, &free_index) && xQueueReceive(tx_queue, &tx_msg, 0) == pdPASS) {
            PHAL_txCANMessage(&tx_msg, free_index);
            is_tx_success = true;
        }
#endif

#ifdef USE_CAN2
        tx_queue = can_tx_queues[BUS_TO_PERIPHERAL(CAN2)];
        while (PHAL_getFreeTxMailbox(CAN2, &free_index) && xQueueReceive(tx_queue, &tx_msg, 0) == pdPASS) {
            PHAL_txCANMessage(&tx_msg, free_index);
            is_tx_success = true;
        }
#endif
    } while (is_tx_success);
}

[[gnu::always_inline]]
inline void CAN_rx_ISR(CAN_TypeDef *bus, uint8_t fifo) {
    CanMsgTypeDef_t rx_msg;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Drain the hardware FIFO
    while (PHAL_rxCANMessage(bus, fifo, &rx_msg)) {
        if (xQueueSendFromISR(can_rx_queue, &rx_msg, &xHigherPriorityTaskWoken) != pdPASS) {
            can_stats.rx_overflow++;
        }
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// ! Define these in your main.c for now !
#ifdef USE_CAN1
void __attribute__((weak, used)) CAN1_RX0_IRQHandler() {
    CAN_rx_ISR(CAN1, 0);
}

void __attribute__((weak, used)) CAN1_RX1_IRQHandler() {
    CAN_rx_ISR(CAN1, 1);
}

void CAN1_TX_IRQHandler() {
    CAN_wake_tx_from_ISR();
}
#endif

#ifdef USE_CAN2
void __attribute__((weak, used)) CAN2_RX0_IRQHandler() {
    CAN_rx_ISR(CAN2, 0);
}

void __attribute__((weak, used)) CAN2_RX1_IRQHandler() {
    CAN_rx_ISR(CAN2, 1);
}

void CAN2_TX_IRQHandler() {
    CAN_wake_tx_from_ISR();
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

bool CAN_init() {
#ifdef USE_CAN1
    INIT_QUEUE(can1_tx_queue, CanMsgTypeDef_t, CAN_TX_QUEUE_LENGTH);
    can_tx_queues[BUS_TO_PERIPHERAL(CAN1)] = can1_tx_queue;

    NVIC_SetPriority(CAN1_RX0_IRQn, NVIC_RX_IRQ_PRIO);
    NVIC_SetPriority(CAN1_RX1_IRQn, NVIC_RX_IRQ_PRIO);
    NVIC_SetPriority(CAN1_TX_IRQn, NVIC_TX_IRQ_PRIO);
#endif

#ifdef USE_CAN2
    INIT_QUEUE(can2_tx_queue, CanMsgTypeDef_t, CAN_TX_QUEUE_LENGTH);
    can_tx_queues[BUS_TO_PERIPHERAL(CAN2)] = can2_tx_queue;

    NVIC_SetPriority(CAN2_RX0_IRQn, NVIC_RX_IRQ_PRIO);
    NVIC_SetPriority(CAN2_RX1_IRQn, NVIC_RX_IRQ_PRIO);
    NVIC_SetPriority(CAN2_TX_IRQn, NVIC_TX_IRQ_PRIO);
#endif

    INIT_QUEUE(can_rx_queue, CanMsgTypeDef_t, CAN_RX_QUEUE_LENGTH);
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

bool CAN_enable_IRQs() {
#ifdef USE_CAN1
    NVIC_EnableIRQ(CAN1_RX0_IRQn);
    NVIC_EnableIRQ(CAN1_RX1_IRQn);
    NVIC_EnableIRQ(CAN1_TX_IRQn);
#endif
#ifdef USE_CAN2
    NVIC_EnableIRQ(CAN2_RX0_IRQn);
    NVIC_EnableIRQ(CAN2_RX1_IRQn);
    NVIC_EnableIRQ(CAN2_TX_IRQn);
#endif
    return true;
}

#elif defined(STM32G474xx)

#ifdef USE_FDCAN1
DEFINE_QUEUE(can1_tx_queue, CanMsgTypeDef_t, CAN_TX_QUEUE_LENGTH);
#endif

#ifdef USE_FDCAN2
DEFINE_QUEUE(can2_tx_queue, CanMsgTypeDef_t, CAN_TX_QUEUE_LENGTH);
#endif

#ifdef USE_FDCAN3
DEFINE_QUEUE(can3_tx_queue, CanMsgTypeDef_t, CAN_TX_QUEUE_LENGTH);
#endif

void CAN_tx_update() {
    // Block until a notification is received
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // Feed the peripheral TX FIFOs until all queues are empty or FIFOs are full
    bool is_tx_success;
    do { // Loop to handle the case where a FIFO slot opens during processing
        is_tx_success = false;
        CanMsgTypeDef_t tx_msg;
        QueueHandle_t tx_queue;

#ifdef USE_FDCAN1
        tx_queue = can_tx_queues[BUS_TO_PERIPHERAL(FDCAN1)];
        while (PHAL_FDCAN_txFifoFree(FDCAN1) && xQueueReceive(tx_queue, &tx_msg, 0) == pdPASS) {
            PHAL_FDCAN_send(&tx_msg);
            is_tx_success = true;
        }
#endif

#ifdef USE_FDCAN2
        tx_queue = can_tx_queues[BUS_TO_PERIPHERAL(FDCAN2)];
        while (PHAL_FDCAN_txFifoFree(FDCAN2) && xQueueReceive(tx_queue, &tx_msg, 0) == pdPASS) {
            PHAL_FDCAN_send(&tx_msg);
            is_tx_success = true;
        }
#endif

#ifdef USE_FDCAN3
        tx_queue = can_tx_queues[BUS_TO_PERIPHERAL(FDCAN3)];
        while (PHAL_FDCAN_txFifoFree(FDCAN3) && xQueueReceive(tx_queue, &tx_msg, 0) == pdPASS) {
            PHAL_FDCAN_send(&tx_msg);
            is_tx_success = true;
        }
#endif
    } while (is_tx_success);
}

// FDCAN RX callback - enqueues received messages to the RX queue
void PHAL_FDCAN_rxCallback(CanMsgTypeDef_t *msg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (xQueueSendFromISR(can_rx_queue, msg, &xHigherPriorityTaskWoken) != pdPASS) {
        can_stats.rx_overflow++;
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// FDCAN TX callback - wakes the TX task to handle the next message
void PHAL_FDCAN_txCallback(FDCAN_GlobalTypeDef *fdcan) {
    (void)fdcan;
    CAN_wake_tx_from_ISR();
}

bool CAN_init() {
    // set up TX queues and filters for each FDCAN peripheral
#ifdef USE_FDCAN1
    INIT_QUEUE(can1_tx_queue, CanMsgTypeDef_t, CAN_TX_QUEUE_LENGTH);
    can_tx_queues[BUS_TO_PERIPHERAL(FDCAN1)] = can1_tx_queue;
    FDCAN1_set_filters();
    NVIC_SetPriority(FDCAN1_IT0_IRQn, NVIC_RX_IRQ_PRIO);
    NVIC_SetPriority(FDCAN1_IT1_IRQn, NVIC_TX_IRQ_PRIO);
#endif

#ifdef USE_FDCAN2
    INIT_QUEUE(can2_tx_queue, CanMsgTypeDef_t, CAN_TX_QUEUE_LENGTH);
    can_tx_queues[BUS_TO_PERIPHERAL(FDCAN2)] = can2_tx_queue;
    FDCAN2_set_filters();
    NVIC_SetPriority(FDCAN2_IT0_IRQn, NVIC_RX_IRQ_PRIO);
    NVIC_SetPriority(FDCAN2_IT1_IRQn, NVIC_TX_IRQ_PRIO);
#endif

#ifdef USE_FDCAN3
    INIT_QUEUE(can3_tx_queue, CanMsgTypeDef_t, CAN_TX_QUEUE_LENGTH);
    can_tx_queues[BUS_TO_PERIPHERAL(FDCAN3)] = can3_tx_queue;
    FDCAN3_set_filters();
    NVIC_SetPriority(FDCAN3_IT0_IRQn, NVIC_RX_IRQ_PRIO);
    NVIC_SetPriority(FDCAN3_IT1_IRQn, NVIC_TX_IRQ_PRIO);
#endif

    // Initialize RX queue
    INIT_QUEUE(can_rx_queue, CanMsgTypeDef_t, CAN_RX_QUEUE_LENGTH);

    // Clear stats
    can_stats = (can_stats_t) {0};

    // Initialize CAN data from generated code
    CAN_data_init();

    return true;
}

bool CAN_enable_IRQs() {
#ifdef USE_FDCAN1
    NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
    NVIC_EnableIRQ(FDCAN1_IT1_IRQn);
#endif
#ifdef USE_FDCAN2
    NVIC_EnableIRQ(FDCAN2_IT0_IRQn);
    NVIC_EnableIRQ(FDCAN2_IT1_IRQn);
#endif
#ifdef USE_FDCAN3
    NVIC_EnableIRQ(FDCAN3_IT0_IRQn);
    NVIC_EnableIRQ(FDCAN3_IT1_IRQn);
#endif
    return true;
}

#else
#error "Unsupported architecture"
#endif
