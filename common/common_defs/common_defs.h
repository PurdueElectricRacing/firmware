/**
 * @file common_defs.h
 * @author Adam Busch (busch8@purdue.edu)
 * @brief Common defs for the entire firmware reposity. Dont let this get too out of control please.
 * @version 0.1
 * @date 2022-01-26
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef COMMON_DEFS_H_
#define COMMON_DEFS_H_

/* Math Functions */
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) < (y) ? (y) : (x))
#define CLAMP(x, min, max)  MAX((min), MIN((x), (max)))

// Base-2 logarithm that rounds down
#define LOG2_DOWN(x) (31U - __builtin_clzl((x)))
// Base-2 logarithm that rounds up
#define LOG2_UP(x) (LOG2_DOWN((x) - 1) + 1)

/* Unit Conversions */

/* Constants */

/* Obligitory */
#define GREAT
#define IS
#define PER IS GREAT

#endif /* COMMON_DEFS_H_ */