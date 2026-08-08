#include "firmware_can_bridge.h"

#include <string.h>

#include "firmware/can_library/generated/MAIN_MODULE.h"
#include "firmware/source/daq/spmc/timestamped_frame.h"

FDCAN_GlobalTypeDef per_test_fdcan2 = {.instance = 2};
FDCAN_GlobalTypeDef per_test_fdcan3 = {.instance = 3};
uint32_t per_test_tick_ms = 0;
volatile can_data_t can_data;

static CanMsgTypeDef_t captured_message;
static bool message_captured;

void a_box_fault_event_CALLBACK(void) {}
void dashboard_fault_event_CALLBACK(void) {}
void pdu_fault_event_CALLBACK(void) {}
void torque_vector_fault_event_CALLBACK(void) {}
void a_box_fault_sync_CALLBACK(void) {}
void dashboard_fault_sync_CALLBACK(void) {}
void pdu_fault_sync_CALLBACK(void) {}
void torque_vector_fault_sync_CALLBACK(void) {}

void CAN_enqueue_tx_FDCAN2(CanMsgTypeDef_t *message) {
    captured_message = *message;
    message_captured = true;
}

void CAN_enqueue_tx_FDCAN3(CanMsgTypeDef_t *message) {
    captured_message = *message;
    message_captured = true;
}

bool per_fw_send_main_hb(uint8_t car_state, uint32_t timestamp, uint8_t output[16]) {
    message_captured = false;
    CAN_SEND_main_hb((car_state_t)car_state);
    if (!message_captured || captured_message.IDE || captured_message.DLC > 8) {
        return false;
    }

    timestamped_frame_t frame = {
        .ticks_ms = timestamp,
        .identity = captured_message.StdId,
        .payload = 0,
    };
    memcpy(&frame.payload, captured_message.Data, captured_message.DLC);
    memcpy(output, &frame, sizeof(frame));
    return true;
}

bool per_fw_dispatch_can(uint32_t id, const uint8_t *payload, uint8_t length, uint32_t tick) {
    if (payload == NULL) {
        return false;
    }
    per_test_tick_ms = tick;
    CAN_rx_dispatcher(id, (uint8_t *)payload, length, FDCAN2);
    return true;
}

bool per_fw_start_button_pressed(void) {
    return can_data.start_button.is_pressed;
}

uint32_t per_fw_start_button_last_rx(void) {
    return can_data.start_button.last_rx;
}

uint32_t per_fw_main_hb_id(void) {
    return MAIN_HB_MSG_ID;
}

uint32_t per_fw_start_button_id(void) {
    return START_BUTTON_MSG_ID;
}
