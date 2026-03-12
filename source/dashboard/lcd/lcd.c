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
// extern q_handle_t q_fault_history; // Global queue from fault library for fault history
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
        .object_name = FAULT6_BUTTON,
        .on_change   = faultsClearButton_CALLBACK // clear fault
    },
    [6] = {
        .type        = ELEMENT_BUTTON,
        .object_name = FAULT7_BUTTON,
        .on_change   = faultsClearButton_CALLBACK // clear fault
    },
    [7] = {
        .type        = ELEMENT_BUTTON,
        .object_name = FAULT8_BUTTON,
        .on_change   = faultsClearButton_CALLBACK // clear fault
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

// Pages available for user navigation
static const page_t selectable_pages[] = {
    PAGE_RACE,
    PAGE_CALIBRATION,
    PAGE_FAULTS,
};
static constexpr uint32_t NUM_SELECTABLE_PAGES = sizeof(selectable_pages) / sizeof(selectable_pages[0]);

static int current_page_index = 0;

/**
 * @brief Advances to the next selectable page
 */
void advancePage() {
    current_page_index = (current_page_index + 1) % NUM_SELECTABLE_PAGES;
    curr_page = selectable_pages[current_page_index];
    updatePage();
}

/**
 * @brief Moves to the previous selectable page
 */
void backPage() {
    current_page_index = (current_page_index - 1 + NUM_SELECTABLE_PAGES) % NUM_SELECTABLE_PAGES;
    curr_page = selectable_pages[current_page_index];
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
        case PAGE_FAULTS:
            NXT_setPage(FAULT_STRING);
            break;
        case PAGE_CALIBRATION:
            NXT_setPage(CALIBRATION_STRING);
            break;
        default:
            curr_page = PAGE_RACE; // something probably went wrong
            break;
    }

    prev_page = curr_page;

    // Bounds Check
    if (curr_page >= NUM_PAGES) {
        return;
    }

    // Call update handler if available
    if (page_handlers[curr_page].update != NULL) {
        page_handlers[curr_page].update();
    }
}

void moveUp() {
    // Bounds Check
    if (curr_page >= NUM_PAGES) {
        return;
    }

    if (page_handlers[curr_page].move_up != NULL) {
        page_handlers[curr_page].move_up();
    }
}

void moveDown() {
    // Bounds Check
    if (curr_page >= NUM_PAGES) {
        return;
    }

    if (page_handlers[curr_page].move_down != NULL) {
        page_handlers[curr_page].move_down();
    }
}

void selectItem() {
    // Bounds Check
    if (curr_page >= NUM_PAGES) {
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
    if (curr_page >= NUM_PAGES) {
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
    if (hover_index == 8) {
        for (int i = 7; i >= 0; i--) { // Clear all faults which are not latched
            clearFault(i);
        }
    } else {
        clearFault(hover_index);
    }

    updateFaultMessages();
}

void style_car_stat() {
    if (can_data.main_hb.stale) {
        NXT_setText(CAR_STAT, "STALE");
        NXT_setFontColor(CAR_STAT, WHITE);
        return;
    }

    switch (can_data.main_hb.car_state) {
        case CAR_STATE_INIT:
            NXT_setFontColor(CAR_STAT, WHITE);
            NXT_setText(CAR_STAT, "INIT");
            NXT_setBorderColor(CAR_STAT, WHITE);
            break;
        case CAR_STATE_IDLE:
            NXT_setFontColor(CAR_STAT, WHITE);
            NXT_setText(CAR_STAT, "IDLE");
            NXT_setBorderColor(CAR_STAT, WHITE);
            break;
        case CAR_STATE_PRECHARGING:
            NXT_setFontColor(CAR_STAT, YELLOW);
            NXT_setText(CAR_STAT, "PRECHRG");
            NXT_setBorderColor(CAR_STAT, YELLOW);
            break;
        case CAR_STATE_ENERGIZED:
            NXT_setFontColor(CAR_STAT, GREEN);
            NXT_setText(CAR_STAT, "ENERGZD");
            NXT_setBorderColor(CAR_STAT, GREEN);
            break;
        case CAR_STATE_BUZZING:
            NXT_setFontColor(CAR_STAT, YELLOW);
            NXT_setText(CAR_STAT, "BUZZING");
            NXT_setBorderColor(CAR_STAT, YELLOW);
            break;
        case CAR_STATE_READY2DRIVE:
            NXT_setFontColor(CAR_STAT, GREEN);
            NXT_setText(CAR_STAT, "R2D");
            NXT_setBorderColor(CAR_STAT, GREEN);
            break;
        case CAR_STATE_FATAL:
            NXT_setFontColor(CAR_STAT, RED);
            NXT_setText(CAR_STAT, "FATAL");
            NXT_setBorderColor(CAR_STAT, RED);
            break;
        default:
            NXT_setFontColor(CAR_STAT, WHITE);
            NXT_setText(CAR_STAT, "UNKNOWN");
            NXT_setBorderColor(CAR_STAT, WHITE);
            break;
    }
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

    if (can_data.pack_stats.stale) {
        NXT_setText(BATT_VOLT, "S");
        NXT_setText(BATT_CURR, "S");
    } else {
        NXT_setTextFormatted(BATT_VOLT, "%d", can_data.pack_stats.pack_voltage);
        NXT_setTextFormatted(BATT_CURR, "%d", can_data.pack_stats.pack_current);
    }

    // todo better speed calc lol
    if (can_data.wheel_speeds.stale) {
        NXT_setText(SPEED, "S");
    } else {
        if (can_data.wheel_speeds.rear_left < 0) {
            NXT_setText(SPEED, "NEG");
        } else {
            uint16_t speed = can_data.wheel_speeds.front_left * RPM_TO_MPH; // Convert to mph
            NXT_setTextFormatted(SPEED, "%d", speed);
        }
    }

    style_car_stat();
}

void raceSelect() {
    MS_select(&race_page);
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
