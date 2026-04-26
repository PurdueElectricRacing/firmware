/**
 * @file gps.c
 * @brief GPS RX and parsing thread
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "sensors.h"
#include "common/freertos/freertos.h"

volatile uint8_t rover_rx_buffer[ROVER_TX_SIZE] = {0}; // updated by DMA stream
NAV_PVT_data_t nav_pvt = {0};
NAV_RELPOSNED_data_t nav_relposned = {0};

volatile uint32_t last_gps_message_time_ms = 0;
void gps_periodic(void) {
    // todo block until woken by dma interrupt

    last_gps_message_time_ms = xTaskGetTickCount();

    NAV_PVT_decode(&nav_pvt, rover_rx_buffer);
    NAV_RELPOSNED_decode(&nav_relposned, (rover_rx_buffer + NAV_PVT_TOTAL_LENGTH));

    // todo:
    // validate the gps header
        // return if invalid

    // identify the gps message type

    // dispatch to the correct decoder
}