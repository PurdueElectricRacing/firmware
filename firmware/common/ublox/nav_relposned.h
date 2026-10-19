#ifndef NAV_RELPOSNED_H
#define NAV_RELPOSNED_H

/**
 * @file nav_relposned.h
 * @brief UBX NAV-RELPOSNED message definition and decoder function.
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include <stdint.h>
#include <stddef.h>

// docs/datasheets/ZED-F9P-Protocols-1.32.pdf @ 3.15.14
static constexpr size_t NAV_RELPOSNED_HEADER_LENGTH = 6;
static constexpr size_t NAV_RELPOSNED_PAYLOAD_LENGTH = 64;
static constexpr size_t NAV_RELPOSNED_CHECKSUM_LENGTH = 2;
static constexpr size_t NAV_RELPOSNED_TOTAL_LENGTH =
    NAV_RELPOSNED_HEADER_LENGTH + NAV_RELPOSNED_PAYLOAD_LENGTH + NAV_RELPOSNED_CHECKSUM_LENGTH;

// UBX NAV-RELPOSNED message header
static constexpr uint8_t NAV_RELPOSNED_HEADER_B0 = 0xB5; // UBX message header sync byte 0
static constexpr uint8_t NAV_RELPOSNED_HEADER_B1 = 0x62; // UBX message header sync byte 1
static constexpr uint8_t NAV_RELPOSNED_CLASS     = 0x01; // UBX message class for NAV-RELPOSNED
static constexpr uint8_t NAV_RELPOSNED_MSG_ID    = 0x3C; // UBX message ID for NAV-RELPOSNED

typedef enum : uint32_t {
    NAV_RELPOSNED_FLAGS_GNSS_FIX_OK   = 0x1, // GNSS fix OK
    NAV_RELPOSNED_FLAGS_DIFF_SOLN     = 0x2, // Differential solution
    NAV_RELPOSNED_FLAGS_REL_POS_VALID = 0x4,

    NAV_RELPOSNED_FLAGS_CARR_SOLN_MASK  = 0x18,
    NAV_RELPOSNED_FLAGS_CARR_SOLN_NONE  = 0x0,
    NAV_RELPOSNED_FLAGS_CARR_SOLN_FLOAT = 0x8,
    NAV_RELPOSNED_FLAGS_CARR_SOLN_FIXED = 0x10,

    NAV_RELPOSNED_FLAGS_IS_MOVING     = 0x20,
    NAV_RELPOSNED_FLAGS_REF_POS_MISS  = 0x40,
    NAV_RELPOSNED_FLAGS_REF_OBS_MISS  = 0x80,
    NAV_RELPOSNED_FLAGS_HEADING_VALID = 0x100,
    NAV_RELPOSNED_FLAGS_NORMALIZED    = 0x200,
} NAV_RELPOSNED_flags_t;

typedef struct {
    uint8_t version;
    uint8_t reserved0;
    uint16_t refStationId;
    uint32_t iTOW;

    int32_t relPosN;
    int32_t relPosE;
    int32_t relPosD;
    int32_t relPosLength;
    int32_t relPosHeading;
    uint8_t reserved1[4];

    int8_t relPosHPN;
    int8_t relPosHPE;
    int8_t relPosHPD;
    int8_t relPosHPLength;

    uint32_t accN;
    uint32_t accE;
    uint32_t accD;
    uint32_t accLength;
    uint32_t accHeading;
    uint8_t reserved2[4];

    NAV_RELPOSNED_flags_t flags;
} NAV_RELPOSNED_data_t;

static_assert(sizeof(NAV_RELPOSNED_data_t) == NAV_RELPOSNED_PAYLOAD_LENGTH, "NAV_RELPOSNED_data_t size must be 64 bytes");
static_assert(offsetof(NAV_RELPOSNED_data_t, version) == 0);
static_assert(offsetof(NAV_RELPOSNED_data_t, reserved0) == 1);
static_assert(offsetof(NAV_RELPOSNED_data_t, refStationId) == 2);
static_assert(offsetof(NAV_RELPOSNED_data_t, iTOW) == 4);
static_assert(offsetof(NAV_RELPOSNED_data_t, relPosN) == 8);
static_assert(offsetof(NAV_RELPOSNED_data_t, relPosE) == 12);
static_assert(offsetof(NAV_RELPOSNED_data_t, relPosD) == 16);
static_assert(offsetof(NAV_RELPOSNED_data_t, relPosLength) == 20);
static_assert(offsetof(NAV_RELPOSNED_data_t, relPosHeading) == 24);
static_assert(offsetof(NAV_RELPOSNED_data_t, reserved1) == 28);
static_assert(offsetof(NAV_RELPOSNED_data_t, relPosHPN) == 32);
static_assert(offsetof(NAV_RELPOSNED_data_t, relPosHPE) == 33);
static_assert(offsetof(NAV_RELPOSNED_data_t, relPosHPD) == 34);
static_assert(offsetof(NAV_RELPOSNED_data_t, relPosHPLength) == 35);
static_assert(offsetof(NAV_RELPOSNED_data_t, accN) == 36);
static_assert(offsetof(NAV_RELPOSNED_data_t, accE) == 40);
static_assert(offsetof(NAV_RELPOSNED_data_t, accD) == 44);
static_assert(offsetof(NAV_RELPOSNED_data_t, accLength) == 48);
static_assert(offsetof(NAV_RELPOSNED_data_t, accHeading) == 52);
static_assert(offsetof(NAV_RELPOSNED_data_t, reserved2) == 56);
static_assert(offsetof(NAV_RELPOSNED_data_t, flags) == 60);

void NAV_RELPOSNED_decode(NAV_RELPOSNED_data_t *relposned, const volatile uint8_t *rx_buffer);

#endif // NAV_RELPOSNED_H