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

#ifndef COMMON_DEFS_H_
#define COMMON_DEFS_H_

#include <stdint.h>

/* Math Functions */
static inline int32_t min_i(int32_t x, int32_t y) { return (x < y) ? x : y; }
static inline float min_f(float x, float y) { return (x < y) ? x : y; }
#define MIN(x, y) _Generic((x), float: min_f, double: min_f, default: min_i)((x), (y))

static inline int32_t max_i(int32_t x, int32_t y) { return (x < y) ? y : x; }
static inline float max_f(float x, float y) { return (x < y) ? y : x; }
#define MAX(x, y) _Generic((x), float: max_f, double: max_f, default: max_i)((x), (y))

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

static inline int32_t abs_i(int32_t x) { return (x < 0) ? -x : x; }
static inline float abs_f(float x) { return (x < 0.0f) ? -x : x; }
#define ABS(x) _Generic((x), float: abs_f, double: abs_f, default: abs_i)(x)

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
#define PI (3.14159f)

/* Unit Conversions */
#define DEG_TO_RAD (PI / 180.0f)
#define G_TO_M_S   (9.80665f)

/* Per-Node HSI RCC Trim Constants */
#define HSI_TRIM_TORQUE_VECTOR 15
#define HSI_TRIM_MAIN_MODULE   15
#define HSI_TRIM_PDU           16
#define HSI_TRIM_DASHBOARD     15
#define HSI_TRIM_DAQ           17
#define HSI_TRIM_A_BOX         16

static constexpr uint32_t CONN_LED_TIMEOUT_MS = 1000;
static constexpr uint32_t PREFLIGHT_DURATION_MS = 1500;
static constexpr uint32_t HEARTBEAT_PERIOD_MS = 100;


#endif /* COMMON_DEFS_H_ */
