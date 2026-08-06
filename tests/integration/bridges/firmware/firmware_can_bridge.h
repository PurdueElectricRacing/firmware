#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

bool per_fw_send_main_hb(uint8_t car_state, uint32_t timestamp, uint8_t output[16]);
bool per_fw_dispatch_can(uint32_t id, const uint8_t *payload, uint8_t length, uint32_t tick);
bool per_fw_start_button_pressed(void);
uint32_t per_fw_start_button_last_rx(void);
uint32_t per_fw_main_hb_id(void);
uint32_t per_fw_start_button_id(void);

#ifdef __cplusplus
}
#endif
