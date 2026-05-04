/**
 * @file pedals.c
 * @brief Pedal processing logic
 *
 * @author Irving Wang (irvingw@purdue.edu)
 * @author Luke Oxley (lcoxley@purdue.edu)
 */

#include "pedals.h"

#include <stdint.h>

#include "can_library/faults_common.h"
#include "can_library/generated/DASHBOARD.h"
#include "common/utils/clamp.h"
#include "common/utils/rescale.h"
#include "main.h"

// ! pedal calibration constants
static constexpr uint16_t THROTTLE1_MIN = 0;
static constexpr uint16_t THROTTLE1_MAX = 410;
static_assert(THROTTLE1_MIN < THROTTLE1_MAX, "Invalid throttle 1 calibration values");

static constexpr uint16_t THROTTLE2_MIN = 5;
static constexpr uint16_t THROTTLE2_MAX = 310;
static_assert(THROTTLE2_MIN < THROTTLE2_MAX, "Invalid throttle 2 calibration values");

static constexpr uint16_t REGEN1_MIN = 2800;
static constexpr uint16_t REGEN1_MAX = 3350;
// static constexpr uint16_t BRAKE2_MIN = 0;
// static constexpr uint16_t BRAKE2_MAX = 4095;

// static constexpr uint16_t BRAKE1_PRESSURE_MIN = 0;
// static constexpr uint16_t BRAKE1_PRESSURE_MAX = 3000;

static constexpr uint8_t PEDAL_MAX = 100;
static constexpr uint8_t PEDAL_MIN = 0;
static constexpr uint8_t APPS_THROTTLE_THRESHOLD = 30; // 10%
static constexpr uint8_t APPS_BRAKE_THRESHOLD = PEDAL_MAX / 10u; // 10%

// Contains the current pedal values for global visibility
volatile pedals_data_t pedal_values = {
    .throttle = 0,
    .regen = 0,
    .brake = 0
};

/**
 * @brief Processes pedal sensor readings and sets faults as necessary
 *
 * @note This function is called periodically by the scheduler
 */
void pedals_periodic(void) {
    // Get current values (don't want them changing mid-calculation)
    uint16_t throttle1 = raw_adc_values.throttle1;
    uint16_t throttle2 = 4095 - raw_adc_values.throttle2; // Invert value for t2 (pull-up resistor)
    uint16_t regen1    = raw_adc_values.regen1;
    // uint16_t brake2 = raw_adc_values.brake2_pressure;

    // FSAE 2026 T.4.2.10
    update_fault(FAULT_ID_APPS_WIRING_T1, 1);
    update_fault(FAULT_ID_APPS_WIRING_T2, 1);

    // Hard clamp the raw values to the min and max values to account for physical limits
    throttle1 = CLAMP(throttle1, THROTTLE1_MIN, THROTTLE1_MAX);
    throttle2 = CLAMP(throttle2, THROTTLE2_MIN, THROTTLE2_MAX);
    regen1    = CLAMP(regen1, REGEN1_MIN, REGEN1_MAX);
    // brake2 = CLAMP(brake2, BRAKE2_MIN, BRAKE2_MAX);

    // Normalize pedal signals to the 0-4095 range while preserving a linear relationship
    throttle1 = RESCALE(throttle1, THROTTLE1_MIN, THROTTLE1_MAX, PEDAL_MIN, PEDAL_MAX);
    throttle2 = RESCALE(throttle2, THROTTLE2_MIN, THROTTLE2_MAX, PEDAL_MIN, PEDAL_MAX);
    regen1    = RESCALE(regen1, REGEN1_MIN, REGEN1_MAX, PEDAL_MIN, PEDAL_MAX);
    // brake2 = RESCALE(brake2, BRAKE2_MIN, BRAKE2_MAX, 0, 100);
    
    pedal_values.throttle = throttle1;
    pedal_values.regen = regen1;

    uint16_t throttle_command = throttle1;

    // FSAE 2026 T.4.2.5
    // uint16_t throttle_diff;
    // if (throttle1 > throttle2) {
    //     throttle_diff = throttle1 - throttle2;
    // } else {
    //     throttle_diff = throttle2 - throttle1;
    // }
    update_fault(FAULT_ID_APPS_IMPLAUSIBLE, 1); // ! disabled for now
    if (is_latched(FAULT_ID_APPS_IMPLAUSIBLE)) {
        throttle_command = 0;
    }

    // If both pedals are pressed, set throttle to 0
    // todo FSAE 2026 EV.4.7
    bool is_brake_pressed = pedal_values.regen >= APPS_BRAKE_THRESHOLD; // regen is the first throw of the brake
    bool is_throttle_pressed = pedal_values.throttle >= APPS_THROTTLE_THRESHOLD;
    update_fault(FAULT_ID_APPS_BRAKE, is_brake_pressed && is_throttle_pressed);
    if (is_latched(FAULT_ID_APPS_BRAKE)) {
        throttle_command = 0;
    }

    CAN_SEND_pedals(throttle_command, pedal_values.regen, pedal_values.brake);
}