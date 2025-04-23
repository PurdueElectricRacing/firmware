#include "lcd.h"

#include "can_parse.h"
#include "nextion.h"
#include "pedals.h"
#include "common/faults/faults.h"
#include "common_defs.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "menu_system.h"
#include "main.h"

volatile page_t curr_page;                   // Current page displayed on the LCD
volatile page_t prev_page;                   // Previous page displayed on the LCD
uint16_t cur_fault_buf_ndx;                  // Current index in the fault buffer
volatile uint8_t fault_time_displayed;                // Amount of units of time that the fault has been shown to the driver
volatile uint16_t fault_buf[5] = {           // Buffer of displayed faults
    0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
};
char *errorText;                             // Pointer to data to display for the Error, Warning, and Critical Fault codes
extern pedal_values_t pedal_values;          // Global from pedals module for throttle display
extern q_handle_t q_fault_history;           // Global queue from fault library for fault history
extern volatile dashboard_input_state_t input_state;  // Global dashboard input states
extern brake_status_t brake_status;          // Global brake status struct
extern driver_pedal_profile_t driver_pedal_profiles[4];


// Driver Page Functions
void driverPageUpdate();
void driverMoveUp();
void driverMoveDown();
void driverSelect();

// Profile Page Functions
void pedalProfilesPageUpdate();
void pedalProfilesMoveUp();
void pedalProfilesMoveDown();
void pedalProfilesSelect();
void pedalProfilesSaveButton_CALLBACK();

// Cooling Page Functions
void coolingPageUpdate();
void coolingMoveUp();
void coolingMoveDown();
void coolingSelect();

// TV Page Functions
void tvPageUpdate();
void tvMoveUp();
void tvMoveDown();
void tvSelect();
void tvSetEnumNames();

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

// Warning/Error/Fatal Page Functions
void errorPageSelect();

// DAQ Logging Page Functions
void loggingPageUpdate();
void loggingSelect();

// Telemetry Functions
void raceTelemetryUpdate();
void sdcTelemetryUpdate();
void faultTelemetryUpdate();
void calibrationTelemetryUpdate();

// Utility Functions
void updateSDCStatus(uint8_t status, char *element);
void setFaultIndicator(uint16_t fault, char *element);

// Page handlers array stored in flash
const page_handler_t page_handlers[] = { // Order must match page_t enum
    [PAGE_RACE]        = {racePageUpdate, raceUpCallback, raceDownCallback, raceSelect, raceTelemetryUpdate},         // No move handlers, telemetry is passive
    [PAGE_COOLING]     = {coolingPageUpdate, coolingMoveUp, coolingMoveDown, coolingSelect, NULL},
    [PAGE_TVSETTINGS]  = {tvPageUpdate, tvMoveUp, tvMoveDown, tvSelect, NULL},
    [PAGE_FAULTS]      = {faultsPageUpdate, faultsMoveUp, faultsMoveDown, faultsSelect, faultTelemetryUpdate},
    [PAGE_SDCINFO]     = {NULL, NULL, NULL, NULL, sdcTelemetryUpdate},                          // SDCINFO is passive
    [PAGE_DRIVER]      = {driverPageUpdate, driverMoveUp, driverMoveDown, driverSelect, NULL},
    [PAGE_PROFILES]    = {pedalProfilesPageUpdate, pedalProfilesMoveUp, pedalProfilesMoveDown, pedalProfilesSelect, NULL},
    [PAGE_LOGGING]     = {loggingPageUpdate, NULL, NULL, loggingSelect, NULL},
    [PAGE_CALIBRATION] = {NULL, NULL, NULL, NULL, calibrationTelemetryUpdate},                  // Calibration is passive
    [PAGE_PREFLIGHT]   = {NULL, NULL, NULL, NULL, NULL},                                        // Preflight is passive
    [PAGE_WARNING]     = {NULL, NULL, NULL, errorPageSelect, NULL},                             // Error pages share a select handler
    [PAGE_ERROR]       = {NULL, NULL, NULL, errorPageSelect, NULL},                             // Error pages share a select handler
    [PAGE_FATAL]       = {NULL, NULL, NULL, errorPageSelect, NULL}                              // Error pages share a select handler
};


menu_element_t race_elements[] = {
    {
        .type          = ELEMENT_OPTION,
        .object_name   = RACE_TV_ON,
        .current_value = 0,
        .on_change     = sendTVParameters
    }
};

menu_page_t race_page = {
    .elements            = race_elements,
    .num_elements        = sizeof(race_elements) / sizeof(race_elements[0]),
    .current_index       = 0,
    .is_element_selected = false
};

typedef enum {
    COOLING_DT_FAN_INDEX  = 0,
    COOLING_DT_PUMP_INDEX = 1,
    COOLING_B_FAN_INDEX   = 2,
    COOLING_B_PUMP_INDEX  = 3,
} cooling_elements_t;

menu_element_t cooling_elements[] = {
    [COOLING_DT_FAN_INDEX] = {
        .type           = ELEMENT_VAL,
        .object_name    = DT_FAN_VAL,
        .current_value  = 0,
        .min_value      = 0,
        .max_value      = 100,
        .increment      = 25,
        .on_change      = sendCoolingParameters
    },
    [COOLING_DT_PUMP_INDEX] = {
        .type           = ELEMENT_OPTION,
        .object_name    = DT_PUMP_OP,
        .current_value  = 0,
        .on_change      = sendCoolingParameters
    },
    [COOLING_B_FAN_INDEX] = {
        .type           = ELEMENT_VAL,
        .object_name    = B_FAN_VAL,
        .current_value  = 0,
        .min_value      = 0,
        .max_value      = 100,
        .increment      = 25,
        .on_change      = sendCoolingParameters
    },
    [COOLING_B_PUMP_INDEX]= {
        .type           = ELEMENT_OPTION,
        .object_name    = B_PUMP_OP,
        .current_value  = 0,
        .on_change      = sendCoolingParameters
    }
};

menu_page_t cooling_page = {
    .elements            = cooling_elements,
    .num_elements        = sizeof(cooling_elements) / sizeof(cooling_elements[0]),
    .current_index       = 0,
    .is_element_selected = false
};

typedef enum {
    TV_VCU_PERMIT_INDEX  = 0,
    TV_VCU_CONTROL_INDEX = 1,
    TV_DEADBAND_INDEX    = 2,
    TV_P_GAIN_INDEX      = 3,
    TV_TORQUE_DROP_INDEX = 4,
    TV_MAX_SLIP_INDEX    = 5,
} tv_elements_t;

typedef enum {
    TV_PERMIT_NONE = 0,
    TV_PERMIT_VARIABLE= 1,
} tv_permit_modes_t;

typedef enum {
    TV_SPEED_CONTROL_MODE = 0,
    TV_TORQUE_CONTROL_MODE = 1
} tv_control_modes_t;


// TV Settings page menu elements
menu_element_t tv_elements[] = {
    [TV_VCU_PERMIT_INDEX] = {  // Not using the OPTION type here to display the text instead of the value
        .type           = ELEMENT_VAL,
        .object_name    = TV_PERMIT_MODE_TXT,
        .current_value  = TV_PERMIT_VARIABLE, // Default to variable
        .min_value      = TV_PERMIT_NONE,
        .max_value      = TV_PERMIT_VARIABLE,
        .increment      = 1,
        .on_change      = sendTVParameters
    },
    [TV_VCU_CONTROL_INDEX] = { // Not using the OPTION type here to display the text instead of the value
        .type           = ELEMENT_VAL,
        .object_name    = TV_CONTROL_MODE_TXT,
        .current_value  = TV_TORQUE_CONTROL_MODE, // Default to torque control
        .min_value      = TV_SPEED_CONTROL_MODE,
        .max_value      = TV_TORQUE_CONTROL_MODE,
        .increment      = 1,
        .on_change      = sendTVParameters
    },
    [TV_DEADBAND_INDEX] = {
        .type           = ELEMENT_VAL,
        .object_name    = TV_DEADBAND_TXT,
        .current_value  = TV_DEADBAND_DEFAULT_VALUE,
        .min_value      = 0,
        .max_value      = 25,
        .increment      = 1,
        .on_change      = sendTVParameters
    },
    [TV_P_GAIN_INDEX] = {
        .type           = ELEMENT_FLT,
        .object_name    = TV_P_GAIN_FLT,
        .current_value  = TV_P_GAIN_DEFAULT_VALUE,
        .min_value      = 0,
        .max_value      = 500,
        .increment      = 10,
        .on_change      = sendTVParameters
    },
    [TV_TORQUE_DROP_INDEX] = {
        .type           = ELEMENT_FLT,
        .object_name    = TV_TORQUE_DROP_FLT,
        .current_value  = TV_SLIP_DEFAULT_VALUE,
        .min_value      = 0,
        .max_value      = 100,
        .increment      = 2,
        .on_change      = sendTVParameters
    },
    [TV_MAX_SLIP_INDEX] = {
        .type           = ELEMENT_FLT,
        .object_name    = TV_MAX_SLIP_FLT,
        .current_value  = TV_TORQUE_DROP_DEFAULT_VALUE,
        .min_value      = 0,
        .max_value      = 100,
        .increment      = 2,
        .on_change      = sendTVParameters
    }
};

menu_page_t tv_page = {
    .elements            = tv_elements,
    .num_elements        = sizeof(tv_elements) / sizeof(tv_elements[0]),
    .current_index       = 0,
    .is_element_selected = false
};

menu_element_t faults_elements[] = {
    [0] =  {
        .type         = ELEMENT_BUTTON,
        .object_name  = FAULT1_BUTTON,
        .on_change    = faultsClearButton_CALLBACK // clear fault
    },
    [1] =  {
        .type         = ELEMENT_BUTTON,
        .object_name  = FAULT2_BUTTON,
        .on_change    = faultsClearButton_CALLBACK // clear fault
    },
    [2] =  {
        .type         = ELEMENT_BUTTON,
        .object_name  = FAULT3_BUTTON,
        .on_change    = faultsClearButton_CALLBACK // clear fault
    },
    [3] =  {
        .type         = ELEMENT_BUTTON,
        .object_name  = FAULT4_BUTTON,
        .on_change    = faultsClearButton_CALLBACK // clear fault
    },
    [4] =  {
        .type         = ELEMENT_BUTTON,
        .object_name  = FAULT5_BUTTON,
        .on_change    = faultsClearButton_CALLBACK // clear fault
    },
    [5] =  {
        .type         = ELEMENT_BUTTON,
        .object_name  = CLEAR_BUTTON,
        .on_change    = faultsClearButton_CALLBACK // clear all faults
    }
};

menu_page_t faults_page = {
    .elements            = faults_elements,
    .num_elements        = sizeof(faults_elements) / sizeof(faults_elements[0]),
    .current_index       = 0,
    .is_element_selected = false
};

menu_element_t driver_elements[] = {
    [0] = {
        .type          = ELEMENT_LIST,
        .object_name   = DRIVER1_LIST,
        .current_value = 1  // Default to driver 1
    },
    [1] = {
        .type          = ELEMENT_LIST,
        .object_name   = DRIVER2_LIST,
        .current_value = 0
    },
    [2] = {
        .type          = ELEMENT_LIST,
        .object_name   = DRIVER3_LIST,
        .current_value = 0
    },
    [3] = {
        .type          = ELEMENT_LIST,
        .object_name   = DRIVER4_LIST,
        .current_value = 0
    }
};

menu_page_t driver_page = {
    .elements            = driver_elements,
    .num_elements        = sizeof(driver_elements) / sizeof(driver_elements[0]),
    .current_index       = 0,
    .is_element_selected = false
};

typedef enum {
    PROFILE_BRAKE_INDEX    = 0,
    PROFILE_THROTTLE_INDEX = 1,
    PROFILE_SAVE_INDEX     = 2
} pedal_profile_elements_t;

// Profile page menu elements
menu_element_t pedal_profile_elements[] = {
    [PROFILE_BRAKE_INDEX] = {
        .type           = ELEMENT_FLT,
        .object_name    = PROFILE_BRAKE_FLT,
        .current_value  = 0,
        .min_value      = 0,
        .max_value      = 20,
        .increment      = 5,
    },
    [PROFILE_THROTTLE_INDEX] = {
        .type           = ELEMENT_FLT,
        .object_name    = PROFILE_THROTTLE_FLT,
        .current_value  = 0,
        .min_value      = 0,
        .max_value      = 20,
        .increment      = 5,
    },
    [PROFILE_SAVE_INDEX] = {
        .type           = ELEMENT_BUTTON,
        .object_name    = PROFILE_SAVE_BUTTON,
        .on_change      = pedalProfilesSaveButton_CALLBACK
    }
};

menu_page_t pedal_profile_page = {
    .elements            = pedal_profile_elements,
    .num_elements        = sizeof(pedal_profile_elements) / sizeof(pedal_profile_elements[0]),
    .current_index       = 0,
    .is_element_selected = false,
    .saved               = true,
};

typedef enum {
    LOGGING_OP_INDEX = 0
} logging_elements_t;

menu_element_t logging_elements[] = {
    [LOGGING_OP_INDEX] = {
        .type          = ELEMENT_OPTION,
        .object_name   = LOG_OP,
        .current_value = 0,
        .on_change     = sendLoggingParameters
    }
};

menu_page_t logging_page = {
    .elements            = logging_elements,
    .num_elements        = sizeof(logging_elements) / sizeof(logging_elements[0]),
    .current_index       = 0,
    .is_element_selected = false
};

// Initialize the LCD screen
// Preflight will be shown on power on, then reset to RACE
void initLCD() {
    curr_page = PAGE_RACE;
    prev_page = PAGE_PREFLIGHT;
    errorText = 0;
    NXT_setBaud(LCD_BAUD_RATE);
    NXT_setBrightness(100);

    readPedalProfiles();
    pedal_profile_page.saved = true;

    // Set page (leave preflight)
    updatePage();
}

/**
 * @brief Updates LCD display page based on encoder position
 *
 * Key behaviors:
 * - Updates current page based on encoder for non-error pages
 * - Maintains display of error pages when active
 */
void updatePage() {
    // Only update the encoder if we are on a "selectable" page
    bool is_error_page = (curr_page == PAGE_ERROR) || (curr_page == PAGE_WARNING) || (curr_page == PAGE_FATAL);

    // Only update prev_page for non-error pages
    if (!is_error_page) {
        curr_page = input_state.encoder_position;
        fault_time_displayed = 0;
    }

    // If we do not detect a page update, do nothing
    if (curr_page == prev_page) {
        return;
    }

    if (!is_error_page) { // Must come after redundant page check
        prev_page = curr_page;
    }

    // Set the page on display
    switch (curr_page) {
        case PAGE_PREFLIGHT:
            NXT_setPage(PREFLIGHT_STRING);
            break;
        case PAGE_RACE:
            NXT_setPage(RACE_STRING);
            break;
        case PAGE_COOLING:
            NXT_setPage(COOLING_STRING);
            break;
        case PAGE_TVSETTINGS:
            NXT_setPage(TVSETTINGS_STRING);
            break;
        case PAGE_FAULTS:
            NXT_setPage(FAULT_STRING);
            break;
        case PAGE_SDCINFO:
            NXT_setPage(SDCINFO_STRING);
            break;
        case PAGE_DRIVER:
            NXT_setPage(DRIVER_STRING);
            break;
        case PAGE_PROFILES:
            NXT_setPage(DRIVER_CONFIG_STRING);
            break;
        case PAGE_LOGGING:
            NXT_setPage(LOGGING_STRING);
            break;
        case PAGE_CALIBRATION:
            NXT_setPage(APPS_STRING);
            break;
        case PAGE_WARNING:
            NXT_setPage(WARN_STRING);
            NXT_setText(ERR_TXT, errorText);
            return;
        case PAGE_ERROR:
            NXT_setPage(ERR_STRING);
            NXT_setText(ERR_TXT, errorText);
            return;
        case PAGE_FATAL:
            NXT_setPage(FATAL_STRING);
            NXT_setText(ERR_TXT, errorText);
            return;
        default:
            curr_page = PAGE_PREFLIGHT; // something probably went wrong
            break;
    }

    // Bounds Check
    if (curr_page > PAGE_COUNT && curr_page < 0) {
        return;
    }

    // Call update handler if available
    if (page_handlers[curr_page].update != NULL) {
        page_handlers[curr_page].update();
    }

    lcdTxUpdate();
}

void moveUp() {
    // Bounds Check
    if (curr_page > PAGE_COUNT && curr_page < 0) {
        return;
    }

    if (page_handlers[curr_page].move_up != NULL) {
        page_handlers[curr_page].move_up();
    }
}

void moveDown() {
    // Bounds Check
    if (curr_page > PAGE_COUNT && curr_page < 0) {
        return;
    }

    if (page_handlers[curr_page].move_down != NULL) {
        page_handlers[curr_page].move_down();
    }
}

void selectItem() {
    // Bounds Check
    if (curr_page > PAGE_COUNT && curr_page < 0) {
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
    if (curr_page > PAGE_COUNT && curr_page < 0) {
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
    static uint8_t update_group = 0;

    if (curr_page != PAGE_CALIBRATION) {
        return;
    }

    //NXT_setValue(CALIBRATION_BRAKE_BAR, 0); // todo brake bar
    //NXT_setValue(CALIBRATION_THROTTLE_BAR, (int) ((filtered_pedals / 4095.0) * 100));

    NXT_setTextFormatted(CALIBRATION_BRAKE1_VAL, "%d",raw_adc_values.b1);
    NXT_setTextFormatted(CALIBRATION_BRAKE2_VAL, "%d",raw_adc_values.b2);
    NXT_setTextFormatted(CALIBRATION_THROTTLE1_VAL, "%d",raw_adc_values.t1);
    NXT_setTextFormatted(CALIBRATION_THROTTLE2_VAL, "%d",raw_adc_values.t2);

    uint16_t brake_diff = ABS(raw_adc_values.b1 - raw_adc_values.b2);
    uint16_t brake_dev = (brake_diff * 1000) / 4095.0;
    NXT_setValue(CALIBRATION_BRAKE_DEV_VAL, brake_dev);

    uint16_t throttle_diff = ABS(raw_adc_values.t1 - raw_adc_values.t2);
    uint16_t throttle_dev = (throttle_diff * 1000) / 4095.0;
    NXT_setValue(CALIBRATION_THROTTLE_DEV_VAL, throttle_dev);

    //uint16_t brake1_thresh = (raw_adc_values.brk1_thr / 4095.0) * 3.3 * 10;
    //uint16_t brake2_thresh = (raw_adc_values.brk2_thr / 4095.0) * 3.3 * 10;
    uint16_t brake1_thresh = 0;
    uint16_t brake2_thresh = 0;

    // 2 updates left in the queue, updaate infrequent items at 1/3 the rate
    switch (update_group) {
        case 0:
            // update brake stat
            if (brake_status.brake_status) {
                NXT_setText(CALIBRATION_BRAKE_STAT, "ON");
                NXT_setFontColor(CALIBRATION_BRAKE_STAT, GREEN);
            } else {
                NXT_setText(CALIBRATION_BRAKE_STAT, "OFF");
                NXT_setFontColor(CALIBRATION_BRAKE_STAT, WHITE);
            }
            update_group++;
            break;
        case 1:
            // update brake fail
            if (brake_status.brake_fail) {
                NXT_setText(CALIBRATION_BRAKE_FAIL, "FAIL");
                NXT_setFontColor(CALIBRATION_BRAKE_FAIL, RED);
            } else {
                NXT_setText(CALIBRATION_BRAKE_FAIL, "OK");
                NXT_setFontColor(CALIBRATION_BRAKE_FAIL, GREEN);
            }
            update_group++;
            break;
        case 2:
            // update bspd thresholds
            NXT_setValue(CALIBRATION_BRAKE1_THRESHOLD, brake1_thresh);
            NXT_setValue(CALIBRATION_BRAKE2_THRESHOLD, brake2_thresh);
            update_group = 0;
            break;
        default:
            update_group = 0;
            break;
    }
}


/**
 * @brief Sends TV parameters to TV using current values from tv_elements array
 */
void sendTVParameters() {
    SEND_DASHBOARD_VCU_PARAMETERS(
        tv_elements[TV_VCU_PERMIT_INDEX].current_value,
        tv_elements[TV_VCU_CONTROL_INDEX].current_value,
        tv_elements[TV_DEADBAND_INDEX].current_value,
        (uint16_t)tv_elements[TV_P_GAIN_INDEX].current_value,
        tv_elements[TV_TORQUE_DROP_INDEX].current_value,
        tv_elements[TV_MAX_SLIP_INDEX].current_value);
}

/**
 * @brief Sends Cooling parameters to PDU using current values from cooling_elements array.
 */
void sendCoolingParameters() {
  SEND_COOLING_DRIVER_REQUEST(cooling_elements[COOLING_B_PUMP_INDEX].current_value,
                              cooling_elements[COOLING_B_FAN_INDEX].current_value,
                              cooling_elements[COOLING_DT_PUMP_INDEX].current_value, // TODO: remove (deprecated)
                              0,
                              cooling_elements[COOLING_DT_FAN_INDEX].current_value);
}

/**
 * @brief Sends Logging parameters to DAQ using current values from logging_elements array.
 */
void sendLoggingParameters() {
    SEND_DASHBOARD_START_LOGGING(logging_elements[LOGGING_OP_INDEX].current_value);
}

/**
 * @brief Updates fault messages on LCD screen based on priority and timing
 *
 * Manages fault display rotation, processes new faults from queue, and updates
 * screen according to fault priorities and display timing requirements.
 */
void updateFaultDisplay() {
    if ((curr_page == PAGE_ERROR) || (curr_page == PAGE_WARNING) || (curr_page == PAGE_FATAL)) {
        fault_time_displayed++;
        uint8_t fault_time_percentage = (uint8_t)(100 - (fault_time_displayed / 8.0) * 100);
        NXT_setValue(TIME_BAR, fault_time_percentage);
        lcdTxUpdate();

        if (fault_time_displayed > 8) { // reset after 8 cycles
            fault_time_displayed = 0;
            curr_page = prev_page;
            prev_page = PAGE_PREFLIGHT;
            updatePage();
            lcdTxUpdate();
            return;
        }
    } else {
        fault_time_displayed = 0; // reset if not on error page
    }

    // No new fault to display
    if (qIsEmpty(&q_fault_history) && (most_recent_latched == 0xFFFF)) {
        return;
    }

    // Track if we alrady have this fault in the display buffer
    bool pageUpdateRequired = false;

    // Process up to 5 faults each time for now
    for (int i = 0; i < 5; i++) {
        uint16_t next_to_check = 0xFFFF;

        if (!qReceive(&q_fault_history, &next_to_check)) { // Break out if issue or the queue is empty
            break;
        }

        // Iterate through fault buffer for existance of fault already
        bool faultAlreadyInBuffer = false;
        for (int j = 0; j < 5; j++) {
            // This should be based off of the queue item not anything else
            if (fault_buf[j] == next_to_check) {
                faultAlreadyInBuffer = true;
                break;
            }
        }

        if (faultAlreadyInBuffer) {
            continue;
        }

        // Try all the slots for inserting the fault
        bool faultWasInserted = false;
        for (uint8_t k = 0; k < 5; k++) {
            // If fault is currently not in our fault buffer, replace it if the
            // current fault is cleared,
            //  or if the new fault has higher priority
            if (fault_buf[cur_fault_buf_ndx] != 0xFFFF) {
                if ((checkFault(fault_buf[cur_fault_buf_ndx]) == false) ||
                    (faultArray[next_to_check].priority > faultArray[fault_buf[cur_fault_buf_ndx]].priority)) {
                    fault_buf[cur_fault_buf_ndx] = next_to_check;
                    faultWasInserted = true;
                    pageUpdateRequired = true;
                    break;
                }
            } else {
                // Empty slot just insert
                fault_buf[cur_fault_buf_ndx] = next_to_check;
                faultWasInserted = true;
                pageUpdateRequired = true;
                break;
            }
            cur_fault_buf_ndx = (cur_fault_buf_ndx + 1) % 5;
        }

        // Put back in the queue if it wasn't processed
        if (!faultWasInserted) {
            qSendToBack(&q_fault_history, &next_to_check);
        }
    }

    // Set the alert page to show based on most_recent_latched
    if ((most_recent_latched != 0xFFFF)) {
        curr_page = faultArray[most_recent_latched].priority + 9;
        errorText = faultArray[most_recent_latched].screen_MSG;
        pageUpdateRequired = true;
    }

    // Update page if required
    if (pageUpdateRequired) {
        updatePage();
    }

    // Await next fault
    most_recent_latched = 0xFFFF;
}

/**
 * @brief Updates the color of fault text indicators on the faults page
 */
void faultTelemetryUpdate() {
    if (curr_page != PAGE_FAULTS) {
        return;
    }

    setFaultIndicator(fault_buf[0], FAULT1_TXT);
    setFaultIndicator(fault_buf[1], FAULT2_TXT);
    setFaultIndicator(fault_buf[2], FAULT3_TXT);
    setFaultIndicator(fault_buf[3], FAULT4_TXT);
    setFaultIndicator(fault_buf[4], FAULT5_TXT);
}

/**
 * @brief Updates the Shutdown Circuit (SDC) status display on the dashboard LCD
 *
 * Only executes if the current page is SDC info page.
 */
void sdcTelemetryUpdate() {
    static uint8_t update_group = 0U;
    if (curr_page != PAGE_SDCINFO) {
        return;
    }

    // cycle through the update groups
    update_group ^= 1;
    if (update_group) {
        updateSDCStatus(can_data.precharge_hb.IMD, SDC_IMD_STAT_TXT); // IMD from ABOX
        updateSDCStatus(can_data.precharge_hb.BMS, SDC_BMS_STAT_TXT);
        updateSDCStatus(!checkFault(ID_BSPD_LATCHED_FAULT), SDC_BSPD_STAT_TXT);
        updateSDCStatus(can_data.sdc_status.BOTS, SDC_BOTS_STAT_TXT);
        updateSDCStatus(can_data.sdc_status.inertia, SDC_INER_STAT_TXT);
        updateSDCStatus(can_data.sdc_status.c_estop, SDC_CSTP_STAT_TXT);
        updateSDCStatus(can_data.sdc_status.main, SDC_MAIN_STAT_TXT);
    } else {
        updateSDCStatus(can_data.sdc_status.r_estop, SDC_RSTP_STAT_TXT);
        updateSDCStatus(can_data.sdc_status.l_estop, SDC_LSTP_STAT_TXT);
        updateSDCStatus(can_data.sdc_status.HVD, SDC_HVD_STAT_TXT);
        updateSDCStatus(can_data.sdc_status.hub, SDC_RHUB_STAT_TXT);
        updateSDCStatus(can_data.sdc_status.TSMS, SDC_TSMS_STAT_TXT);
        updateSDCStatus(can_data.sdc_status.pchg_out, SDC_PCHG_STAT_TXT);
        //todo set first trip from latest change in the sdc
    }
}

// ! Helper function definitions

void errorPageSelect() {
    fault_time_displayed = 0;   // Reset fault timer first
    curr_page = prev_page;      // Return to previous page
    prev_page = PAGE_PREFLIGHT; // so select item doesnt't break
    updatePage();               // Important: Update the page before returning
    return;
}

void driverPageUpdate() {
    MS_refreshPage(&driver_page);
}

void driverMoveUp() {
    MS_moveUp(&driver_page);
}

void driverMoveDown() {
    MS_moveDown(&driver_page);
}

void driverSelect() {
    MS_select(&driver_page);
}

void pedalProfilesPageUpdate() {
    // Update displayed driver name
    int driver_index = MS_listGetSelected(&driver_page);
    if (driver_index < 0) {
        return;
    }

    switch (driver_index) {
        case 0:
            NXT_setText(PROFILE_CURRENT_TXT, DRIVER1_NAME);
            break;
        case 1:
            NXT_setText(PROFILE_CURRENT_TXT, DRIVER2_NAME);
            break;
        case 2:
            NXT_setText(PROFILE_CURRENT_TXT, DRIVER3_NAME);
            break;
        case 3:
            NXT_setText(PROFILE_CURRENT_TXT, DRIVER4_NAME);
            break;
    }

    pedal_profile_elements[PROFILE_BRAKE_INDEX].current_value =
        driver_pedal_profiles[driver_index].brake_travel_threshold;
    pedal_profile_elements[PROFILE_THROTTLE_INDEX].current_value =
        driver_pedal_profiles[driver_index].throttle_travel_threshold;

    // Update display and styling
    MS_refreshPage(&pedal_profile_page);
}

void pedalProfilesMoveUp() {
    MS_moveUp(&pedal_profile_page);

    // Update save status indicator on any value change
    if (!pedal_profile_page.is_element_selected) {
        NXT_setFontColor(PROFILE_STATUS_TXT, pedal_profile_page.saved ? GREEN : RED);
        NXT_setText(PROFILE_STATUS_TXT, pedal_profile_page.saved ? "SAVED" : "UNSAVED");
    }
}

void pedalProfilesMoveDown() {
    MS_moveDown(&pedal_profile_page);

    // Update save status indicator on any value change
    if (!pedal_profile_page.is_element_selected) {
        NXT_setFontColor(PROFILE_STATUS_TXT, pedal_profile_page.saved ? GREEN : RED);
        NXT_setText(PROFILE_STATUS_TXT, pedal_profile_page.saved ? "SAVED" : "UNSAVED");
    }
}

void pedalProfilesSelect() {
    // Handle other elements using menu system
    MS_select(&pedal_profile_page);

    // Mark as unsaved when values change
    if (pedal_profile_page.is_element_selected) {
        pedal_profile_page.saved = false;
        NXT_setFontColor(PROFILE_STATUS_TXT, RED);
        NXT_setText(PROFILE_STATUS_TXT, "UNSAVED");
    }
}

/**
 * @brief Saves the current pedal profile settings to permanent memory
 *        for the selected driver and updates the UI with the save status.
 */
void pedalProfilesSaveButton_CALLBACK() {
    int driver_index = MS_listGetSelected(&driver_page);
    // Save profile values
    driver_pedal_profiles[driver_index].brake_travel_threshold = pedal_profile_elements[PROFILE_BRAKE_INDEX].current_value;
    driver_pedal_profiles[driver_index].throttle_travel_threshold = pedal_profile_elements[PROFILE_THROTTLE_INDEX].current_value;

    if (PROFILE_WRITE_SUCCESS != writePedalProfiles()) {
        pedal_profile_page.saved = false;
        NXT_setFontColor(PROFILE_STATUS_TXT, RED);
        NXT_setText(PROFILE_STATUS_TXT, "FAILED");
    } else {
        pedal_profile_page.saved = true;
        NXT_setFontColor(PROFILE_STATUS_TXT, GREEN);
        NXT_setText(PROFILE_STATUS_TXT, "SAVED");
    }
}

void coolingPageUpdate() {
    MS_refreshPage(&cooling_page);
    NXT_setValue(DT_FAN_BAR, cooling_elements[COOLING_DT_FAN_INDEX].current_value);
    NXT_setValue(B_FAN_BAR, cooling_elements[COOLING_B_FAN_INDEX].current_value);

    if (can_data.coolant_out.stale) {
        NXT_setBackground(COOLING_CAN_STATUS, RED);
    } else {
        NXT_setBackground(COOLING_CAN_STATUS, GREEN);
    }
}

void coolingMoveUp() {
    MS_moveUp(&cooling_page);

    // Passively update the bar values
    if (cooling_page.is_element_selected) {
        NXT_setValue(DT_FAN_BAR, cooling_elements[COOLING_DT_FAN_INDEX].current_value);
        NXT_setValue(B_FAN_BAR, cooling_elements[COOLING_B_FAN_INDEX].current_value);
    }
}

void coolingMoveDown() {
    MS_moveDown(&cooling_page);

    // Passively update the bar values
    if (cooling_page.is_element_selected) {
        NXT_setValue(DT_FAN_BAR, cooling_elements[COOLING_DT_FAN_INDEX].current_value);
        NXT_setValue(B_FAN_BAR, cooling_elements[COOLING_B_FAN_INDEX].current_value);
    }
}

void coolingSelect() {
    MS_select(&cooling_page);
}

/**
 * @brief Callback function for coolant_in message that updates the cooling page
 *
 * @param msg_data_a Pointer to the parsed CAN message data
 */
void coolant_out_CALLBACK(CanParsedData_t* msg_data_a) {
    if (curr_page != PAGE_COOLING) {
      cooling_elements[COOLING_B_FAN_INDEX].current_value = msg_data_a->coolant_out.dt_fan;
      cooling_elements[COOLING_B_PUMP_INDEX].current_value = msg_data_a->coolant_out.dt_pump;
      cooling_elements[COOLING_DT_FAN_INDEX].current_value = msg_data_a->coolant_out.bat_fan;
      cooling_elements[COOLING_DT_PUMP_INDEX].current_value = msg_data_a->coolant_out.bat_pump;
    }
}

void tvSetEnumNames()
{
  switch (tv_elements[TV_VCU_PERMIT_INDEX].current_value) {
      case TV_PERMIT_NONE:
          NXT_setText(TV_PERMIT_MODE_TXT, "NONE");
          break;
      case TV_PERMIT_VARIABLE:
          NXT_setText(TV_PERMIT_MODE_TXT, "VARIABLE");
          break;
      default:
          NXT_setText(TV_PERMIT_MODE_TXT, "ERR");
          break;
  }

  switch (tv_elements[TV_VCU_CONTROL_INDEX].current_value) {
      case TV_SPEED_CONTROL_MODE:
          NXT_setText(TV_CONTROL_MODE_TXT, "SPEED");
          break;
      case TV_TORQUE_CONTROL_MODE:
          NXT_setText(TV_CONTROL_MODE_TXT, "TORQUE");
          break;
      default:
          NXT_setText(TV_CONTROL_MODE_TXT, "ERR");
          break;
  }
}

void tvPageUpdate() {
    MS_refreshPage(&tv_page);
    tvSetEnumNames();
}

void tvMoveUp() {
    MS_moveUp(&tv_page);
    tvSetEnumNames();
}

void tvMoveDown() {
    MS_moveDown(&tv_page);
    tvSetEnumNames();
}

void tvSelect() {
    MS_select(&tv_page);
    // TODO Race page TV settings
    //race_elements[0].current_value = tv_elements[TV_ENABLE_INDEX].current_value; // Sync TV settings
}

/**
 * @brief Updates the display of fault messages on the LCD screen
 *
 * Checks fault buffer entries and displays either the corresponding fault message
 * or "No Fault" message for each of the 5 fault text fields on screen
 */
void updateFaultMessages() {
    if (fault_buf[0] == 0xFFFF) {
        NXT_setText(FAULT1_TXT, FAULT_NONE_STRING);
    } else {
        NXT_setText(FAULT1_TXT, faultArray[fault_buf[0]].screen_MSG);
    }

    if (fault_buf[1] == 0xFFFF) {
        NXT_setText(FAULT2_TXT, FAULT_NONE_STRING);
    } else {
        NXT_setText(FAULT2_TXT, faultArray[fault_buf[1]].screen_MSG);
    }

    if (fault_buf[2] == 0xFFFF) {
        NXT_setText(FAULT3_TXT, FAULT_NONE_STRING);
    } else {
        NXT_setText(FAULT3_TXT, faultArray[fault_buf[2]].screen_MSG);
    }

    if (fault_buf[3] == 0xFFFF) {
        NXT_setText(FAULT4_TXT, FAULT_NONE_STRING);
    } else {
        NXT_setText(FAULT4_TXT, faultArray[fault_buf[3]].screen_MSG);
    }

    if (fault_buf[4] == 0xFFFF) {
        NXT_setText(FAULT5_TXT, FAULT_NONE_STRING);
    } else {
        NXT_setText(FAULT5_TXT, faultArray[fault_buf[4]].screen_MSG);
    }
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
    if (index < 0 || index > 4) {
        return;
    }

    if (fault_buf[index] == 0xFFFF) {
        return;
    }

    if (checkFault(fault_buf[index])) {  // Check if fault is not latched
        return;
    }

    // Shift the elements to the left
    for (int i = index; i < 4; i++) {
        fault_buf[i] = fault_buf[i + 1];
    }
    fault_buf[4] = 0xFFFF;
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
        for (int i = 4; i >= 0; i--) {  // Clear all faults which are not latched
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

    NXT_setValue(BRK_BAR, (int) ((pedal_values.brake / 4095.0) * 100)); // TODO BRK BAR
    NXT_setValue(THROT_BAR, (int) ((pedal_values.throttle / 4095.0) * 100));

    // update the speed
    if (can_data.rear_wheel_speeds.stale) {
        NXT_setText(SPEED, "S");
    } else {
        //uint16_t speed = can_data.rear_wheel_speeds.left_speed_mc * RPM_TO_MPH; // Convert to mph
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
        NXT_setTextFormatted(BATT_CURR, "%dA", current);  // Note: Changed 'V' to 'A' for current
    }

    // Update the motor temperature
    if (can_data.INVA_TEMPS.stale && can_data.INVB_TEMPS.stale) {
        NXT_setText(MOT_TEMP, "S");
        NXT_setText(MC_TEMP, "S");
    } else if (can_data.INVA_TEMPS.stale) {
        NXT_setText(MOT_TEMP, "SA");
        NXT_setText(MC_TEMP, "SA");
    } else if (can_data.INVB_TEMPS.stale) {
        NXT_setText(MOT_TEMP, "SB");
        NXT_setText(MC_TEMP, "SB");
    } else {
        uint8_t motor_temp = MAX(can_data.INVA_TEMPS.AMK_MotorTemp, can_data.INVB_TEMPS.AMK_MotorTemp) / 10;
        uint8_t controller_temp = MAX(can_data.INVA_TEMPS.AMK_IGBTTemp, can_data.INVB_TEMPS.AMK_IGBTTemp) / 10;

        NXT_setTextFormatted(MOT_TEMP, "%dC", motor_temp);
        NXT_setTextFormatted(MC_TEMP, "%dC", controller_temp);
    }

    if (can_data.INVA_CRIT.stale && can_data.INVA_CRIT.stale) {
        NXT_setText(AMK_MOTOR_OVERLOAD, "S");
    } else if (can_data.INVA_CRIT.stale) {
        NXT_setText(AMK_MOTOR_OVERLOAD, "SA");
    } else if (can_data.INVA_CRIT.stale) {
        NXT_setText(AMK_MOTOR_OVERLOAD, "SB");
    } else {
        uint16_t motor_overload = MAX(can_data.INVA_CRIT.AMK_DisplayOverloadMotor, can_data.INVB_CRIT.AMK_DisplayOverloadMotor) *10;
        //uint16_t motor_overload = 77;

        NXT_setTextFormatted(AMK_MOTOR_OVERLOAD, "%d%", motor_overload);
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
            case CAR_STATE_PRECHARGING:
                NXT_setFontColor(CAR_STAT, ORANGE);
                NXT_setText(CAR_STAT, "PRECHARGE");
                break;
            case CAR_STATE_ENERGIZED:
                NXT_setFontColor(CAR_STAT, ORANGE);
                NXT_setText(CAR_STAT, "ENERGIZED");
                break;
            case CAR_STATE_IDLE:
                NXT_setFontColor(CAR_STAT, INFO_GRAY);
                NXT_setText(CAR_STAT, "IDLE");
                break;
            case CAR_STATE_READY2DRIVE:
                NXT_setFontColor(CAR_STAT, GREEN);
                NXT_setText(CAR_STAT, "R2D");
                break;
            case CAR_STATE_ERROR:
                NXT_setFontColor(CAR_STAT, YELLOW);
                NXT_setText(CAR_STAT, "ERROR");
                break;
            case CAR_STATE_FATAL:
                NXT_setFontColor(CAR_STAT, RED);
                NXT_setText(CAR_STAT, "FATAL");
                break;
            case CAR_STATE_CONSTANT_TORQUE:
                NXT_setFontColor(CAR_STAT, GREEN);
                NXT_setText(CAR_STAT, "CONST TRQ");
                break;
            default:
                NXT_setFontColor(CAR_STAT, WHITE);
                NXT_setText(CAR_STAT, "UNKNOWN");
                break;
        }
    }
}

void raceUpCallback() {
    // turn on both pumps
    cooling_elements[COOLING_B_PUMP_INDEX].current_value = 1;
    cooling_elements[COOLING_DT_PUMP_INDEX].current_value = 1;

    sendCoolingParameters();
}

void raceDownCallback() {
    // turn off both pumps
    cooling_elements[COOLING_B_PUMP_INDEX].current_value = 0;
    cooling_elements[COOLING_DT_PUMP_INDEX].current_value = 0;

    sendCoolingParameters();
}

void raceSelect() {
    MS_select(&race_page);
    // TODO Race page TV settings
    //tv_elements[TV_ENABLE_INDEX].current_value = race_elements[0].current_value; // Sync TV settings
}

void loggingPageUpdate() {
    MS_refreshPage(&logging_page);

    if (logging_elements[LOGGING_OP_INDEX].current_value == 1) {
        NXT_setText(LOGGING_STATUS_TXT, "DAQ ON");
        NXT_setFontColor(LOGGING_STATUS_TXT, GREEN);
    } else {
        NXT_setText(LOGGING_STATUS_TXT, "DAQ OFF");
        NXT_setFontColor(LOGGING_STATUS_TXT, RED);
    }
}

void loggingSelect() {
    MS_select(&logging_page);

    if (logging_elements[LOGGING_OP_INDEX].current_value == 1) {
        NXT_setText(LOGGING_STATUS_TXT, "DAQ ON");
        NXT_setFontColor(LOGGING_STATUS_TXT, GREEN);
    } else {
        NXT_setText(LOGGING_STATUS_TXT, "DAQ OFF");
        NXT_setFontColor(LOGGING_STATUS_TXT, RED);
    }
}

/**
 * @brief Sets the color of a fault indicator element based on fault status
 *
 * @param fault The fault code to check (0xFFFF indicates no fault)
 * @param element Pointer to the display element to be colored
 */
void setFaultIndicator(uint16_t fault, char *element) {
    if (fault == 0xFFFF) {
        NXT_setFontColor(element, WHITE);
        return;
    }

    if (checkFault(fault)) {
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
void updateSDCStatus(uint8_t status, char *element) {
    if (status)
    {
        NXT_setBackground(element, GREEN);
    }
    else
    {
        NXT_setBackground(element, RED);
    }
}
