#ifndef LERP_LUT_H
#define LERP_LUT_H

/**
 * @file lerp_lut.h
 * @brief Linear interpolation lookup table (LUT).
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include <stddef.h>

typedef struct {
    float key;
    float value;
} lut_entry_t;

typedef struct {
    const lut_entry_t *entries;
    size_t size;
} lerp_lut_t;

#define LERP_LUT_INIT(name, _entries, _size) \
    const lerp_lut_t name = { \
        .entries = _entries, \
        .size    = _size, \
    }; \
    static_assert(sizeof(_entries) / sizeof(lut_entry_t) == _size, \
                  "Entries array size must match _size"); \
    static_assert(_size > 1, "LUT size must be greater than 1")

float lut_lookup(const lerp_lut_t *lut, float key);

#endif // LERP_LUT_H