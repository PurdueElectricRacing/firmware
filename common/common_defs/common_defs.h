/**
 * @file common_defs.h
 * @author Adam Busch (busch8@purdue.edu)
 * @brief Common defs for the entire firmware repository. Dont let this get too out of control please.
 * @version 0.1
 * @date 2022-01-26
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef COMMON_DEFS_H
#define COMMON_DEFS_H

#include <stdint.h>

/* Math Functions */
static inline int32_t clamp_i(int32_t x, int32_t min, int32_t max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}
static inline float clamp_f(float x, float min, float max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}
#define CLAMP(x, min, max) _Generic((x), \
    float: clamp_f, \
    double: clamp_f, \
    default: clamp_i \
)((x), (min), (max))

// Base-2 logarithm that rounds down
static inline uint32_t LOG2_DOWN(uint32_t x) {
    return 31U - (uint32_t)__builtin_clz(x);
}

// Base-2 logarithm that rounds up
static inline uint32_t LOG2_UP(uint32_t x) {
    return LOG2_DOWN(x - 1) + 1;
}

static inline uint32_t ROUNDDOWN(uint32_t a, uint32_t n) {
    return a - (a % n);
}

// Round up to the nearest multiple of n
static inline uint32_t ROUNDUP(uint32_t a, uint32_t n) {
    return ROUNDDOWN(a + n - 1, n);
}

/* Constants */
static constexpr float PI = 3.14159f;

/* Unit Conversions */
#define DEG_TO_RAD (PI / 180.0f)
#define G_TO_M_S   (9.80665f)

#endif // COMMON_DEFS_H
