#ifndef RESCALE_H
#define RESCALE_H

/**
 * @file rescale.h
 * @brief Utility header to linearly map values from one range to another.
 *
 * Typesafe, supporting ints and floats.
 * Integer promotions also cover int8_t, uint8_t, int16_t, uint16_t
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

[[gnu::always_inline]]
static inline signed int rescale_signed(
    signed int input,
    signed int input_min,
    signed int input_max,
    signed int output_min,
    signed int output_max
) {
    signed int input_range = input_max - input_min;
    signed int output_range = output_max - output_min;

    if (input_range == 0) return output_min;

    long numerator = (long)(input - input_min) * (long)output_range;
    return output_min + (signed int)(numerator / input_range);
}

[[gnu::always_inline]]
static inline unsigned int rescale_unsigned(
    unsigned int input,
    unsigned int input_min,
    unsigned int input_max,
    unsigned int output_min,
    unsigned int output_max
) {
    unsigned int input_range = input_max - input_min;
    unsigned int output_range = output_max - output_min;

    if (input_range == 0u) return output_min;

    unsigned long numerator =
        (unsigned long)(input - input_min) * (unsigned long)output_range;

    return output_min + (unsigned int)(numerator / input_range);
}

[[gnu::always_inline]]
static inline float rescale_float(
    float input,
    float input_min,
    float input_max,
    float output_min,
    float output_max
) {
    float input_range = input_max - input_min;
    float output_range = output_max - output_min;

    if (input_range == 0.0f) return output_min;

    return output_min + ((input - input_min) * output_range) / input_range;
}

#define RESCALE(input, input_min, input_max, output_min, output_max) \
    _Generic((input) + (input_min) + (input_max) + (output_min) + (output_max), \
    signed int: rescale_signed, \
    unsigned int: rescale_unsigned, \
    float: rescale_float \
)(input, input_min, input_max, output_min, output_max)

#endif // RESCALE_H