/**
 * @file can_codec_tests.c
 * @brief Standalone tests for CAN codec helper functions.
 * @note does not run on the STM32 target, only on the host machine.
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../can_codec.h"

static void test_payload_helpers(void) {
    uint8_t bytes[8] = {
        0xAA,
        0xAA,
        0xAA,
        0xAA,
        0xAA,
        0xAA,
        0xAA,
        0xAA,
    };

    CAN_store_payload_u64(bytes, 0x1122334455667788ULL, 0);
    for (size_t i = 0; i < sizeof(bytes); i++) {
        assert(bytes[i] == 0xAA);
    }

    CAN_store_payload_u64(bytes, 0x1122334455667788ULL, 3);
    assert(bytes[0] == 0x88);
    assert(bytes[1] == 0x77);
    assert(bytes[2] == 0x66);
    assert(bytes[3] == 0xAA);
    assert(bytes[4] == 0xAA);
    assert(bytes[5] == 0xAA);
    assert(bytes[6] == 0xAA);
    assert(bytes[7] == 0xAA);

    uint64_t partial = CAN_load_payload_u64(bytes, 3);
    assert(partial == 0x667788ULL);

    CAN_store_payload_u64(bytes, 0x1122334455667788ULL, 8);
    assert(CAN_load_payload_u64(bytes, 8) == 0x1122334455667788ULL);

    memset(bytes, 0xAA, sizeof(bytes));
    assert(CAN_load_payload_u64(bytes, 0) == 0);
}

static void test_bswap_helper(void) {
    assert(CAN_apply_bswap(0x123456789ABCDEF0ULL, 0) == 0x123456789ABCDEF0ULL);
    assert(CAN_apply_bswap(0x1234ULL, 16) == 0x3412ULL);
    assert(CAN_apply_bswap(0x12345678ULL, 32) == 0x78563412ULL);
    assert(CAN_apply_bswap(0x0123456789ABCDEFULL, 64) == 0xEFCDAB8967452301ULL);
    assert(CAN_apply_bswap(0x123456789ABCDEF0ULL, 7) == 0x123456789ABCDEF0ULL);
}

static void test_raw_signal_helpers(void) {
    uint64_t payload = 0;
    payload = CAN_pack_raw_signal(payload, 0xAB, 0xFF, 0, 0);
    assert(payload == 0xAB);
    assert(CAN_unpack_raw_signal(payload, 0xFF, 0, 0) == 0xAB);

    payload = 0;
    payload = CAN_pack_raw_signal(payload, 0x5, 0xF, 12, 0);
    assert(payload == 0x5000);
    assert(CAN_unpack_raw_signal(payload, 0xF, 12, 0) == 0x5);

    payload = CAN_pack_raw_signal(payload, 0xFFFF, 0x3, 0, 0);
    assert((payload & 0x3) == 0x3);
    assert((payload & ~0x3ULL) == 0x5000);
    assert(CAN_unpack_raw_signal(payload, 0x3, 0, 0) == 0x3);

    payload = 0xF000000000000000ULL;
    payload = CAN_pack_raw_signal(payload, 0x12, 0xFF, 8, 0);
    assert((payload & 0xF000000000000000ULL) == 0xF000000000000000ULL);
    assert(CAN_unpack_raw_signal(payload, 0xFF, 8, 0) == 0x12);

    payload = CAN_pack_raw_signal(0, 0x1234, 0xFFFF, 0, 16);
    assert(payload == 0x3412);
    assert(CAN_unpack_raw_signal(payload, 0xFFFF, 0, 16) == 0x1234);

    payload = CAN_pack_raw_signal(0, 0x12345678, 0xFFFFFFFFULL, 8, 32);
    assert(payload == 0x7856341200ULL);
    assert(CAN_unpack_raw_signal(payload, 0xFFFFFFFFULL, 8, 32) == 0x12345678);

    payload = CAN_pack_raw_signal(0, 0x0123456789ABCDEFULL, UINT64_MAX, 0, 64);
    assert(payload == 0xEFCDAB8967452301ULL);
    assert(CAN_unpack_raw_signal(payload, UINT64_MAX, 0, 64) == 0x0123456789ABCDEFULL);
}

static void test_sign_extend_helper(void) {
    assert(CAN_sign_extend_raw(0, 0) == 0);

    assert(CAN_sign_extend_raw(0x7F, 8) == 127);
    assert(CAN_sign_extend_raw(0x80, 8) == -128);
    assert(CAN_sign_extend_raw(0xFF, 8) == -1);

    assert(CAN_sign_extend_raw(0x7FF, 12) == 2047);
    assert(CAN_sign_extend_raw(0x800, 12) == -2048);
    assert(CAN_sign_extend_raw(0xFFF, 12) == -1);

    assert(CAN_sign_extend_raw(0x7FFF, 16) == 32767);
    assert(CAN_sign_extend_raw(0x8000, 16) == -32768);
    assert(CAN_sign_extend_raw(0xFFFF, 16) == -1);

    assert(CAN_sign_extend_raw(0x7FFFFFFFFFFFFFFFULL, 64) == INT64_MAX);
    assert(CAN_sign_extend_raw(0x8000000000000000ULL, 64) == INT64_MIN);
}

static void test_float_helpers(void) {
    assert(CAN_float32_to_raw(1.0f) == 0x3F800000ULL);

    uint64_t raw_f32 = CAN_float32_to_raw(-12.5f);
    float value_f32  = CAN_raw_to_float32(raw_f32);
    assert(CAN_float32_to_raw(value_f32) == raw_f32);

    assert(CAN_float32_to_raw(CAN_raw_to_float32(0x40490FDBULL)) == 0x40490FDBULL);
}

int main(void) {
    test_payload_helpers();
    test_bswap_helper();
    test_raw_signal_helpers();
    test_sign_extend_helper();
    test_float_helpers();

    printf("can_codec_tests passed\n");
    return 0;
}
