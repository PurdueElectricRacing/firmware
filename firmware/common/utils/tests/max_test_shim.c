/**
 * @file max_test_shim.c
 * @brief C-compilation environment for testing C23 max.h macros.
 */

#include "max_test_shim.h"
#include "max.h"

signed int test_max2_signed(signed int a, signed int b) {
    return max2_signed(a, b);
}

unsigned int test_max2_unsigned(unsigned int a, unsigned int b) {
    return max2_unsigned(a, b);
}

float test_max2_float(float a, float b) {
    return max2_float(a, b);
}

int test_MAXOF2_int(int a, int b) {
    return MAXOF2(a, b);
}

unsigned int test_MAXOF2_uint(unsigned int a, unsigned int b) {
    return MAXOF2(a, b);
}

float test_MAXOF2_float(float a, float b) {
    return MAXOF2(a, b);
}

int8_t test_MAXOF2_int8(int8_t a, int8_t b) {
    static_assert(_Generic((typeof(MAXOF2(a, b)))0, int8_t: 1, default: 0),
                  "MAXOF2(int8_t, int8_t) must retain int8_t type");
    return MAXOF2(a, b);
}

uint16_t test_MAXOF2_uint16(uint16_t a, uint16_t b) {
    static_assert(_Generic((typeof(MAXOF2(a, b)))0, uint16_t: 1, default: 0),
                  "MAXOF2(uint16_t, uint16_t) must retain uint16_t type");
    return MAXOF2(a, b);
}

int test_MAXOF3_int(int a, int b, int c) {
    return MAXOF3(a, b, c);
}

float test_MAXOF3_float(float a, float b, float c) {
    return MAXOF3(a, b, c);
}

int test_MAXOF4_int(int a, int b, int c, int d) {
    return MAXOF4(a, b, c, d);
}

int test_MAXOF_4arg(int a, int b, int c, int d) {
    return MAXOF(a, b, c, d);
}

int test_MAXOF_3arg(int a, int b, int c) {
    return MAXOF(a, b, c);
}

int test_MAXOF_2arg(int a, int b) {
    return MAXOF(a, b);
}
