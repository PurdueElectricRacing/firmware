#ifndef MIN_H
#define MIN_H

/**
 * @file min.h
 * @brief Minimum value functions utility header.
 *
 * Typesafe, supporting ints and floats.
 * Generic MINOF interface for 2, 3, or 4 arguments.
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

[[gnu::always_inline]]
static inline int min2_int(int a, int b) {
    return (a < b) ? a : b;
}

[[gnu::always_inline]]
static inline float min2_float(float a, float b) {
    return (a < b) ? a : b;
}

#define MINOF2(a, b) _Generic((a) + (b), \
    int: min2_int, \
    float: min2_float \
)(a, b)

#define MINOF3(a, b, c) MINOF2(MINOF2(a, b), c)
#define MINOF4(a, b, c, d) MINOF2(MINOF2(a, b), MINOF2(c, d))
// if you need more than four arguments, just add them here

// dispatcher macro selects the correct MINOF function based on the number of arguments
#define DISPATCH_MINOF(_1, _2, _3, _4, NAME, ...) NAME
#define MINOF(...) DISPATCH_MINOF(__VA_ARGS__, MINOF4, MINOF3, MINOF2)(__VA_ARGS__)

#endif // MIN_H