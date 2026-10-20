/**
 * @file nav_relposned.h
 * @brief UBX NAV-RELPOSNED message definition and decoder function.
 * 
 * ! ZED-F9P-Protocols-1.32.pdf @ 3.15.14
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#ifndef NAV_RELPOSNED_H
#define NAV_RELPOSNED_H

#include "ubx_common.h"
#include <string.h>

static constexpr ubx_header_t NAV_RELPOSNED_HEADER = {
    .sync_word      = {UBX_SYNC_B0, UBX_SYNC_B1},
    .message_class  = 0x01,
    .message_id     = 0x3C,
    .payload_length = 64
};

static constexpr size_t NAV_RELPOSNED_BUFFER_SIZE = UBX_OVERHEAD + NAV_RELPOSNED_HEADER.payload_length;

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
} NAV_RELPOSNED_payload_t;

static_assert(sizeof(NAV_RELPOSNED_payload_t) == NAV_RELPOSNED_HEADER.payload_length, "NAV_RELPOSNED_payload_t size must be 64 bytes");
static_assert(offsetof(NAV_RELPOSNED_payload_t, version) == 0);
static_assert(offsetof(NAV_RELPOSNED_payload_t, reserved0) == 1);
static_assert(offsetof(NAV_RELPOSNED_payload_t, refStationId) == 2);
static_assert(offsetof(NAV_RELPOSNED_payload_t, iTOW) == 4);
static_assert(offsetof(NAV_RELPOSNED_payload_t, relPosN) == 8);
static_assert(offsetof(NAV_RELPOSNED_payload_t, relPosE) == 12);
static_assert(offsetof(NAV_RELPOSNED_payload_t, relPosD) == 16);
static_assert(offsetof(NAV_RELPOSNED_payload_t, relPosLength) == 20);
static_assert(offsetof(NAV_RELPOSNED_payload_t, relPosHeading) == 24);
static_assert(offsetof(NAV_RELPOSNED_payload_t, reserved1) == 28);
static_assert(offsetof(NAV_RELPOSNED_payload_t, relPosHPN) == 32);
static_assert(offsetof(NAV_RELPOSNED_payload_t, relPosHPE) == 33);
static_assert(offsetof(NAV_RELPOSNED_payload_t, relPosHPD) == 34);
static_assert(offsetof(NAV_RELPOSNED_payload_t, relPosHPLength) == 35);
static_assert(offsetof(NAV_RELPOSNED_payload_t, accN) == 36);
static_assert(offsetof(NAV_RELPOSNED_payload_t, accE) == 40);
static_assert(offsetof(NAV_RELPOSNED_payload_t, accD) == 44);
static_assert(offsetof(NAV_RELPOSNED_payload_t, accLength) == 48);
static_assert(offsetof(NAV_RELPOSNED_payload_t, accHeading) == 52);
static_assert(offsetof(NAV_RELPOSNED_payload_t, reserved2) == 56);
static_assert(offsetof(NAV_RELPOSNED_payload_t, flags) == 60);

static inline bool NAV_RELPOSNED_decode(
    const volatile uint8_t *rx_buffer,
    NAV_RELPOSNED_payload_t *nav_relposned
) {
    ubx_header_t header; // create copy to avoid alignment issues
    memcpy(&header, (const void *)rx_buffer, sizeof(ubx_header_t));

    if (!UBX_is_matching_header(&header, &NAV_RELPOSNED_HEADER)) {
        return false;
    }

    if (!UBX_is_valid_checksum((const uint8_t *)rx_buffer, NAV_RELPOSNED_HEADER.payload_length)) {
        return false;
    }

    memcpy(nav_relposned, (const void *)(rx_buffer + UBX_HEADER_LENGTH), sizeof(NAV_RELPOSNED_payload_t));

    return true;
}

#endif // NAV_RELPOSNED_H
