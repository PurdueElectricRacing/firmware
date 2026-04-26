#include "common/freertos/freertos.h"

volatile uint32_t last_gps_message_time_ms = 0;
void gps_thread(void) {
    // todo block until woken by dma interrupt

    last_gps_message_time_ms = xTaskGetTickCount();

    if (last_gps_message_time_ms == 0) {
        // this should never happen, but just in case
        last_gps_message_time_ms = 1;
    }

    // validate the gps header
        // return if invalid

    // identify the gps message type

    // dispatch to the correct decoder
}