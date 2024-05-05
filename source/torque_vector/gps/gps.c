#include <stdio.h>
#include <stdbool.h>
#include "gps.h"
//#include "sfs_pp.h"
//#include "SFS.h"

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

union i_Long iLong;
union i_Short iShort;
union u_Long uLong;

extern q_handle_t q_tx_can;
double checkVelN;
double checkVelN2;
signed long prev_iTOW;
uint16_t counter;
u_int16_t diff;
// Nav Message
GPS_Handle_t gps_handle = {.raw_message = {0},
                           .g_speed = 0,
                           .g_speed_bytes = {0xFF, 0xFF, 0xFF, 0xFF},
                           .g_speed_rounded = 0,
                           .longitude_bytes = {0xFF, 0xFF, 0xFF, 0xFF},
                           .longitude = 0,
                           .lon_rounded = 0,
                           .latitude_bytes = {0xFF, 0xFF, 0xFF, 0xFF},
                           .latitude = 0,
                           .lat_rounded = 0,
                           .height_bytes = {0xFF, 0xFF, 0xFF, 0xFF},
                           .height = 0,
                           .height_rounded = 0,
                           .n_vel_bytes = {0xFF, 0xFF, 0xFF, 0xFF},
                           .n_vel = 0,
                           .n_vel_rounded = 0,
                           .e_vel_bytes = {0xFF, 0xFF, 0xFF, 0xFF},
                           .e_vel = 0,
                           .e_vel_rounded = 0,
                           .d_vel_bytes = {0xFF, 0xFF, 0xFF, 0xFF},
                           .d_vel = 0,
                           .d_vel_rounded = 0,
                           .headVeh_bytes = {0xFF, 0xFF, 0xFF, 0xFF},
                           .headVeh = 0,
                           .headVeh_rounded = 0,
                           .mag_dec_bytes = {0xFF, 0xFF},
                           .mag_dec = 0,
                           .fix_type = -1,
                           .iTOW_bytes = {0xFF, 0xFF, 0xFF, 0xFF},
                           .iTOW = 0,
                           .unique_iTOW = true,
                           .messages_received = -1};

// Parse velocity from raw message
bool parseVelocity(GPS_Handle_t *GPS)
{
    // For future reference, we use the UBX protocol to communicate with GPS - Specifically UBX-NAV-PVT
    // Validate the message header, class, and id
    if (((GPS->raw_message)[0] == UBX_NAV_PVT_HEADER_B0) && (GPS->raw_message[1] == UBX_NAV_PVT_HEADER_B1) &&
        ((GPS->raw_message)[2] == UBX_NAV_PVT_CLASS) && ((GPS->raw_message)[3] == UBX_NAV_PVT_MSG_ID))
    {
        bool correctFix = false;
        // Collect fix type
        GPS->fix_type = GPS->raw_message[26];
        if (GPS->fix_type > GPS_FIX_2D && GPS->fix_type < GPS_FIX_TIME_ONLY)
        {
            correctFix = true;
            // Collect Ground Speed
            GPS->g_speed_bytes[0] = GPS->raw_message[66];
            GPS->g_speed_bytes[1] = GPS->raw_message[67];
            GPS->g_speed_bytes[2] = GPS->raw_message[68];
            GPS->g_speed_bytes[3] = GPS->raw_message[69];
            iLong.bytes[0] = GPS->raw_message[66];
            iLong.bytes[1] = GPS->raw_message[67];
            iLong.bytes[2] = GPS->raw_message[68];
            iLong.bytes[3] = GPS->raw_message[69];
            GPS->g_speed = iLong.iLong; /* mm/s is the raw data */
            GPS->g_speed_rounded = (int16_t)(GPS->g_speed / 10);

            // Collect Longitude
            GPS->longitude_bytes[0] = GPS->raw_message[30];
            GPS->longitude_bytes[1] = GPS->raw_message[31];
            GPS->longitude_bytes[2] = GPS->raw_message[32];
            GPS->longitude_bytes[3] = GPS->raw_message[33];
            iLong.bytes[0] = GPS->raw_message[30];
            iLong.bytes[1] = GPS->raw_message[31];
            iLong.bytes[2] = GPS->raw_message[32];
            iLong.bytes[3] = GPS->raw_message[33];
            GPS->longitude = iLong.iLong; /* deg * 10^7 is the raw data */
            GPS->lon_rounded = (int32_t)(GPS->longitude);

            // Colect Latitude
            GPS->latitude_bytes[0] = GPS->raw_message[34];
            GPS->latitude_bytes[1] = GPS->raw_message[35];
            GPS->latitude_bytes[2] = GPS->raw_message[36];
            GPS->latitude_bytes[3] = GPS->raw_message[37];
            iLong.bytes[0] = GPS->raw_message[34];
            iLong.bytes[1] = GPS->raw_message[35];
            iLong.bytes[2] = GPS->raw_message[36];
            iLong.bytes[3] = GPS->raw_message[37];
            GPS->latitude = iLong.iLong; /* deg * 10^7 is the raw data */
            GPS->lat_rounded = (int32_t)(GPS->latitude);

            // Collect Height above mean sea level
            GPS->height_bytes[0] = GPS->raw_message[42];
            GPS->height_bytes[1] = GPS->raw_message[43];
            GPS->height_bytes[2] = GPS->raw_message[44];
            GPS->height_bytes[3] = GPS->raw_message[45];
            iLong.bytes[0] = GPS->raw_message[42];
            iLong.bytes[1] = GPS->raw_message[43];
            iLong.bytes[2] = GPS->raw_message[44];
            iLong.bytes[3] = GPS->raw_message[45];
            GPS->height = iLong.iLong; /* mm is the raw data */
            GPS->height_rounded = (int16_t)(GPS->height / 10);

            // Collect North Velocity
            GPS->n_vel_bytes[0] = GPS->raw_message[54];
            GPS->n_vel_bytes[1] = GPS->raw_message[55];
            GPS->n_vel_bytes[2] = GPS->raw_message[56];
            GPS->n_vel_bytes[3] = GPS->raw_message[57];
            iLong.bytes[0] = GPS->raw_message[54];
            iLong.bytes[1] = GPS->raw_message[55];
            iLong.bytes[2] = GPS->raw_message[56];
            iLong.bytes[3] = GPS->raw_message[57];
            GPS->n_vel = iLong.iLong; /* mm/s is the raw data */
            GPS->n_vel_rounded = (int16_t)(GPS->n_vel / 10);

            // Collect East Velocity
            GPS->e_vel_bytes[0] = GPS->raw_message[58];
            GPS->e_vel_bytes[1] = GPS->raw_message[59];
            GPS->e_vel_bytes[2] = GPS->raw_message[60];
            GPS->e_vel_bytes[3] = GPS->raw_message[61];
            iLong.bytes[0] = GPS->raw_message[58];
            iLong.bytes[1] = GPS->raw_message[59];
            iLong.bytes[2] = GPS->raw_message[60];
            iLong.bytes[3] = GPS->raw_message[61];
            GPS->e_vel = iLong.iLong; /* mm/s is the raw data */
            GPS->e_vel_rounded = (int16_t)(GPS->e_vel / 10);

            // Collect Down Velocity
            GPS->d_vel_bytes[0] = GPS->raw_message[62];
            GPS->d_vel_bytes[1] = GPS->raw_message[63];
            GPS->d_vel_bytes[2] = GPS->raw_message[64];
            GPS->d_vel_bytes[3] = GPS->raw_message[65];
            iLong.bytes[0] = GPS->raw_message[62];
            iLong.bytes[1] = GPS->raw_message[63];
            iLong.bytes[2] = GPS->raw_message[64];
            iLong.bytes[3] = GPS->raw_message[65];
            GPS->d_vel = iLong.iLong; /* mm/s is the raw data */
            GPS->d_vel_rounded = (int16_t)(GPS->d_vel / 10);

            // Collect Vehicle Heading of Motion
            GPS->headVeh_bytes[0] = GPS->raw_message[70];
            GPS->headVeh_bytes[1] = GPS->raw_message[71];
            GPS->headVeh_bytes[2] = GPS->raw_message[72];
            GPS->headVeh_bytes[3] = GPS->raw_message[73];
            iLong.bytes[0] = GPS->raw_message[70];
            iLong.bytes[1] = GPS->raw_message[71];
            iLong.bytes[2] = GPS->raw_message[72];
            iLong.bytes[3] = GPS->raw_message[73];
            GPS->headVeh = iLong.iLong; /* deg * 10^5 is the raw data */
            GPS->headVeh_rounded = (int16_t)(GPS->headVeh / 10000); /* deg * 10^1 */

            // Collect iTOW (Time of Week)
            GPS->iTOW_bytes[0] = GPS->raw_message[6];
            GPS->iTOW_bytes[1] = GPS->raw_message[7];
            GPS->iTOW_bytes[2] = GPS->raw_message[8];
            GPS->iTOW_bytes[3] = GPS->raw_message[9];
            uLong.bytes[0] = GPS->raw_message[6];
            uLong.bytes[1] = GPS->raw_message[7];
            uLong.bytes[2] = GPS->raw_message[8];
            uLong.bytes[3] = GPS->raw_message[9];
            GPS->iTOW = uLong.uLong; /* ms is the raw data */

            /* Determine if data is new */
            if (GPS->iTOW != prev_iTOW) {
                GPS->unique_iTOW = true;
                diff = GPS->iTOW - prev_iTOW;
                counter = 0;
                prev_iTOW = GPS->iTOW;
            } else {
                GPS->unique_iTOW = false;
                ++counter;
            }

            SEND_GPS_VELOCITY(GPS->n_vel_rounded, GPS->e_vel_rounded, GPS->d_vel_rounded);
            SEND_GPS_SPEED(GPS->g_speed_rounded, GPS->headVeh_rounded);

            SEND_GPS_COORDINATES(GPS->lat_rounded, GPS->lon_rounded);
            SEND_GPS_POSITION(GPS->height_rounded);
        }

        // Collect Magnetic Declination
        GPS->mag_dec_bytes[0] = GPS->raw_message[94];
        GPS->mag_dec_bytes[1] = GPS->raw_message[95];
        iShort.bytes[0] = GPS->raw_message[94];
        iShort.bytes[1] = GPS->raw_message[95];
        GPS->mag_dec = iShort.iShort; /* deg * 10^2 is the raw data */

        return correctFix;
    }
    return false;
}