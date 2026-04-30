#ifndef CLAMP_H
#define CLAMP_H

/**
 * @file clamp.h
 * @brief Utility header to clamp a value to a bounded range
 *
 * Typesafe, supporting ints and floats.
 * Integer promotions also cover int8_t, uint8_t, int16_t, uint16_t
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

[[gnu::always_inline]]
static inline signed int clamp_signed(signed int input, signed int lower_bound, signed int upper_bound) {
    if (input < lower_bound) return lower_bound;
    if (input > upper_bound) return upper_bound;
    return input;
}

[[gnu::always_inline]]
static inline unsigned int clamp_unsigned(unsigned int input, unsigned int lower_bound, unsigned int upper_bound) {
    if (input < lower_bound) return lower_bound;
    if (input > upper_bound) return upper_bound;
    return input;
}

[[gnu::always_inline]]
static inline float clamp_f(float input, float lower_bound, float upper_bound) {
    if (input < lower_bound) return lower_bound;
    if (input > upper_bound) return upper_bound;
    return input;
}

#define CLAMP(input, lower_bound, upper_bound) \
    _Generic((input) + (lower_bound) + (upper_bound), \
    signed int: clamp_signed, \
    unsigned int: clamp_unsigned, \
    float: clamp_f \
)(input, lower_bound, upper_bound)


#endif // CLAMP_H