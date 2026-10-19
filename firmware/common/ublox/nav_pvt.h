#ifndef NAV_PVT_H
#define NAV_PVT_H

/**
 * @file nav_pvt.h
 * @brief UBX NAV-PVT message definition and decoder function.
 *
 * ! docs/datasheets/ZED-F9P-Protocols-1.32.pdf @ 3.15.13
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include <stdint.h>
#include <stddef.h>

// lengths
static constexpr size_t NAV_PVT_HEADER_LENGTH  = 6;
static constexpr size_t NAV_PVT_PAYLOAD_LENGTH = 92;
static constexpr size_t NAV_PVT_CRC_LENGTH     = 2;
static constexpr size_t NAV_PVT_TOTAL_LENGTH =
    NAV_PVT_HEADER_LENGTH + NAV_PVT_PAYLOAD_LENGTH + NAV_PVT_CRC_LENGTH;

// UBX NAV-PVT message header
static constexpr uint8_t NAV_PVT_HEADER_B0 = 0xB5; // UBX message header sync byte 0
static constexpr uint8_t NAV_PVT_HEADER_B1 = 0x62; // UBX message header sync byte 1
static constexpr uint8_t NAV_PVT_CLASS     = 0x01; // UBX message class for NAV-PVT
static constexpr uint8_t NAV_PVT_MSG_ID    = 0x07; // UBX message ID for NAV-PVT

typedef enum : uint8_t {
    GPS_VALID_DATE           = 0x01, // Valid UTC Date
    GPS_VALID_TIME           = 0x02, // Valid UTC Time
    GPS_VALID_FULLY_RESOLVED = 0x04, // UTC fully resolved
    GPS_VALID_MAG            = 0x08  // Valid Magnetic Declination
} gps_valid_flags_t;

typedef enum : uint8_t {
    GPS_FIX_TYPE_NONE                = 0x00, // No fix
    GPS_FIX_TYPE_DEAD_RECKONING      = 0x01, // Dead Reckoning only
    GPS_FIX_TYPE_GNSS_2D             = 0x02, // 2D fix
    GPS_FIX_TYPE_GNSS_3D             = 0x03, // 3D fix
    GPS_FIX_TYPE_GNSS_DEAD_RECKONING = 0x04, // GNSS + Dead Reckoning
    GPS_FIX_TYPE_TIME_ONLY           = 0x05  // Time only fix
} gps_fix_type_t;

typedef enum : uint8_t {
    GPS_FLAG1_GNSS_FIX_OK = 0x01, // GNSS fix OK
    GPS_FLAG1_DIFF_SOLN   = 0x02, // Differential GPS fix

    GPS_FLAG1_PSM_MASK            = 0x1C, // PSM (Power Save Mode) mask
    GPS_FLAG1_PSM_OFF             = 0x00, // PSM off
    GPS_FLAG1_PSM_ENABLED         = 0x04, // PSM on
    GPS_FLAG1_PSM_ACQUIRED        = 0x08, // PSM active
    GPS_FLAG1_PSM_TRACKING        = 0x0C, // PSM tracking
    GPS_FLAG1_PSM_POWER_OPTIMIZED = 0x10, // PSM power optimized
    GPS_FLAG1_PSM_INACTIVE        = 0x14, // PSM inactive

    GPS_FLAG1_HEADING_VALID       = 0x20, // Heading is valid
    GPS_FLAG1_CARRIER_PHASE_MASK  = 0xC0, // Carrier phase is valid
    GPS_FLAG1_CARRIER_PHASE_NONE  = 0x00, // No carrier phase
    GPS_FLAG1_CARRIER_PHASE_FLOAT = 0x40, // Carrier phase float
    GPS_FLAG1_CARRIER_PHASE_FIXED = 0x80  // Carrier phase fixed
} gps_flags1_t;

typedef enum : uint8_t {
    GPS_FLAGS2_CONFIRMED_AVAILABLE = 0x20, // UTC date and time are confirmed available
    GPS_FLAGS2_CONFIRMED_DATE      = 0x40, // UTC date is confirmed
    GPS_FLAGS2_CONFIRMED_TIME      = 0x80  // UTC time is confirmed
} gps_flags2_t;

typedef enum : uint16_t {
    GPS_FLAGS3_INVALID_LLH = 0x1,

    GPS_FLAGS3_LAST_CORRECTION_AGE_MASK   = 0x1E,
    GPS_FLAGS3_CORRECTION_AGE_UNAVAILABLE = 0x0,
    GPS_FLAGS3_CORRECTION_AGE_0_TO_1_S    = 0x2,
    GPS_FLAGS3_CORRECTION_AGE_1_TO_2_S    = 0x4,
    GPS_FLAGS3_CORRECTION_AGE_2_TO_5_S    = 0x6,
    GPS_FLAGS3_CORRECTION_AGE_5_TO_10_S   = 0x8,
    GPS_FLAGS3_CORRECTION_AGE_10_TO_15_S  = 0xA,
    GPS_FLAGS3_CORRECTION_AGE_15_TO_20_S  = 0xC,
    GPS_FLAGS3_CORRECTION_AGE_20_TO_30_S  = 0xE,
    GPS_FLAGS3_CORRECTION_AGE_30_TO_45_S  = 0x10,
    GPS_FLAGS3_CORRECTION_AGE_45_TO_60_S  = 0x12,
    GPS_FLAGS3_CORRECTION_AGE_60_TO_90_S  = 0x14,
    GPS_FLAGS3_CORRECTION_AGE_90_TO_120_S = 0x16,
    GPS_FLAGS3_CORRECTION_AGE_GE_120_S   = 0x18,
} gps_flags3_t;

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

    int32_t velNorth;         // NED North Velocity in mm/s
    int32_t velEast;          // NED East Velocity in mm/s
    int32_t velDown;          // NED DownVelocity in mm/s
    int32_t groundSpeed;      // Ground Speed in mm/s
    int32_t headingMotion;    // Heading of motion
    uint32_t speedAccuracy;   // Speed accuracy estimate in mm/s
    uint32_t headingAccuracy; // Heading accuracy estimate in 1e-5 degrees

    uint16_t positionDOP; // Position Dilution of Precision (0.01 scale)
    gps_flags3_t flags3;  // Additional flags
    uint8_t reserved[4];

    int32_t headingVehicle; // Heading of vehicle in 1e-5 degrees

    int16_t magneticDec;  // Magnetic declination in 1e-2 degrees
    uint16_t magneticAcc; // Magnetic declination accuracy in 1e-2 degrees
} NAV_PVT_data_t;

static_assert(sizeof(NAV_PVT_data_t) == NAV_PVT_PAYLOAD_LENGTH, "NAV_PVT_data_t size must be 92 bytes");

static_assert(offsetof(NAV_PVT_data_t, iTOW) == 0);
static_assert(offsetof(NAV_PVT_data_t, year) == 4);
static_assert(offsetof(NAV_PVT_data_t, month) == 6);
static_assert(offsetof(NAV_PVT_data_t, day) == 7);
static_assert(offsetof(NAV_PVT_data_t, hour) == 8);
static_assert(offsetof(NAV_PVT_data_t, minute) == 9);
static_assert(offsetof(NAV_PVT_data_t, second) == 10);
static_assert(offsetof(NAV_PVT_data_t, valid) == 11);
static_assert(offsetof(NAV_PVT_data_t, timeAccuracy) == 12);
static_assert(offsetof(NAV_PVT_data_t, nano) == 16);

static_assert(offsetof(NAV_PVT_data_t, fixType) == 20);
static_assert(offsetof(NAV_PVT_data_t, flags1) == 21);
static_assert(offsetof(NAV_PVT_data_t, flags2) == 22);
static_assert(offsetof(NAV_PVT_data_t, numSatellites) == 23);

static_assert(offsetof(NAV_PVT_data_t, longitude) == 24);
static_assert(offsetof(NAV_PVT_data_t, latitude) == 28);
static_assert(offsetof(NAV_PVT_data_t, height) == 32);
static_assert(offsetof(NAV_PVT_data_t, heightMSL) == 36);
static_assert(offsetof(NAV_PVT_data_t, horizontalAccuracy) == 40);
static_assert(offsetof(NAV_PVT_data_t, verticalAccuracy) == 44);

static_assert(offsetof(NAV_PVT_data_t, velNorth) == 48);
static_assert(offsetof(NAV_PVT_data_t, velEast) == 52);
static_assert(offsetof(NAV_PVT_data_t, velDown) == 56);
static_assert(offsetof(NAV_PVT_data_t, groundSpeed) == 60);
static_assert(offsetof(NAV_PVT_data_t, headingMotion) == 64);
static_assert(offsetof(NAV_PVT_data_t, speedAccuracy) == 68);
static_assert(offsetof(NAV_PVT_data_t, headingAccuracy) == 72);

static_assert(offsetof(NAV_PVT_data_t, positionDOP) == 76);
static_assert(offsetof(NAV_PVT_data_t, flags3) == 78);
static_assert(offsetof(NAV_PVT_data_t, reserved) == 80);
static_assert(offsetof(NAV_PVT_data_t, headingVehicle) == 84);
static_assert(offsetof(NAV_PVT_data_t, magneticDec) == 88);
static_assert(offsetof(NAV_PVT_data_t, magneticAcc) == 90);

void NAV_PVT_decode(NAV_PVT_data_t *nav_pvt, const volatile uint8_t *rx_buffer);

#endif // NAV_PVT_H