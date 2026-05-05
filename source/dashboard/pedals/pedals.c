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
#include "common/utils/abs.h"
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
static constexpr uint8_t APPS_THROTTLE_THRESHOLD = 25; // 25% travel
static constexpr uint8_t APPS_MECH_BRAKE_THRESHOLD = 80; // 80% travel of the regen pot

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
    // snapshot ADC values into local memory
    uint16_t throttle1 = raw_adc_values.throttle1;
    uint16_t throttle2 = 4095 - raw_adc_values.throttle2; // Invert value for t2 (pull-up resistor)
    uint16_t regen1    = raw_adc_values.regen1;
    // uint16_t brake2 = raw_adc_values.brake2_pressure;

    // FSAE 2026 T.4.2.10: open/short circuit detection
    update_fault(FAULT_ID_APPS_WIRING_T1, 1);
    update_fault(FAULT_ID_APPS_WIRING_T2, 1);

    // saturate the raw values to the calibration range
    throttle1 = CLAMP(throttle1, THROTTLE1_MIN, THROTTLE1_MAX);
    throttle2 = CLAMP(throttle2, THROTTLE2_MIN, THROTTLE2_MAX);
    regen1    = CLAMP(regen1, REGEN1_MIN, REGEN1_MAX);
    // brake2 = CLAMP(brake2, BRAKE2_MIN, BRAKE2_MAX);

    // rescale the pedal signals to [0,100] range
    throttle1 = RESCALE(throttle1, THROTTLE1_MIN, THROTTLE1_MAX, PEDAL_MIN, PEDAL_MAX);
    throttle2 = RESCALE(throttle2, THROTTLE2_MIN, THROTTLE2_MAX, PEDAL_MIN, PEDAL_MAX);
    regen1    = RESCALE(regen1, REGEN1_MIN, REGEN1_MAX, PEDAL_MIN, PEDAL_MAX);
    // brake2 = RESCALE(brake2, BRAKE2_MIN, BRAKE2_MAX, 0, 100);

    pedal_values.throttle    = throttle1;
    pedal_values.regen       = regen1;
    uint8_t throttle_command = throttle1;

    // FSAE 2026 T.4.2.5: if the two throttle sensors differ by 10%, trigger implaus
    int throttle_diff = ABS((int)throttle1 - (int)throttle2);
    update_fault(FAULT_ID_APPS_IMPLAUSIBLE, throttle_diff); // ! disabled for now
    if (is_latched(FAULT_ID_APPS_IMPLAUSIBLE)) {
        throttle_command = 0;
    }

    // FSAE 2026 EV.4.7: if both pedals are pressed, set throttle to 0
    // todo: unlatch when throttle is released to 5%
    bool is_brake_pressed = pedal_values.regen >= APPS_MECH_BRAKE_THRESHOLD;
    bool is_throttle_pressed = pedal_values.throttle >= APPS_THROTTLE_THRESHOLD;
    update_fault(FAULT_ID_APPS_BRAKE, is_brake_pressed && is_throttle_pressed);
    if (is_latched(FAULT_ID_APPS_BRAKE)) {
        throttle_command = 0;
    }

    CAN_SEND_pedals(throttle_command, pedal_values.regen, pedal_values.brake);
}