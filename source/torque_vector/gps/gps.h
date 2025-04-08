// /**
//  * @file gps.h
//  * @author Chris McGalliard (cmcgalli@purdue.edu)
//  * @brief
//  * @version 0.1
//  * @date 2022-12-28
//  *
//  *
//  */

#include <stdint.h>
#include "bsxlite_interface.h"
#include "can_parse.h"
// #include "SFS.h"
#include "common/common_defs/common_defs.h"

#ifndef _GPS_H
#define _GPS_H

union i_Long
{
    uint8_t bytes[4];
    signed long iLong;
};

union u_Long
{
    uint8_t bytes[4];
    unsigned long uLong;
};

union i_Short
{
    uint8_t bytes[2];
    signed short iShort;
};

typedef struct
{
    vector_3d_t acceleration;
    vector_3d_t gyroscope;
    signed long messages_received;
    uint8_t raw_message[100];

    signed long g_speed;
    int16_t g_speed_rounded;

    signed long longitude;
    int32_t lon_rounded;

    signed long latitude;
    int32_t lat_rounded;

    signed long height;
    int16_t height_rounded;

    signed long n_vel;
    int16_t n_vel_rounded;

    signed long e_vel;
    int16_t e_vel_rounded;

    signed long d_vel;
    int16_t d_vel_rounded;

    signed long headVeh;
    int16_t headVeh_rounded;

    signed short mag_dec;

    uint8_t fix_type;

    unsigned long iTOW;
    bool unique_iTOW;

    uint8_t year; // Year (UTC) - (milenium truncated)
    uint8_t month; // Month, range 1..12 (UTC)
    uint8_t day;  // Day of month, range 1..31 (UTC)
    uint8_t hour; // Hour of day, range 0..23 (UTC)
    uint8_t minute; // Minute of hour, range 0..59 (UTC)
    uint8_t second; // Second of minute, range 0..60 (UTC)
    int16_t millisecond; // Millisecond of second (calculated from nanoseconds)
    bool is_valid_date;
    bool is_valid_time;
    bool is_fully_resolved;
    bool is_valid_mag;

    uint8_t gyro_OK;

} GPS_Handle_t; // GPS handle

// GPS Message Attributes
#define UBX_NAV_PVT_HEADER_B0 0xB5
#define UBX_NAV_PVT_HEADER_B1 0x62
#define UBX_NAV_PVT_CLASS     0x01
#define UBX_NAV_PVT_MSG_ID    0x07

#define GPS_FIX_NONE    0
#define GPS_FIX_DEAD_RECKONING   1
#define GPS_FIX_2D  2
#define GPS_FIX_3D  3
#define GPS_FIX_GNSS_DEAD_RECKONING 4
#define GPS_FIX_TIME_ONLY       5

           
#define GPS_VALID_DATE           0x01    // Valid UTC Date
#define GPS_VALID_TIME           0x02    // Valid UTC Time
#define GPS_VALID_FULLY_RESOLVED 0x04    // UTC fully resolved
#define GPS_VALID_MAG           0x08     // Valid Magnetic Declination

/**
 * @brief Function to Parse periodic GPS UBX message
 *
 * @param GPS Handle for GPS configuration
 * @param rtU Handle for SFS
 * @return true Parsing successful
 * @return false Parsing failed
 */
bool parseVelocity(GPS_Handle_t *GPS);

#endif //_GPS_H