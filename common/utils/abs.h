#ifndef ABS_H
#define ABS_H

/**
 * @file abs.h
 * @brief Absolute value functions utility header.
 *
 * Typesafe, supporting ints and floats.
 * Integer promotions also cover int8_t, int16_t
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

[[gnu::always_inline]]
static inline int abs_i(int x) {
    return (x < 0) ? -x : x;
}

[[gnu::always_inline]]
static inline float abs_f(float x) {
    return (x < 0.0f) ? -x : x;
}

#define ABS(x) _Generic((x), \
    int: abs_i, \
    float: abs_f \
)((x))

#endif // ABS_H