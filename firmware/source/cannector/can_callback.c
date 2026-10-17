/**
 * @file can_callback.c
 * @brief CAN callback implementation
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include <string.h>
#include "activity_leds.h"

#include "common/phal_G4/fdcan/fdcan.h"
#include "common/rtos/rtos.h"
#include "common/timestamped_frame/timestamped_frame.h"
#include "main.h"

volatile uint32_t last_can_rx_time_ms = 0;

// Callback must be ISR safe
void PHAL_FDCAN_rxCallback(CanMsgTypeDef_t *msg) {
    timestamped_frame_t frame = {0};

    frame.ticks_ms      = xTaskGetTickCountFromISR();
    last_can_rx_time_ms = frame.ticks_ms;

    // todo replace when superDBC is merged
    set_bus_id(&frame, (msg->Bus == FDCAN1) ? 0 : 1); 
    set_xid(&frame, msg->IDE);
    set_can_id(&frame, msg->IDE ? msg->ExtId : msg->StdId);

    memcpy(&frame.payload, msg->Data, msg->DLC);

    // If full, discard the oldest frame and append the new frame
    // All CAN producers use the same IRQ priority, so they cannot
    // preempt this sequence and the USB consumer runs in a task context.
    if (xQueueSendToBackFromISR(can_queue, &frame, nullptr) != pdTRUE) {
        timestamped_frame_t oldest;
        xQueueReceiveFromISR(can_queue, &oldest, nullptr);
        xQueueSendToBackFromISR(can_queue, &frame, nullptr);
    }

    // Update the activity LED for this bus
    if (msg->Bus == FDCAN1) {
        activity_leds_record(ACTIVITY_VCAN, frame.ticks_ms);
    } else if (msg->Bus == FDCAN2) {
        activity_leds_record(ACTIVITY_MCAN, frame.ticks_ms);
    } else if (msg->Bus == FDCAN3) {
        activity_leds_record(ACTIVITY_SCAN, frame.ticks_ms);
    }
}
