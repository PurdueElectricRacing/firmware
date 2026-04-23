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

// UBX NAV-RELPOSNED message header
static constexpr uint8_t NAV_RELPOSNED_HEADER_B0 = 0xB5; // UBX message header sync byte 0
static constexpr uint8_t NAV_RELPOSNED_HEADER_B1 = 0x62; // UBX message header sync byte 1
static constexpr uint8_t NAV_RELPOSNED_CLASS     = 0x01; // UBX message class for NAV-RELPOSNED
static constexpr uint8_t NAV_RELPOSNED_MSG_ID    = 0x3C; // UBX message ID for NAV-RELPOSNED

static constexpr size_t NAV_RELPOSNED_TOTAL_LENGTH = 48;

typedef enum : uint32_t {
    NAV_RELPOSNED_FLAGS_GNSS_FIX_OK    = 0x1, // GNSS fix OK
    NAV_RELPOSNED_FLAGS_DIFF_SOLN      = 0x2, // Differential solution
    NAV_RELPOSNED_FLAGS_REL_POS_VALID  = 0x4,

    NAV_RELPOSNED_FLAGS_CARR_SOLN_MASK  = 0x18,
    NAV_RELPOSNED_FLAGS_CARR_SOLN_NONE  = 0x0,
    NAV_RELPOSNED_FLAGS_CARR_SOLN_FLOAT = 0x8,
    NAV_RELPOSNED_FLAGS_CARR_SOLN_FIXED = 0x10,

    NAV_RELPOSNED_FLAGS_IS_MOVING    = 0x20,
    NAV_RELPOSNED_FLAGS_REF_POS_MISS = 0x40,
    NAV_RELPOSNED_FLAGS_REF_OBS_MISS = 0x80
} NAV_RELPOSNED_flags_t;

typedef struct {
    uint8_t version;
    uint8_t reserved0;
    uint16_t refStationId;
    uint32_t iTOW;

    int32_t relPosN;
    int32_t relPosE;
    int32_t relPosD;

    int8_t relPosHPN;
    int8_t relPosHPE;
    int8_t relPosHPD;
    uint8_t reserved1;

    uint32_t accN;
    uint32_t accE;
    uint32_t accD;

    NAV_RELPOSNED_flags_t flags;
} NAV_RELPOSNED_data_t;

static_assert(sizeof(NAV_RELPOSNED_data_t) == 40, "NAV_RELPOSNED_data_t size must be 40 bytes");
static_assert(offsetof(NAV_RELPOSNED_data_t, version) == 0);
static_assert(offsetof(NAV_RELPOSNED_data_t, refStationId) == 2);
static_assert(offsetof(NAV_RELPOSNED_data_t, iTOW) == 4);
static_assert(offsetof(NAV_RELPOSNED_data_t, relPosN) == 8);
static_assert(offsetof(NAV_RELPOSNED_data_t, relPosE) == 12);
static_assert(offsetof(NAV_RELPOSNED_data_t, relPosD) == 16);
static_assert(offsetof(NAV_RELPOSNED_data_t, relPosHPN) == 20);
static_assert(offsetof(NAV_RELPOSNED_data_t, accN) == 24);
static_assert(offsetof(NAV_RELPOSNED_data_t, accE) == 28);
static_assert(offsetof(NAV_RELPOSNED_data_t, accD) == 32);
static_assert(offsetof(NAV_RELPOSNED_data_t, flags) == 36);

void NAV_RELPOSNED_decode(NAV_RELPOSNED_data_t *relposned, const uint8_t *rx_buffer);

#endif // NAV_RELPOSNED_H