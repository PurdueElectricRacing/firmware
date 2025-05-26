/**
 * @file gps.h
 * @author Chris McGalliard (cmcgalli@purdue.edu)
 * @brief
 * @version 0.1
 * @date 2022-12-28
 */

#include <stdint.h>
#include <stdbool.h>

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

// typedef struct
// {
//     vector_3d_t acceleration;
//     vector_3d_t gyroscope;
//     signed long messages_received;
//     uint8_t raw_message[100];

//     signed long g_speed;
//     int16_t g_speed_rounded;

//     signed long longitude;
//     int32_t lon_rounded;

//     signed long latitude;
//     int32_t lat_rounded;

//     signed long height;
//     int16_t height_rounded;

//     signed long n_vel;
//     int16_t n_vel_rounded;

//     signed long e_vel;
//     int16_t e_vel_rounded;

//     signed long d_vel;
//     int16_t d_vel_rounded;

//     signed long headVeh;
//     int16_t headVeh_rounded;

//     signed short mag_dec;

//     uint8_t fix_type;

//     unsigned long iTOW;
//     bool is_unique_iTOW;

//     uint8_t year; // Year (UTC) - (milenium truncated)
//     uint8_t month; // Month, range 1..12 (UTC)
//     uint8_t day;  // Day of month, range 1..31 (UTC)
//     uint8_t hour; // Hour of day, range 0..23 (UTC)
//     uint8_t minute; // Minute of hour, range 0..59 (UTC)
//     uint8_t second; // Second of minute, range 0..60 (UTC)
//     int16_t millisecond; // Millisecond of second (calculated from nanoseconds)
//     bool is_valid_date;
//     bool is_valid_time;
//     bool is_fully_resolved;
//     bool is_valid_mag;

//     uint8_t gyro_OK;

// } GPS_Handle_t; // GPS handle

// Bitmask for NAV_PVT_t.valid flags
typedef uint8_t gps_valid_flags_t;
#define GPS_VALID_DATE           ((gps_valid_flags_t)0x01) // Valid UTC Date
#define GPS_VALID_TIME           ((gps_valid_flags_t)0x02) // Valid UTC Time
#define GPS_VALID_FULLY_RESOLVED ((gps_valid_flags_t)0x04) // UTC fully resolved
#define GPS_VALID_MAG            ((gps_valid_flags_t)0x08) // Valid Magnetic Declination

// GPS Fix Types for NAV_PVT_t.fixType
typedef uint8_t gps_fix_type_t;
#define GPS_FIX_TYPE_NONE                ((gps_fix_type_t)0x00) // No fix
#define GPS_FIX_TYPE_DEAD_RECKONING      ((gps_fix_type_t)0x01) // Dead Reckoning only
#define GPS_FIX_TYPE_2D                  ((gps_fix_type_t)0x02) // 2D fix
#define GPS_FIX_TYPE_3D                  ((gps_fix_type_t)0x03) // 3D fix
#define GPS_FIX_TYPE_GNSS_DEAD_RECKONING ((gps_fix_type_t)0x04) // GNSS + Dead Reckoning
#define GPS_FIX_TYPE_TIME_ONLY           ((gps_fix_type_t)0x05) // Time only fix

typedef uint8_t gps_fix_status_t;
#define GPS_FIX_FLAG_GPS_FIX_OK         ((gps_fix_status_t)0x01) // GPS fix OK
#define GPS_FIX_FLAG_DIFF_SOLN          ((gps_fix_status_t)0x02) // Differential GPS fix
//TODO PSM flags

typedef struct {
    uint32_t iTOW;
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    gps_valid_flags_t valid; // GPS_VALID_*

    uint32_t timeAccuracy; // time accuracy in nanoseconds
    int32_t nano;          // nanoseconds of second

    gps_fix_type_t fixType;     // GPS_FIX_TYPE_*
    gps_fix_status_t fixStatus; // GPS_FIX_FLAG_*

    uint8_t numSV;               // Number of satellites used in Nav Solution
    int32_t longitude;           // Longitude in 1e-7 degrees
    int32_t latitude;            // Latitude in 1e-7 degrees
    int32_t height;              // Height above ellipsoid in mm
    int32_t heightMSL;           // Height above mean sea level in mm
    uint32_t horizontalAccuracy; // Horizontal accuracy estimate in mm
    uint32_t verticalAccuracy;   // Vertical accuracy estimate in mm

    int32_t velNorth;       // NED North Velocity in mm/s
    int32_t velEast;        // NED East Velocity in mm/s
    int32_t velDown;        // NED DownVelocity in mm/s
    int32_t groundSpeed;    // Ground Speed in mm/s
    int32_t headingMotion;  // Heading of motion
    uint32_t speedAccuracy; // Speed accuracy estimate in mm/s

    uint16_t positionDOP; // Position DOP (0.01)
    uint8_t reserved[6];

    int16_t headingVehicle; // Heading of vehicle in 1e-5 degrees

    int16_t magDec;  // Magnetic declination in 1e-2 degrees
    uint16_t magAcc; // Magnetic declination accuracy in 1e-2 degrees

} NAV_PVT_t;

#define GPS_RX_BUF_SIZE 100 // Size of GPS receive buffer

typedef struct {
    uint8_t gps_rx_buffer[GPS_RX_BUF_SIZE];  // Raw message as received from GPS
    NAV_PVT_t decoded_message; // Decoded message

    bool isValidDate;
    bool isValidTime;
    bool isFullyResolved;
    bool isValidMag;
} GPS_Handle_t;

// GPS Message Attributes
#define UBX_NAV_PVT_HEADER_B0 0xB5
#define UBX_NAV_PVT_HEADER_B1 0x62
#define UBX_NAV_PVT_CLASS     0x01
#define UBX_NAV_PVT_MSG_ID    0x07

// Reworked function here
//void GPS_Decode(GPS_Handle_t *GPS);



/**
 * @brief Function to Parse periodic GPS UBX message
 *
 * @param GPS Handle for GPS configuration
 * @param rtU Handle for SFS
 * @return true Parsing successful
 * @return false Parsing failed
 */
bool GPS_Parse(GPS_Handle_t *GPS);

#endif //_GPS_H