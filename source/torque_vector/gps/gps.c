#include "gps.h"

#include <stdbool.h>
#include <stdint.h>

bool GPS_Decode(GPS_Handle_t *gps) {
    if (!gps) { // Null pointer check
        return false;
    }

    NAV_PVT_t *nav_pvt = &(gps->data);
    uint8_t *rx_buffer = gps->gps_rx_buffer;

    // Check the header
    if (!is_valid_header(rx_buffer[0], rx_buffer[1], rx_buffer[2], rx_buffer[3])) {
        return false;
    }

    // Decode NAV-PVT message
    nav_pvt->iTOW = bytes_to_uint32(rx_buffer[6], rx_buffer[7], rx_buffer[8], rx_buffer[9]);

    // TODO Check unique timestamp?

    nav_pvt->year   = bytes_to_uint16(rx_buffer[10], rx_buffer[11]);
    nav_pvt->month  = rx_buffer[12];
    nav_pvt->day    = rx_buffer[13];
    nav_pvt->hour   = rx_buffer[14];
    nav_pvt->minute = rx_buffer[15];
    nav_pvt->second = rx_buffer[16];

    nav_pvt->valid        = (gps_valid_flags_t)rx_buffer[17];
    nav_pvt->timeAccuracy = bytes_to_uint32(rx_buffer[18], rx_buffer[19], rx_buffer[20], rx_buffer[21]);
    nav_pvt->nano         = bytes_to_int32(rx_buffer[22], rx_buffer[23], rx_buffer[24], rx_buffer[25]);

    nav_pvt->fixType = (gps_fix_type_t)rx_buffer[26];
    nav_pvt->flags1  = (gps_flags1_t)rx_buffer[27];
    nav_pvt->flags2  = (gps_flags2_t)rx_buffer[28];

    nav_pvt->numSatellites      = rx_buffer[29];
    nav_pvt->longitude          = bytes_to_int32(rx_buffer[30], rx_buffer[31], rx_buffer[32], rx_buffer[33]);
    nav_pvt->latitude           = bytes_to_int32(rx_buffer[34], rx_buffer[35], rx_buffer[36], rx_buffer[37]);
    nav_pvt->height             = bytes_to_int32(rx_buffer[42], rx_buffer[43], rx_buffer[44], rx_buffer[45]);
    nav_pvt->heightMSL          = bytes_to_int32(rx_buffer[46], rx_buffer[47], rx_buffer[48], rx_buffer[49]);
    nav_pvt->horizontalAccuracy = bytes_to_uint32(rx_buffer[50], rx_buffer[51], rx_buffer[52], rx_buffer[53]);
    nav_pvt->verticalAccuracy   = bytes_to_uint32(rx_buffer[54], rx_buffer[55], rx_buffer[56], rx_buffer[57]);

    nav_pvt->velNorth      = bytes_to_int32(rx_buffer[58], rx_buffer[59], rx_buffer[60], rx_buffer[61]);
    nav_pvt->velEast       = bytes_to_int32(rx_buffer[62], rx_buffer[63], rx_buffer[64], rx_buffer[65]);
    nav_pvt->velDown       = bytes_to_int32(rx_buffer[66], rx_buffer[67], rx_buffer[68], rx_buffer[69]);
    nav_pvt->groundSpeed   = bytes_to_int32(rx_buffer[70], rx_buffer[71], rx_buffer[72], rx_buffer[73]);
    nav_pvt->headingMotion = bytes_to_int32(rx_buffer[74], rx_buffer[75], rx_buffer[76], rx_buffer[77]);
    nav_pvt->speedAccuracy = bytes_to_uint32(rx_buffer[78], rx_buffer[79], rx_buffer[80], rx_buffer[81]);

    nav_pvt->positionDOP = bytes_to_uint16(rx_buffer[82], rx_buffer[83]);

    nav_pvt->reserved[0] = rx_buffer[84];
    nav_pvt->reserved[1] = rx_buffer[85];
    nav_pvt->reserved[2] = rx_buffer[86];
    nav_pvt->reserved[3] = rx_buffer[87];
    nav_pvt->reserved[4] = rx_buffer[88];
    nav_pvt->reserved[5] = rx_buffer[89];

    nav_pvt->headingVehicle = bytes_to_int32(rx_buffer[90], rx_buffer[91], rx_buffer[92], rx_buffer[93]);
    nav_pvt->magneticDec    = bytes_to_int16(rx_buffer[94], rx_buffer[95]);
    nav_pvt->magneticAcc    = bytes_to_uint16(rx_buffer[96], rx_buffer[97]);

    // TODO Process checksum

    // Evaluate flags
    gps->isValidDate     = (nav_pvt->valid & GPS_VALID_DATE);
    gps->isValidTime     = (nav_pvt->valid & GPS_VALID_TIME);
    gps->isFullyResolved = (nav_pvt->valid & GPS_VALID_FULLY_RESOLVED);
    gps->isValidMag      = (nav_pvt->valid & GPS_VALID_MAG);

    gps->hasGNSSFix = ((nav_pvt->fixType == GPS_FIX_TYPE_GNSS_2D) || (nav_pvt->fixType == GPS_FIX_TYPE_GNSS_3D));

    return true;
}