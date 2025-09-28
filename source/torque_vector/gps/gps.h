/**
 * @file gps.h
 * @author Irving Wang (wang5952@purdue.edu)
 * @brief
 * @version 0.1
 * @date 2025-06-14
 */

#include <stdint.h>
#include <stdbool.h>

#ifndef _GPS_H
#define _GPS_H

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

// GPS RX buffer size
#define GPS_RX_BUF_SIZE 100 // The total message length is 100 bytes

// GPS UBX NAV-PVT message header
#define UBX_NAV_PVT_HEADER_B0 0xB5 // UBX message header sync byte 0
#define UBX_NAV_PVT_HEADER_B1 0x62 // UBX message header sync byte 1
#define UBX_NAV_PVT_CLASS     0x01 // UBX message class for NAV-PVT
#define UBX_NAV_PVT_MSG_ID    0x07 // UBX message ID for NAV-PVT

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
#define GPS_FIX_TYPE_GNSS_2D             ((gps_fix_type_t)0x02) // 2D fix
#define GPS_FIX_TYPE_GNSS_3D             ((gps_fix_type_t)0x03) // 3D fix
#define GPS_FIX_TYPE_GNSS_DEAD_RECKONING ((gps_fix_type_t)0x04) // GNSS + Dead Reckoning
#define GPS_FIX_TYPE_TIME_ONLY           ((gps_fix_type_t)0x05) // Time only fix

// GPS Fix Status Flags for NAV_PVT_t.fixStatus
typedef uint8_t gps_flags1_t;
#define GPS_FLAG1_GNSS_FIX_OK ((gps_flags1_t)0x01) // GPS fix OK
#define GPS_FLAG1_DIFF_SOLN   ((gps_flags1_t)0x02) // Differential GPS fix

#define GPS_FLAG1_PSM_MASK            ((gps_flags1_t)0x1C) // PSM (Power Save Mode) mask
#define GPS_FLAG1_PSM_OFF             ((gps_flags1_t)0x00) // PSM off
#define GPS_FLAG1_PSM_ENABLED         ((gps_flags1_t)0x04) // PSM on
#define GPS_FLAG1_PSM_ACQUIRED        ((gps_flags1_t)0x08) // PSM active
#define GPS_FLAG1_PSM_TRACKING        ((gps_flags1_t)0x0C) // PSM tracking
#define GPS_FLAG1_PSM_POWER_OPTIMIZED ((gps_flags1_t)0x10) // PSM power optimized
#define GPS_FLAG1_PSM_INACTIVE        ((gps_flags1_t)0x14) // PSM inactive

#define GPS_FLAG1_HEADING_VALID       ((gps_flags1_t)0x20) // Heading is valid
#define GPS_FLAG1_CARRIER_PHASE_MASK  ((gps_flags1_t)0xC0) // Carrier phase is valid
#define GPS_FLAG1_CARRIER_PHASE_NONE  ((gps_flags1_t)0x00) // No carrier phase
#define GPS_FLAG1_CARRIER_PHASE_FLOAT ((gps_flags1_t)0x40) // Carrier phase float
#define GPS_FLAG1_CARRIER_PHASE_FIXED ((gps_flags1_t)0x80) // Carrier phase fixed

typedef uint8_t gps_flags2_t;
#define GPS_FLAGS2_CONFIRMED_AVAILABLE ((gps_flags2_t)0x20) // UTC date and time are confirmed available
#define GPS_FLAGS2_CONFIRMED_DATE      ((gps_flags2_t)0x40) // UTC date is confirmed
#define GPS_FLAGS2_CONFIRMED_TIME      ((gps_flags2_t)0x80) // UTC time is confirmed

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

    gps_fix_type_t fixType; // GPS_FIX_TYPE_*
    gps_flags1_t flags1;    // GPS_FIX_FLAG_*
    gps_flags2_t flags2;    // GPS_FLAGS2_*

    uint8_t numSatellites;       // Number of satellites used in Nav Solution
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

    uint16_t positionDOP; // Position Dilution of Precision (0.01 scale)
    uint8_t reserved[6];

    int32_t headingVehicle; // Heading of vehicle in 1e-5 degrees

    int16_t magneticDec;  // Magnetic declination in 1e-2 degrees
    uint16_t magneticAcc; // Magnetic declination accuracy in 1e-2 degrees
} NAV_PVT_t;

typedef struct {
    uint8_t gps_rx_buffer[GPS_RX_BUF_SIZE];  // Raw message as received from GPS
    NAV_PVT_t data; // Decoded message

    bool isValidDate;
    bool isValidTime;
    bool isFullyResolved;
    bool isValidMag;
    bool hasGNSSFix;
} GPS_Handle_t;

static inline int32_t bytes_to_int32(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3) {
    uint32_t value = (uint32_t)b0 |
                     ((uint32_t)b1 << 8) |
                     ((uint32_t)b2 << 16) |
                     ((uint32_t)b3 << 24);
    return (int32_t)value; // Cast to signed int32_t
}

static inline uint32_t bytes_to_uint32(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3) {
    return (uint32_t)b0 |
            ((uint32_t)b1 << 8) |
            ((uint32_t)b2 << 16) |
            ((uint32_t)b3 << 24);
}

static inline uint16_t bytes_to_uint16(uint8_t b0, uint8_t b1) {
    return (uint16_t)b0 | ((uint16_t)b1 << 8);
}

static inline int16_t bytes_to_int16(uint8_t b0, uint8_t b1) {
    uint16_t value = (uint16_t)b0 | ((uint16_t)b1 << 8);
    return (int16_t)value; // Cast to signed int16_t
}

static inline bool is_valid_header(uint8_t sync0, uint8_t sync1, uint8_t class_id, uint8_t msg_id) {
    return (sync0 == UBX_NAV_PVT_HEADER_B0) &&
           (sync1 == UBX_NAV_PVT_HEADER_B1) &&
           (class_id == UBX_NAV_PVT_CLASS) &&
           (msg_id == UBX_NAV_PVT_MSG_ID);
}

/**
 * @brief Function to decode periodic GPS UBX message
 *
 * @param GPS Handle for GPS configuration and data
 * @return true Parsing successful
 * @return false Parsing failed
 */
bool GPS_Decode(GPS_Handle_t *gps);

#endif //_GPS_H