/**
 * @file ubx_common.h
 * @brief UBX common definitions and utility functions.
 *
 * ! ZED-F9P-Protocols-1.32.pdf @ 3.2
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#ifndef UBX_COMMON_H
#define UBX_COMMON_H

#include <stdint.h>
#include <stddef.h>

static constexpr size_t UBX_HEADER_LENGTH = 6;
static constexpr uint8_t UBX_SYNC_B0      = 0xB5;
static constexpr uint8_t UBX_SYNC_B1      = 0x62;
typedef struct {
    uint8_t sync_word[2];
    uint8_t message_class;
    uint8_t message_id;
    uint16_t payload_length;
} ubx_header_t;
static_assert(sizeof(ubx_header_t) == UBX_HEADER_LENGTH, "ubx_header_t size must be 6 bytes");

static constexpr size_t UBX_CHECKSUM_LENGTH = 2;
static constexpr size_t UBX_OVERHEAD        = UBX_HEADER_LENGTH + UBX_CHECKSUM_LENGTH;

static inline bool UBX_is_matching_header(
    const ubx_header_t *header,
    const ubx_header_t *expected
) {
    return (header->sync_word[0] == expected->sync_word[0])
        && (header->sync_word[1] == expected->sync_word[1])
        && (header->message_class == expected->message_class)
        && (header->message_id == expected->message_id)
        && (header->payload_length == expected->payload_length);
}

static inline bool UBX_is_valid_checksum(
    const uint8_t *rx_buffer,
    const size_t payload_length
) {
    const size_t checksum_offset = UBX_HEADER_LENGTH + payload_length;

    // the checksum does not include the sync word
    // algorithm defined in ZED-F9P-Protocols-1.32.pdf @ 3.4
    uint8_t ck_a = 0;
    uint8_t ck_b = 0;
    for (size_t i = 2; i < checksum_offset; i++) {
        ck_a = ck_a + rx_buffer[i];
        ck_b = ck_b + ck_a;
    }

    uint8_t checksum[2] = {
        rx_buffer[checksum_offset + 0],
        rx_buffer[checksum_offset + 1]
    };

    return (ck_a == checksum[0]) && (ck_b == checksum[1]);
}

#endif // UBX_COMMON_H