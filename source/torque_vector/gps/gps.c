#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "gps.h"
#include "can_parse.h"

/*
    Instructions on configuring GPS:
    1. GPS will not send messages without a fix, so find a space where the GPS gets a fix (use UCenter for this)
    2. Pull up configuration menu, go to "msg" configuration
        2a. Select "01-07 NAV-PVT", and enable it on your desired communication port and USB
        2b. Find any messages containing "NMEA", and disable them for USB and your desired communication port
        2c. Send this config change to the GPS
    3. Go to "PRT (Port)" configuration
        3a. This is where you configure your peripheral. Currently, USART1, with baudrate at 115200 is used. Also, ensure that "UBX" is selected ad protocol out.
        3b. Send this config change to the GPS
    4. Go to "RATE" configuration
        4a. Select 40ms as the measurement period
        4b. Send this config change to the GPS
    5. Go to "CFG" setting, and hit the "send" button to once again save changes to the GPS.
    6. At this point, confirm you changes by turning it off and reconnecting it, ensuring the correct message is sent at 40ms
*/

double checkVelN;
double checkVelN2;
signed long prev_iTOW;
uint16_t counter;
uint16_t diff;
// Nav Message
GPS_Handle_t gps_handle = {0};


bool GPS_Decode(GPS_Handle_t *gps) {
    if (!gps) { // Null pointer check
        return false;
    }

    NAV_PVT_t *nav_pvt = &(gps->decoded_message);
    uint8_t *rx_buffer = gps->gps_rx_buffer;

    // Check the header
    if ((rx_buffer[0] != UBX_NAV_PVT_HEADER_B0) || (rx_buffer[1] != UBX_NAV_PVT_HEADER_B1) ||
        (rx_buffer[2] != UBX_NAV_PVT_CLASS) || (rx_buffer[3] != UBX_NAV_PVT_MSG_ID)) {
        return false; // Invalid message header
    }

    // Decode NAV-PVT message
    nav_pvt->iTOW   = bytes_to_uint32(rx_buffer[6], rx_buffer[7], rx_buffer[8], rx_buffer[9]);
    nav_pvt->year   = bytes_to_uint16(rx_buffer[10], rx_buffer[11]);
    nav_pvt->month  = rx_buffer[12];
    nav_pvt->day    = rx_buffer[13];
    nav_pvt->hour   = rx_buffer[14];
    nav_pvt->minute = rx_buffer[15];
    nav_pvt->second = rx_buffer[16];

    nav_pvt->valid        = (gps_valid_flags_t)rx_buffer[17];
    nav_pvt->timeAccuracy = bytes_to_uint32(rx_buffer[18], rx_buffer[19], rx_buffer[20], rx_buffer[21]);
    nav_pvt->nano         = bytes_to_int32(rx_buffer[22], rx_buffer[23], rx_buffer[24], rx_buffer[25]);

    nav_pvt->fixType   = (gps_fix_type_t)rx_buffer[26];
    nav_pvt->fixStatus = (gps_fix_status_t)rx_buffer[27];

    nav_pvt->numSV              = rx_buffer[28];
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
    nav_pvt->magDec         = bytes_to_int16(rx_buffer[94], rx_buffer[95]);
    nav_pvt->magAcc         = bytes_to_uint16(rx_buffer[96], rx_buffer[97]);

    // Evaluate flags

}

// // Parse velocity from raw message
// bool GPS_Parse(GPS_Handle_t *GPS)
// {
//     // For future reference, we use the UBX protocol to communicate with GPS - Specifically UBX-NAV-PVT 
//     // Validate the message header, class, and id
//     if (((GPS->raw_message)[0] == UBX_NAV_PVT_HEADER_B0) && (GPS->raw_message[1] == UBX_NAV_PVT_HEADER_B1) &&
//         ((GPS->raw_message)[2] == UBX_NAV_PVT_CLASS) && ((GPS->raw_message)[3] == UBX_NAV_PVT_MSG_ID))
//     {
//         bool correctFix = false;
//         // Collect fix type
//         GPS->fix_type = GPS->raw_message[26];
//         if (GPS->fix_type > GPS_FIX_2D && GPS->fix_type < GPS_FIX_TIME_ONLY)
//         {
//             correctFix = true;
//             // Collect Ground Speed
//             iLong.bytes[0] = GPS->raw_message[66];
//             iLong.bytes[1] = GPS->raw_message[67];
//             iLong.bytes[2] = GPS->raw_message[68];
//             iLong.bytes[3] = GPS->raw_message[69];
//             GPS->g_speed = iLong.iLong; /* mm/s is the raw data */
//             GPS->g_speed_rounded = (int16_t)(GPS->g_speed / 10);

//             // Collect Longitude
//             iLong.bytes[0] = GPS->raw_message[30];
//             iLong.bytes[1] = GPS->raw_message[31];
//             iLong.bytes[2] = GPS->raw_message[32];
//             iLong.bytes[3] = GPS->raw_message[33];
//             GPS->longitude = iLong.iLong; /* deg * 10^7 is the raw data */
//             GPS->lon_rounded = (int32_t)(GPS->longitude);

//             // Colect Latitude
//             iLong.bytes[0] = GPS->raw_message[34];
//             iLong.bytes[1] = GPS->raw_message[35];
//             iLong.bytes[2] = GPS->raw_message[36];
//             iLong.bytes[3] = GPS->raw_message[37];
//             GPS->latitude = iLong.iLong; /* deg * 10^7 is the raw data */
//             GPS->lat_rounded = (int32_t)(GPS->latitude);

//             // Collect Height above mean sea level
//             iLong.bytes[0] = GPS->raw_message[42];
//             iLong.bytes[1] = GPS->raw_message[43];
//             iLong.bytes[2] = GPS->raw_message[44];
//             iLong.bytes[3] = GPS->raw_message[45];
//             GPS->height = iLong.iLong; /* mm is the raw data */
//             GPS->height_rounded = (int16_t)(GPS->height / 10);

//             // Collect North Velocity
//             iLong.bytes[0] = GPS->raw_message[54];
//             iLong.bytes[1] = GPS->raw_message[55];
//             iLong.bytes[2] = GPS->raw_message[56];
//             iLong.bytes[3] = GPS->raw_message[57];
//             GPS->n_vel = iLong.iLong; /* mm/s is the raw data */
//             GPS->n_vel_rounded = (int16_t)(GPS->n_vel / 10);

//             // Collect East Velocity
//             iLong.bytes[0] = GPS->raw_message[58];
//             iLong.bytes[1] = GPS->raw_message[59];
//             iLong.bytes[2] = GPS->raw_message[60];
//             iLong.bytes[3] = GPS->raw_message[61];
//             GPS->e_vel = iLong.iLong; /* mm/s is the raw data */
//             GPS->e_vel_rounded = (int16_t)(GPS->e_vel / 10);

//             // Collect Down Velocity
//             iLong.bytes[0] = GPS->raw_message[62];
//             iLong.bytes[1] = GPS->raw_message[63];
//             iLong.bytes[2] = GPS->raw_message[64];
//             iLong.bytes[3] = GPS->raw_message[65];
//             GPS->d_vel = iLong.iLong; /* mm/s is the raw data */
//             GPS->d_vel_rounded = (int16_t)(GPS->d_vel / 10);

//             // Collect Vehicle Heading of Motion
//             iLong.bytes[0] = GPS->raw_message[70];
//             iLong.bytes[1] = GPS->raw_message[71];
//             iLong.bytes[2] = GPS->raw_message[72];
//             iLong.bytes[3] = GPS->raw_message[73];
//             GPS->headVeh = iLong.iLong; /* deg * 10^5 is the raw data */
//             GPS->headVeh_rounded = (int16_t)(GPS->headVeh / 10000); /* deg * 10^1 */

//             // Collect iTOW (Time of Week)
//             uLong.bytes[0] = GPS->raw_message[6];
//             uLong.bytes[1] = GPS->raw_message[7];
//             uLong.bytes[2] = GPS->raw_message[8];
//             uLong.bytes[3] = GPS->raw_message[9];
//             GPS->iTOW = uLong.uLong; /* ms is the raw data */

            
//             // Collect date and time (UTC)
//             iShort.bytes[0] = GPS->raw_message[10];
//             iShort.bytes[1] = GPS->raw_message[11];
//             GPS->year = (uint8_t)(iShort.iShort - 2000); // chop off 2000 from the year and store as uint8_t

//             GPS->month = GPS->raw_message[12];
//             GPS->day = GPS->raw_message[13];
//             GPS->hour = GPS->raw_message[14];
//             GPS->minute = GPS->raw_message[15];
//             GPS->second = GPS->raw_message[16];

//             iLong.bytes[0] = GPS->raw_message[18];
//             iLong.bytes[1] = GPS->raw_message[19];
//             iLong.bytes[2] = GPS->raw_message[20];
//             iLong.bytes[3] = GPS->raw_message[21];
//             GPS->millisecond = (int16_t)(iLong.iLong / 1000000); // convert nanoseconds to milliseconds

//             GPS->is_valid_date = (GPS->raw_message[17] & GPS_VALID_DATE) ? true : false;
//             GPS->is_valid_time = (GPS->raw_message[17] & GPS_VALID_TIME) ? true : false;
//             GPS->is_fully_resolved = (GPS->raw_message[17] & GPS_VALID_FULLY_RESOLVED) ? true : false;
//             GPS->is_valid_mag = (GPS->raw_message[17] & GPS_VALID_MAG) ? true : false;

//             /* Determine if data is new */
//             if (GPS->iTOW != prev_iTOW) {
//                 GPS->is_unique_iTOW = true;
//                 diff = GPS->iTOW - prev_iTOW;
//                 counter = 0;
//                 prev_iTOW = GPS->iTOW;
//             } else {
//                 GPS->is_unique_iTOW = false;
//                 ++counter;
//             }

//             // Only post onto CAN if GPS is fully resolved
//             if (GPS->is_fully_resolved) {
//                 SEND_GPS_TIME(GPS->year, GPS->month, GPS->day, GPS->hour, GPS->minute, GPS->second, GPS->millisecond);
//             }

//             SEND_GPS_VELOCITY(GPS->n_vel_rounded, GPS->e_vel_rounded, GPS->d_vel_rounded);
//             SEND_GPS_SPEED(GPS->g_speed_rounded, GPS->headVeh_rounded);

//             SEND_GPS_COORDINATES(GPS->lat_rounded, GPS->lon_rounded);
//             SEND_GPS_POSITION(GPS->height_rounded);
//         }

//         // Collect Magnetic Declination
//         iShort.bytes[0] = GPS->raw_message[94];
//         iShort.bytes[1] = GPS->raw_message[95];
//         GPS->mag_dec = iShort.iShort; /* deg * 10^2 is the raw data */

//         return correctFix;
//     }
//     return false;
// }
