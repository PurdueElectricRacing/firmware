#include "lcd.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "common/can_library/generated/DASHBOARD.h"
#include "common/can_library/faults_common.h"
#include "common_defs.h"
#include "menu_system.h"
#include "nextion.h"
#include "pedals.h"
#include "main.h"

volatile page_t curr_page; // Current page displayed on the LCD
volatile page_t prev_page; // Previous page displayed on the LCD
char *errorText = nullptr; // Pointer to data to display for the Error, Warning, and Critical Fault codes
extern pedal_values_t pedal_values; // Global from pedals module for throttle display
extern q_handle_t q_fault_history; // Global queue from fault library for fault history
extern volatile dashboard_input_state_t input_state; // Global dashboard input states

// Faults Page Functions
void faultsPageUpdate();
void faultsMoveUp();
void faultsMoveDown();
void faultsSelect();
void updateFaultMessages();
void faultsClearButton_CALLBACK();

// Race Page Functions
void racePageUpdate();
void raceSelect();
void raceUpCallback(); // ! temp function to turn on pumps
void raceDownCallback(); // ! temp function to turn off pumps

// Telemetry Functions
void raceTelemetryUpdate();
void sdcTelemetryUpdate();
void faultTelemetryUpdate();
void calibrationTelemetryUpdate();

// Utility Functions
void updateSDCStatus(uint8_t status, char* element);
void set_faultIndicator(uint16_t fault, char* element);

// Page handlers array stored in flash
const page_handler_t page_handlers[] = {
    // Order must match page_t enum
    [PAGE_RACE]        = {racePageUpdate, nullptr, nullptr, raceSelect, raceTelemetryUpdate}, // No move handlers, telemetry is passive
    [PAGE_FAULTS]      = {faultsPageUpdate, faultsMoveUp, faultsMoveDown, faultsSelect, faultTelemetryUpdate},
    [PAGE_CALIBRATION] = {nullptr, nullptr, nullptr, nullptr, calibrationTelemetryUpdate}, // Calibration is passive
};

menu_element_t race_elements[] = {
    {
        .type          = ELEMENT_OPTION,
        .object_name   = RACE_TV_ON,
        .current_value = 0,
        .on_change     = nullptr,
    }
};

menu_page_t race_page = {
    .elements            = race_elements,
    .num_elements        = sizeof(race_elements) / sizeof(race_elements[0]),
    .current_index       = 0,
    .is_element_selected = false,
};

menu_element_t faults_elements[] = {
    [0] = {
        .type        = ELEMENT_BUTTON,
        .object_name = FAULT1_BUTTON,
        .on_change   = faultsClearButton_CALLBACK // clear fault
    },
    [1] = {
        .type        = ELEMENT_BUTTON,
        .object_name = FAULT2_BUTTON,
        .on_change   = faultsClearButton_CALLBACK // clear fault
    },
    [2] = {
        .type        = ELEMENT_BUTTON,
        .object_name = FAULT3_BUTTON,
        .on_change   = faultsClearButton_CALLBACK // clear fault
    },
    [3] = {
        .type        = ELEMENT_BUTTON,
        .object_name = FAULT4_BUTTON,
        .on_change   = faultsClearButton_CALLBACK // clear fault
    },
    [4] = {
        .type        = ELEMENT_BUTTON,
        .object_name = FAULT5_BUTTON,
        .on_change   = faultsClearButton_CALLBACK // clear fault
    },
    [5] = {
        .type        = ELEMENT_BUTTON,
        .object_name = CLEAR_BUTTON,
        .on_change   = faultsClearButton_CALLBACK // clear all faults
    }
};

menu_page_t faults_page = {
    .elements            = faults_elements,
    .num_elements        = sizeof(faults_elements) / sizeof(faults_elements[0]),
    .current_index       = 0,
    .is_element_selected = false};


// Initialize the LCD screen
// Preflight will be shown on power on, then reset to RACE
void LCD_init(uint32_t baud_rate) {
    NXT_setBaud(baud_rate);
    NXT_setBrightness(100);

    // Set page (leave preflight)
    curr_page = PAGE_RACE;
    updatePage();
}

/**
 * @brief Advances to the next selectable page
 */
void advancePage() {
    // todo
    updatePage();
}

/**
 * @brief Moves to the previous selectable page
 */
void backPage() {
    // todo

    updatePage();
}

/**
 * @brief Updates LCD display page
 *
 * Key behaviors:
 * - Maintains display of error pages when active
 */
void updatePage() {
    // If we do not detect a page update, do nothing
    if (curr_page == prev_page) {
        return;
    }

    // Set the page on display
    switch (curr_page) {
        case PAGE_RACE:
            NXT_setPage(RACE_STRING);
            break;
            break;
        case PAGE_FAULTS:
            NXT_setPage(FAULT_STRING);
            break;
        case PAGE_SDC_INFO:
            NXT_setPage(SDCINFO_STRING);
            break;
            break;
        case PAGE_CALIBRATION:
            NXT_setPage(APPS_STRING);
            break;
        default:
            curr_page = PAGE_RACE; // something probably went wrong
            break;
    }

    // Bounds Check
    if (curr_page > NUM_PAGES && curr_page < 0) {
        return;
    }

    // Call update handler if available
    if (page_handlers[curr_page].update != NULL) {
        page_handlers[curr_page].update();
    }
}

void moveUp() {
    // Bounds Check
    if (curr_page > NUM_PAGES && curr_page < 0) {
        return;
    }

    if (page_handlers[curr_page].move_up != NULL) {
        page_handlers[curr_page].move_up();
    }
}

void moveDown() {
    // Bounds Check
    if (curr_page > NUM_PAGES && curr_page < 0) {
        return;
    }

    if (page_handlers[curr_page].move_down != NULL) {
        page_handlers[curr_page].move_down();
    }
}

void selectItem() {
    // Bounds Check
    if (curr_page > NUM_PAGES && curr_page < 0) {
        return;
    }

    if (page_handlers[curr_page].select != NULL) {
        page_handlers[curr_page].select();
    }
}

/**
 * @brief Updates current telemetry page by calling its handler if available
 */
void updateTelemetryPages() {
    // Bounds Check
    if (curr_page > NUM_PAGES && curr_page < 0) {
        return;
    }

    if (page_handlers[curr_page].telemetry != NULL) {
        page_handlers[curr_page].telemetry();
    }
}

/**
 * @brief Updates the LCD display with current pedal telemetry data when on CALIBRATION page
 *
 * Updates brake and throttle bars, raw ADC values, deviation percentages, and status
 * indicators for brake and throttle pedals. Also displays fault statuses if detected.
 *
 * @note Only executes when current page is PAGE_CALIBRATION
 */
void calibrationTelemetryUpdate() {
    if (curr_page != PAGE_CALIBRATION) {
        return;
    }

    // todo
}

/**
 * @brief Updates fault messages on LCD screen based on priority and timing
 *
 * Manages fault display rotation, processes new faults from queue, and updates
 * screen according to fault priorities and display timing requirements.
 */
void updateFaultDisplay() {
    // ! TODO Reimplement this function using the new fault library
    return;
}

/**
 * @brief Updates the color of fault text indicators on the faults page
 */
void faultTelemetryUpdate() {
    if (curr_page != PAGE_FAULTS) {
        return;
    }

    // todo
}


/**
 * @brief Updates the display of fault messages on the LCD screen
 *
 * Checks fault buffer entries and displays either the corresponding fault message
 * or "No Fault" message for each of the 5 fault text fields on screen
 */
void updateFaultMessages() {
    // todo
}

void faultsPageUpdate() {
    updateFaultMessages();

    MS_refreshPage(&faults_page);
}

void faultsMoveUp() {
    MS_moveUp(&faults_page);
}

void faultsMoveDown() {
    MS_moveDown(&faults_page);
}

void faultsSelect() {
    MS_select(&faults_page);
}

/**
 * @brief Clears a fault from the fault buffer by removing it and shifting remaining faults
 *
 * @param index Position of the fault to clear (0-4)
 */
void clearFault(int index) {
    // todo
}

/**
 * @brief Callback function for fault button press that clears faults
 *
 * If hover index is 5, clears all faults (indices 0-4)
 * If hover index is 0-4, clears only that specific fault
 * Updates fault messages after clearing
 */
void faultsClearButton_CALLBACK() {
    int hover_index = faults_page.current_index;
    if (hover_index == 5) {
        for (int i = 4; i >= 0; i--) { // Clear all faults which are not latched
            clearFault(i);
        }
    } else {
        clearFault(hover_index);
    }

    updateFaultMessages();
}

void racePageUpdate() {
    MS_refreshPage(&race_page);
}

/**
 * @brief Updates telemetry data on the race dashboard LCD display
 *
 * Only updates on race page. Displays 'S' for stale values.
 */
void raceTelemetryUpdate() {
    if (curr_page != PAGE_RACE) {
        return;
    }

    NXT_setValue(BRK_BAR, (int)((pedal_values.brake / 4095.0) * 100)); // TODO BRK BAR
    NXT_setValue(THROT_BAR, (int)((pedal_values.throttle / 4095.0) * 100));

    // update the speed
    if (can_data.gps_speed.stale) {
        NXT_setText(SPEED, "S");
    } else {
        // uint16_t speed = can_data.rear_wheel_speeds.left_speed_mc * RPM_TO_MPH; // Convert to mph
        uint16_t speed = (uint16_t)(can_data.gps_speed.gps_speed * MPS_TO_MPH + 0.5); // Round to nearest whole number
        NXT_setTextFormatted(SPEED, "%d", speed);
    }

    // Update the voltage and current
    if (can_data.orion_currents_volts.stale) {
        NXT_setText(BATT_VOLT, "S");
        NXT_setText(BATT_CURR, "S");
    } else {
        uint16_t voltage = (can_data.orion_currents_volts.pack_voltage / 10);
        NXT_setTextFormatted(BATT_VOLT, "%dV", voltage);

        uint16_t current = (can_data.orion_currents_volts.pack_current / 10);
        NXT_setTextFormatted(BATT_CURR, "%dA", current); // Note: Changed 'V' to 'A' for current
    }

    // Update the battery temperature
    if (can_data.max_cell_temp.stale) {
        NXT_setText(BATT_TEMP, "S");
    } else {
        uint16_t batt_temp = can_data.max_cell_temp.max_temp / 10;
        NXT_setTextFormatted(BATT_TEMP, "%dC", batt_temp);
    }

    // Update the state of charge
    if (can_data.main_hb.stale) {
        NXT_setText(CAR_STAT, "S");
        NXT_setFontColor(CAR_STAT, WHITE);
    } else {
        switch (can_data.main_hb.car_state) {
            case CARSTATE_PRECHARGING:
                NXT_setFontColor(CAR_STAT, ORANGE);
                NXT_setText(CAR_STAT, "PRECHARGE");
                break;
            case CARSTATE_ENERGIZED:
                NXT_setFontColor(CAR_STAT, ORANGE);
                NXT_setText(CAR_STAT, "ENERGIZED");
                break;
            case CARSTATE_IDLE:
                NXT_setFontColor(CAR_STAT, INFO_GRAY);
                NXT_setText(CAR_STAT, "IDLE");
                break;
            case CARSTATE_READY2DRIVE:
                NXT_setFontColor(CAR_STAT, GREEN);
                NXT_setText(CAR_STAT, "R2D");
                break;
            // case CARSTATE_ERROR:
            //     NXT_setFontColor(CAR_STAT, YELLOW);
            //     NXT_setText(CAR_STAT, "ERROR");
            //     break;
            case CARSTATE_FATAL:
                NXT_setFontColor(CAR_STAT, RED);
                NXT_setText(CAR_STAT, "FATAL");
                break;
            // case CARSTATE_CONSTANT_TORQUE:
            //     NXT_setFontColor(CAR_STAT, GREEN);
            //     NXT_setText(CAR_STAT, "CONST TRQ");
            //     break;
            default:
                NXT_setFontColor(CAR_STAT, WHITE);
                NXT_setText(CAR_STAT, "UNKNOWN");
                break;
        }
    }
}

void raceSelect() {
    MS_select(&race_page);
    // TODO Race page TV settings
    // tv_elements[TV_ENABLE_INDEX].current_value = race_elements[0].current_value; // Sync TV settings
}

/**
 * @brief Sets the color of a fault indicator element based on fault status
 *
 * @param fault The fault code to check (0xFFFF indicates no fault)
 * @param element Pointer to the display element to be colored
 */
void set_faultIndicator(uint16_t fault, char* element) {
    if (fault == 0xFFFF) {
        NXT_setFontColor(element, WHITE);
        return;
    }

    if (is_latched(fault)) {
        NXT_setFontColor(element, RED);
    } else {
        NXT_setFontColor(element, GREEN);
    }
}

/**
 * @brief Updates the background color of an LCD element based on status
 *
 * @param status Boolean indicating if element should be marked as active (1) or inactive (0)
 * @param element Pointer to the LCD element to update
 */
void updateSDCStatus(uint8_t status, char* element) {
    if (status) {
        NXT_setBackground(element, GREEN);
    } else {
        NXT_setBackground(element, RED);
    }
}
