/**
 * @file bangbang.c
 * @brief Generic bang-bang controller with hysteresis and minimum switch interval.
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "bangbang.h"

/**
 * @brief Update the state of the bang-bang controller based on the input value and current time.
 * It calls the appropriate on/off functions if the state changes
 *
 * @param controller Pointer to the bang-bang controller.
 * @param value The current input value.
 * @param current_time The current time in milliseconds.
 */
void bangbang_update(bangbang_t *controller, float value, uint32_t current_time) {
    uint32_t time_since_last_switch = current_time - controller->last_switch_ms;
    bool hysteresis_passed = time_since_last_switch >= controller->min_switch_interval;

    // update the internal state of the controller
    if (controller->is_on) {
        if (value <= controller->lower_bound && hysteresis_passed) {
            controller->is_on = false;
            controller->last_switch_ms = current_time;
        }
    } else {
        if (value >= controller->upper_bound && hysteresis_passed) {
            controller->is_on = true;
            controller->last_switch_ms = current_time;
        }
    }

    // call the appropriate function based on the state (if it exists)
    if (controller->is_on) {
        if (controller->on_func != nullptr) controller->on_func();
    } else {
        if (controller->off_func != nullptr) controller->off_func();
    }
}
