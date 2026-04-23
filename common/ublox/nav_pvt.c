
#include <stdint.h>
#include <string.h>
#include "nav_pvt.h"

void NAV_PVT_decode(NAV_PVT_data_t *nav_pvt, uint8_t *rx_buffer) {
    bool is_header_valid =
        (rx_buffer[0] == UBX_NAV_PVT_HEADER_B0) &&
        (rx_buffer[1] == UBX_NAV_PVT_HEADER_B1) &&
        (rx_buffer[2] == UBX_NAV_PVT_CLASS) &&
        (rx_buffer[3] == UBX_NAV_PVT_MSG_ID);

    if (!is_header_valid) {
        return;
    }

    memcpy(nav_pvt, (rx_buffer + 6), sizeof(NAV_PVT_data_t));


    // todo validate checksum
}