#ifndef COUNTOF_H
#define COUNTOF_H

// todo replace with <stdcountof.h> when gcc16 is released

/**
 * @file countof.h
 * @brief macro-based countof implementation
 *
 * Helper macro to count the number of elements in an array.
 * @warning Does not check whether the element is actually an array!
 *
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#define countof(array) (sizeof(array) / sizeof((array)[0]))

#endif // COUNTOF_H