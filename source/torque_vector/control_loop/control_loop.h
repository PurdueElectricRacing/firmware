#ifndef CONTROL_LOOP_H
#define CONTROL_LOOP_H

/**
 * @file control_loop.h
 * @brief Main torque vectoring control loop
 *
 * @author Irving Wang (irvingw@purdue.edu)
 * @author Trevor Koessler (tkoessle@purdue.edu)
 */

#include <stdint.h>

static constexpr uint32_t CONTROL_LOOP_PERIOD_MS = 10;
void control_loop(void);

#endif // CONTROL_LOOP_H