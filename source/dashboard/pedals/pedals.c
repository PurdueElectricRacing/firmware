#include "pedals.h"
//#include "common/phal_F4_F7/flash/flash.h"
#include "common/faults/faults.h"
#include "common_defs.h"
#include "main.h"
#include "common/phal_F4_F7/gpio/gpio.h"
#include "can_parse.h"
#include <stdint.h>

pedal_faults_t pedal_faults = {0};
uint16_t thtl_limit = 4096;

// TODO: tune these values for the new pedals
// ! WARNING: DAQ VARIABLE, IF EEPROM ENABLED, VALUE WILL CHANGE
pedal_calibration_t pedal_calibration = {  // These values are given from 0-4095
    .t1_min = 1000, .t1_max = 1640,
    .t2_min = 2000, .t2_max = 2760,
    .b1_min = 450, .b1_max = 1490,
    .b2_min = 450, .b2_max = 1490,
};

pedal_values_t pedal_values = {
    .throttle = 0,
    .brake    = 0
};

driver_pedal_profile_t driver_pedal_profiles[4] = {
    {0, 10, 10, 0},
    {1, 10, 10, 0},
    {2, 10, 10, 0},
    {3, 10, 10, 0}
};

/**
 * @brief Normalizes a value between min and max to a range of 0 to MAX_PEDAL_MEAS
 *
 * @param value Raw value to normalize
 * @param min Minimum value of the input range
 * @param max Maximum value of the input range
 */
static inline uint16_t normalize(uint16_t value, uint16_t min, uint16_t max) {
    // Use a 32-bit value to prevent overflow
    return (uint16_t) (((uint32_t)(value - min) * MAX_PEDAL_MEAS) / (max - min));
}

/**
 * @brief Processes pedal sensor readings and sets faults as necessary
 * 
 * @note This function is called periodically by the scheduler
 */
void pedalsPeriodic(void) {
    // Get current values (don't want them changing mid-calculation)
    uint16_t t1_raw = raw_adc_values.t1;
    uint16_t t2_raw = 4095 - raw_adc_values.t2; // Invert value for t2 (pull-up resistor)
    uint16_t b1_raw = raw_adc_values.b1;
    uint16_t b2_raw = raw_adc_values.b2;

    // Check for wiring faults
    setFault(ID_APPS_WIRING_T1_FAULT, t1_raw);
    setFault(ID_APPS_WIRING_T2_FAULT, t2_raw);
    setFault(ID_BSE_FAULT, PHAL_readGPIO(BRK_FAIL_TAP_GPIO_Port, BRK_FAIL_TAP_Pin));

    // Scale values based on min and max raw adc values
    uint16_t t1_clamped = CLAMP(t1_raw, pedal_calibration.t1_min, pedal_calibration.t1_max);
    uint16_t t2_clamped = CLAMP(t2_raw, pedal_calibration.t2_min, pedal_calibration.t2_max);
    uint16_t b1_clamped = CLAMP(b1_raw, pedal_calibration.b1_min, pedal_calibration.b1_max);
    uint16_t b2_clamped = CLAMP(b2_raw, pedal_calibration.b2_min, pedal_calibration.b2_max);

    // These values given are in the 0-4095 range
    uint16_t t1_final = normalize(t1_clamped, pedal_calibration.t1_min, pedal_calibration.t1_max);
    uint16_t t2_final = normalize(t2_clamped, pedal_calibration.t2_min, pedal_calibration.t2_max);
    uint16_t b1_final = normalize(b1_clamped, pedal_calibration.b1_min, pedal_calibration.b1_max);
    uint16_t b2_final = normalize(b2_clamped, pedal_calibration.b2_min, pedal_calibration.b2_max);

    // If both pedals are pressed, set fault
    if ((b1_final >= APPS_BRAKE_THRESHOLD && t1_final >= APPS_THROTTLE_FAULT_THRESHOLD) ||
        (checkFault(ID_APPS_BRAKE_FAULT) && t1_final >= APPS_THROTTLE_CLEARFAULT_THRESHOLD)) {
        // set warning fault and treq could be 0
        t2_final = 0;
        t1_final = 0;
        setFault(ID_APPS_BRAKE_FAULT, true);
    } else if (t1_raw <= APPS_THROTTLE_CLEARFAULT_THRESHOLD) { // Clear fault if throttle is released
        setFault(ID_APPS_BRAKE_FAULT, false);
    }

    // Check for APPS sensor deviations (10%)
    setFault(ID_IMPLAUS_DETECTED_FAULT, ABS(t1_final - t2_final));

    // Update the pedal values for external use
    pedal_values.throttle = t1_final;
    pedal_values.brake = b1_final;

    SEND_RAW_THROTTLE_BRAKE(t1_raw, t2_raw, b1_raw, b2_raw, 0);

    SEND_FILT_THROTTLE_BRAKE(t1_final, b1_final);
}


// ! the code below will work only if watchdog is disabled
// static const uint32_t* PROFILE_FLASH_START = (uint32_t*)ADDR_FLASH_SECTOR_3;
// static volatile uint32_t* profile_current_address;

// TODO move to main
int writePedalProfiles() { // TODO switch to EEPROM
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