#include "pedals.h"
//#include "common/phal/flash.h"
#include <stdint.h>

#include "common/can_library/generated/DASHBOARD.h"
#include "common/can_library/faults_common.h"
#include "common_defs.h"
#include "main.h"

pedal_faults_t pedal_faults = {0};

uint16_t thtl_limit = 4096;

// Globals to enable live watching
uint16_t t1_raw;
uint16_t t2_raw;
uint16_t b1_raw;
uint16_t b2_raw;
uint16_t t1_final;
uint16_t t2_final;
uint16_t b1_final;
uint16_t b2_final;

// TODO: tune these values for the new pedals
// ! WARNING: DAQ VARIABLE, IF EEPROM ENABLED, VALUE WILL CHANGE
pedal_calibration_t pedal_calibration = {
    // These values are given from 0-4095
    .t1_min = 980,
    .t1_max = 1600,
    .t2_min = 1550,
    .t2_max = 2050,
    .b1_min = 475,
    .b1_max = 1490,
    .b2_min = 500,
    .b2_max = 1490,
};

// Contains the current pedal values for external use
pedal_values_t pedal_values = {
    .throttle = 0,
    .brake    = 0,
};

// Allows for drivers to set their own pedal profiles
driver_pedal_profile_t driver_pedal_profiles[4] = {
    // TODO link to pedal logic
    {0, 10, 10, 0},
    {1, 10, 10, 0},
    {2, 10, 10, 0},
    {3, 10, 10, 0},
};

/**
 * @brief Normalizes a value between min and max to a range of 0 to MAX_PEDAL_MEAS (4095)
 *
 * @param value Raw value to normalize
 * @param min Minimum value of the input range
 * @param max Maximum value of the input range
 */
static inline uint16_t normalize(uint16_t value, uint16_t min, uint16_t max) {
    // Use a 32-bit value to prevent overflow
    return (uint16_t)(((uint32_t)(value - min) * MAX_PEDAL_MEAS) / (max - min));
}

/**
 * @brief Processes pedal sensor readings and sets faults as necessary
 *
 * @note This function is called periodically by the scheduler
 */
void pedalsPeriodic(void) {
    // Get current values (don't want them changing mid-calculation)
    t1_raw = raw_adc_values.t1;
    t2_raw = 4095 - raw_adc_values.t2; // Invert value for t2 (pull-up resistor)
    b1_raw = raw_adc_values.b1;
    b2_raw = raw_adc_values.b2;

    // Check for wiring faults
    update_fault(FAULT_INDEX_DASHBOARD_APPS_WIRING_T1, t1_raw);
    update_fault(FAULT_INDEX_DASHBOARD_APPS_WIRING_T2, t2_raw);

    // Hard clamp the raw values to the min and max values to account for physical limits
    uint16_t t1_clamped = CLAMP(t1_raw, pedal_calibration.t1_min, pedal_calibration.t1_max);
    uint16_t t2_clamped = CLAMP(t2_raw, pedal_calibration.t2_min, pedal_calibration.t2_max);
    uint16_t b1_clamped = CLAMP(b1_raw, pedal_calibration.b1_min, pedal_calibration.b1_max);
    uint16_t b2_clamped = CLAMP(b2_raw, pedal_calibration.b2_min, pedal_calibration.b2_max);

    // Normalize pedal signals to the 0-4095 range while preserving a linear relationship
    t1_final = normalize(t1_clamped, pedal_calibration.t1_min, pedal_calibration.t1_max);
    t2_final = normalize(t2_clamped, pedal_calibration.t2_min, pedal_calibration.t2_max);
    b1_final = normalize(b1_clamped, pedal_calibration.b1_min, pedal_calibration.b1_max);
    b2_final = normalize(b2_clamped, pedal_calibration.b2_min, pedal_calibration.b2_max);

    // If both pedals are pressed, set a fault
    if ((b1_final >= APPS_BRAKE_THRESHOLD && t1_final >= APPS_THROTTLE_FAULT_THRESHOLD) || (is_latched(FAULT_INDEX_DASHBOARD_APPS_BRAKE) && t1_final >= APPS_THROTTLE_CLEARFAULT_THRESHOLD)) {
        // Set APPS to 0
        t2_final = 0;
        t1_final = 0;
        update_fault(FAULT_INDEX_DASHBOARD_APPS_BRAKE, 1);
    } else if (t1_final <= APPS_THROTTLE_CLEARFAULT_THRESHOLD) { // Clear fault if throttle is released
        update_fault(FAULT_INDEX_DASHBOARD_APPS_BRAKE, 0);
    }

    // Check for APPS sensor deviations (10%)
    update_fault(FAULT_INDEX_DASHBOARD_IMPLAUS_DETECTED, ABS((int16_t)t1_final - (int16_t)t2_final));

    // Update the pedal values for external use
    pedal_values.throttle = t1_final;
    pedal_values.brake    = b1_final;

    // Log the raw pedal values
    CAN_SEND_raw_throttle_brake(t1_raw, t2_raw, b1_raw, b2_raw, 0);

    // Send the normalized pedal values to Main and TV
    CAN_SEND_filt_throttle_brake(t1_final, b1_final);
}

// ! the code below will work only if watchdog is disabled
// static const uint32_t* PROFILE_FLASH_START = (uint32_t*)ADDR_FLASH_SECTOR_3;
// static volatile uint32_t* profile_current_address;

// TODO deprecate this feature
int writePedalProfiles() {
    // profile_current_address = (volatile uint32_t*)PROFILE_FLASH_START;

    //  // !! This will cause a crash if watchdog is enabled !!
    // if (FLASH_OK != PHAL_flashErasePage(PROFILES_START_SECTOR)) {
    //     return PROFILE_WRITE_FAIL;
    // }

    // for (uint8_t i = 0; i < NUM_PROFILES; ++i) {
    //     if (FLASH_OK != PHAL_flashWriteU32((uint32_t)profile_current_address,
    //                                      *(uint32_t*)&driver_pedal_profiles[i])) {
    //         return PROFILE_WRITE_FAIL;
    //     }
    //     profile_current_address++;
    // }

    return PROFILE_WRITE_SUCCESS;
}

void readPedalProfiles() {
    // uint32_t read_address = ADDR_FLASH_SECTOR_3;

    // for (uint8_t i = 0; i < NUM_PROFILES; ++i) {
    //     uint32_t *data = (uint32_t *)&driver_pedal_profiles[i];
    //     *data = *((uint32_t *)read_address);

    //     read_address += sizeof(driver_pedal_profile_t);
    // }
}
