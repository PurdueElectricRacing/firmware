/**
 * @file can_tx.c
 * @brief Common CAN TX queue, task, and ISR/callback handling.
 */

#include "can_common.h"

extern osThreadId_t CAN_tx_update_handle;

volatile can_stats_t can_stats;
QueueHandle_t can_tx_queues[CAN_NUM_PERIPHERALS];

// Guard against bad arch defines
#if !defined(STM32F407xx) && !defined(STM32G474xx)
#error "Unsupported architecture"
#elif defined(STM32F407xx) && defined(STM32G474xx)
#error "Multiple architectures defined"
#endif

#ifdef USE_CAN1
DEFINE_QUEUE(can1_tx_queue, CanMsgTypeDef_t, CAN_TX_QUEUE_LENGTH);
#endif
#ifdef USE_CAN2
DEFINE_QUEUE(can2_tx_queue, CanMsgTypeDef_t, CAN_TX_QUEUE_LENGTH);
#endif
#ifdef USE_FDCAN1
DEFINE_QUEUE(can1_tx_queue, CanMsgTypeDef_t, CAN_TX_QUEUE_LENGTH);
#endif
#ifdef USE_FDCAN2
DEFINE_QUEUE(can2_tx_queue, CanMsgTypeDef_t, CAN_TX_QUEUE_LENGTH);
#endif
#ifdef USE_FDCAN3
DEFINE_QUEUE(can3_tx_queue, CanMsgTypeDef_t, CAN_TX_QUEUE_LENGTH);
#endif

void CAN_tx_init(void) {
#ifdef USE_CAN1
    INIT_QUEUE(can1_tx_queue, CanMsgTypeDef_t, CAN_TX_QUEUE_LENGTH);
    can_tx_queues[BUS_TO_PERIPHERAL(CAN1)] = can1_tx_queue;
    CAN1->IER &= ~CAN_IER_TMEIE;
#endif
#ifdef USE_CAN2
    INIT_QUEUE(can2_tx_queue, CanMsgTypeDef_t, CAN_TX_QUEUE_LENGTH);
    can_tx_queues[BUS_TO_PERIPHERAL(CAN2)] = can2_tx_queue;
    CAN2->IER &= ~CAN_IER_TMEIE;
#endif
#ifdef USE_FDCAN1
    INIT_QUEUE(can1_tx_queue, CanMsgTypeDef_t, CAN_TX_QUEUE_LENGTH);
    can_tx_queues[BUS_TO_PERIPHERAL(FDCAN1)] = can1_tx_queue;
#endif
#ifdef USE_FDCAN2
    INIT_QUEUE(can2_tx_queue, CanMsgTypeDef_t, CAN_TX_QUEUE_LENGTH);
    can_tx_queues[BUS_TO_PERIPHERAL(FDCAN2)] = can2_tx_queue;
#endif
#ifdef USE_FDCAN3
    INIT_QUEUE(can3_tx_queue, CanMsgTypeDef_t, CAN_TX_QUEUE_LENGTH);
    can_tx_queues[BUS_TO_PERIPHERAL(FDCAN3)] = can3_tx_queue;
#endif
}

void CAN_enqueue_tx(CanMsgTypeDef_t *msg) {
    CAN_peripheral_t peripheral = BUS_TO_PERIPHERAL(msg->Bus);

    if (xQueueSendToBack(can_tx_queues[peripheral], msg, 0) != pdPASS) {
        can_stats.tx_overflow++;
        return;
    }

    if (CAN_tx_update_handle != NULL) {
        xTaskNotifyGive(CAN_tx_update_handle);
    }
}

static inline void CAN_wake_tx_from_ISR(void) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (CAN_tx_update_handle != NULL) {
        vTaskNotifyGiveFromISR(CAN_tx_update_handle, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

#if defined(STM32F407xx)

static void CAN_drain_tx_bus(CAN_TypeDef *bus) {
    CAN_peripheral_t peripheral = BUS_TO_PERIPHERAL(bus);
    QueueHandle_t tx_queue = can_tx_queues[peripheral];

    CanMsgTypeDef_t tx_msg;
    uint8_t free_index;

    while (PHAL_getFreeTxMailbox(bus, &free_index)) {
        if (xQueueReceive(tx_queue, &tx_msg, 0) != pdPASS) {
            bus->IER &= ~CAN_IER_TMEIE;
            return;
        }

        PHAL_txCANMessage(&tx_msg, free_index);
    }

    if (uxQueueMessagesWaiting(tx_queue) > 0) {
        bus->IER |= CAN_IER_TMEIE;
    } else {
        bus->IER &= ~CAN_IER_TMEIE;
    }
}

void CAN_tx_update(void) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

#ifdef USE_CAN1
    CAN_drain_tx_bus(CAN1);
#endif

#ifdef USE_CAN2
    CAN_drain_tx_bus(CAN2);
#endif
}

static inline void CAN_tx_ISR(CAN_TypeDef *bus) {
    uint32_t tsr = bus->TSR;

    if (tsr & CAN_TSR_RQCP0) bus->TSR = CAN_TSR_RQCP0;
    if (tsr & CAN_TSR_RQCP1) bus->TSR = CAN_TSR_RQCP1;
    if (tsr & CAN_TSR_RQCP2) bus->TSR = CAN_TSR_RQCP2;

    bus->IER &= ~CAN_IER_TMEIE;

    CAN_wake_tx_from_ISR();
}

#ifdef USE_CAN1
void CAN1_TX_IRQHandler(void) {
    CAN_tx_ISR(CAN1);
}
#endif

#ifdef USE_CAN2
void CAN2_TX_IRQHandler(void) {
    CAN_tx_ISR(CAN2);
}
#endif

#else // STM32G474xx

static void CAN_drain_tx_bus(FDCAN_GlobalTypeDef *bus) {
    CAN_peripheral_t peripheral = BUS_TO_PERIPHERAL(bus);
    QueueHandle_t tx_queue = can_tx_queues[peripheral];

    CanMsgTypeDef_t tx_msg;

    while (PHAL_FDCAN_txFifoFree(bus)) {
        if (xQueueReceive(tx_queue, &tx_msg, 0) != pdPASS) {
            return;
        }

        PHAL_FDCAN_send(&tx_msg);
    }

    /*
     * TODO:
     * When PHAL exposes FDCAN TX-space IRQ arm/disarm,
     * arm it here only if uxQueueMessagesWaiting(tx_queue) > 0.
     */
}

void CAN_tx_update(void) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
#ifdef USE_FDCAN1
    CAN_drain_tx_bus(FDCAN1);
#endif
#ifdef USE_FDCAN2
    CAN_drain_tx_bus(FDCAN2);
#endif
#ifdef USE_FDCAN3
    CAN_drain_tx_bus(FDCAN3);
#endif
}

void PHAL_FDCAN_txCallback(FDCAN_GlobalTypeDef *fdcan) {
    (void)fdcan;
    CAN_wake_tx_from_ISR();
}

#endif