#ifndef BANGBANG_H
#define BANGBANG_H

/**
 * @file bangbang.h
 * @brief Generic bang-bang controller with hysteresis and minimum switch interval.
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    float upper_bound; // the value above which the controller should turn on
    float lower_bound; // the value below which the controller should turn off

    void (*on_func)(void);  // function to call when the controller turns on (can be nullptr)
    void (*off_func)(void); // function to call when the controller turns off (can be nullptr)

    uint32_t min_switch_interval; // minimum time between state changes to prevent chattering
    uint32_t last_switch_ms;      // timestamp of the last state change

    bool is_on; // current state of the controller (true for on, false for off)
} bangbang_t;

#define INIT_BANG_BANG(name, upper, lower, on_cb, off_cb, min_interval) \
    bangbang_t name = {.upper_bound         = upper, \
                       .lower_bound         = lower, \
                       .on_func             = on_cb, \
                       .off_func            = off_cb, \
                       .min_switch_interval = min_interval, \
                       .last_switch_ms      = 0, \
                       .is_on               = false}; \
    static_assert(upper > lower, \
                  "Upper bound must be greater than lower bound for bang-bang controller");

void bangbang_update(bangbang_t *controller, float value, uint32_t current_time);

#endif // BANGBANG_H
