#ifndef COUNTOF_H
#define COUNTOF_H

// todo replace with <stdcountof.h> when gcc16 is released

/**
 * @file countof.h
 * @brief macro-based countof implementation
 *
 * Helper macro to count the number of elements in an array.
 * Passing a pointer is rejected at compile time.
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

// internal, do not use this
#define _COUNTOF_IS_ARRAY(x) (!__builtin_types_compatible_p(typeof(x), typeof(&(x)[0])))

#define countof(array) \
    ({ \
        static_assert(_COUNTOF_IS_ARRAY(array), \
        "countof() argument must be an array"); \
        (sizeof(array) / sizeof((array)[0])); \
    }) \

#endif // COUNTOF_H