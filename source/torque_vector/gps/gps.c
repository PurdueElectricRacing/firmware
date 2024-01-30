#include <stdio.h>
#include <stdbool.h>
#include "gps.h"
#include "sfs_pp.h"
#include "SFS.h"

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

extern q_handle_t q_tx_can;
double checkVelN;
double checkVelN2;
// Nav Message
GPS_Handle_t gps_handle = {.raw_message = {0},
                           .g_speed = 0,
                           .g_speed_bytes = {0xFF, 0xFF, 0xFF, 0xFF},
                           .speed_rounded = 0,
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
                           .mag_dec_bytes = {0xFF, 0xFF},
                           .mag_dec = 0,
                           .fix_type = -1,
                           .messages_received = -1};

// Parse velocity from raw message
bool parseVelocity(GPS_Handle_t *GPS, ExtU *rtU)
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
            GPS->g_speed = iLong.iLong;
            GPS->speed_rounded = (int16_t)((GPS->g_speed * 100) / 10000);

            // Collect Longitude
            GPS->longitude_bytes[0] = GPS->raw_message[30];
            GPS->longitude_bytes[1] = GPS->raw_message[31];
            GPS->longitude_bytes[2] = GPS->raw_message[32];
            GPS->longitude_bytes[3] = GPS->raw_message[33];
            iLong.bytes[0] = GPS->raw_message[30];
            iLong.bytes[1] = GPS->raw_message[31];
            iLong.bytes[2] = GPS->raw_message[32];
            iLong.bytes[3] = GPS->raw_message[33];
            GPS->longitude = iLong.iLong;
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
            GPS->latitude = iLong.iLong;
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
            GPS->height = iLong.iLong;
            GPS->height_rounded = (int16_t)((GPS->height * 100) / 10000);

            // Collect North Velocity
            GPS->n_vel_bytes[0] = GPS->raw_message[54];
            GPS->n_vel_bytes[1] = GPS->raw_message[55];
            GPS->n_vel_bytes[2] = GPS->raw_message[56];
            GPS->n_vel_bytes[3] = GPS->raw_message[57];
            iLong.bytes[0] = GPS->raw_message[54];
            iLong.bytes[1] = GPS->raw_message[55];
            iLong.bytes[2] = GPS->raw_message[56];
            iLong.bytes[3] = GPS->raw_message[57];
            GPS->n_vel = iLong.iLong;
            GPS->n_vel_rounded = (int16_t)((GPS->n_vel * 100) / 10000);
            GPS->n_vel_sfs1 = (double)(GPS->n_vel);
            GPS->n_vel_sfs2 = (double)GPS->n_vel;
            // Collect East Velocity
            GPS->e_vel_bytes[0] = GPS->raw_message[58];
            GPS->e_vel_bytes[1] = GPS->raw_message[59];
            GPS->e_vel_bytes[2] = GPS->raw_message[60];
            GPS->e_vel_bytes[3] = GPS->raw_message[61];
            iLong.bytes[0] = GPS->raw_message[58];
            iLong.bytes[1] = GPS->raw_message[59];
            iLong.bytes[2] = GPS->raw_message[60];
            iLong.bytes[3] = GPS->raw_message[61];
            GPS->e_vel = iLong.iLong;
            GPS->e_vel_rounded = (int16_t)((GPS->e_vel * 100) / 10000);
            // Collect Down Velocity
            GPS->d_vel_bytes[0] = GPS->raw_message[62];
            GPS->d_vel_bytes[1] = GPS->raw_message[63];
            GPS->d_vel_bytes[2] = GPS->raw_message[64];
            GPS->d_vel_bytes[3] = GPS->raw_message[65];
            iLong.bytes[0] = GPS->raw_message[62];
            iLong.bytes[1] = GPS->raw_message[63];
            iLong.bytes[2] = GPS->raw_message[64];
            iLong.bytes[3] = GPS->raw_message[65];
            GPS->d_vel = iLong.iLong;
            GPS->d_vel_rounded = (int16_t)((GPS->d_vel * 100) / 10000);

            rtU->vel[0] = CLAMP(((double)GPS->n_vel) * VEL_CALIBRATION, MIN_POS, MAX_POS);
            rtU->vel[1] = CLAMP(((double)GPS->e_vel) * VEL_CALIBRATION, MIN_POS, MAX_POS);
            rtU->vel[2] = CLAMP(((double)GPS->d_vel) * VEL_CALIBRATION, MIN_POS, MAX_POS);

            rtU->pos[0] = CLAMP(((double)GPS->latitude) * POS_DEG_CALIBRATION, MIN_VEL, MAX_VEL);
            rtU->pos[1] = CLAMP(((double)GPS->longitude) * POS_DEG_CALIBRATION, MIN_VEL, MAX_VEL);
            rtU->pos[2] = CLAMP(((double)GPS->height) * POS_H_CALIBRATION, MIN_VEL, MAX_VEL);

            SEND_GPS_VELOCITY(q_tx_can, GPS->n_vel_rounded, GPS->e_vel_rounded, GPS->d_vel_rounded, GPS->speed_rounded);
            SEND_GPS_COORDINATES(q_tx_can, GPS->lat_rounded, GPS->lon_rounded);
            SEND_GPS_POSITION(q_tx_can, 0, 0, 0, GPS->height_rounded);
        }

        // Collect Magnetic Declination
        GPS->mag_dec_bytes[0] = GPS->raw_message[94];
        GPS->mag_dec_bytes[1] = GPS->raw_message[95];
        iShort.bytes[0] = GPS->raw_message[94];
        iShort.bytes[1] = GPS->raw_message[95];
        GPS->mag_dec = iShort.iShort;

        return correctFix;
    }
    return false;
}