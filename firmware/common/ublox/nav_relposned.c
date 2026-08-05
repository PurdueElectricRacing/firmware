/**
 * @file nav_relposned.c
 * @brief UBX NAV-RELPOSNED message definition and decoder function.
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "nav_relposned.h"
#include <string.h>

void NAV_RELPOSNED_decode(NAV_RELPOSNED_data_t *nav_relposned, const volatile uint8_t *rx_buffer) {
    bool is_header_valid =
        (rx_buffer[0] == NAV_RELPOSNED_HEADER_B0) &&
        (rx_buffer[1] == NAV_RELPOSNED_HEADER_B1) &&
        (rx_buffer[2] == NAV_RELPOSNED_CLASS) &&
        (rx_buffer[3] == NAV_RELPOSNED_MSG_ID);

    if (!is_header_valid) {
        return;
    }

    memcpy(nav_relposned, (uint8_t *)(rx_buffer + 6), sizeof(NAV_RELPOSNED_data_t));

    // todo validate checksum
}