#include "lcd.h"

#include "can_parse.h"
#include "common/phal_F4_F7/spi/spi.h"
#include "nextion.h"
#include "pedals.h"
#include "common/faults/faults.h"
#include "common_defs.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

volatile page_t curr_page;              // Current page displayed on the LCD
volatile page_t prev_page;              // Previous page displayed on the LCD
uint16_t cur_fault_buf_ndx;             // Current index in the fault buffer
volatile uint16_t fault_buf[5] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};   // Buffer of displayed faults
bool sendFirsthalf;                     // Flag for sending data to data page
char *errorText;                        // Pointer to data to display for the Error, Warning, and Critical Fault codes
extern uint16_t filtered_pedals;        // Global from pedals module for throttle display
extern q_handle_t q_tx_can;             // Global queue for CAN tx
extern q_handle_t q_fault_history;      // Global queue from fault library for fault history
volatile cooling_page_t cooling;        // Data for the cooling page
volatile tv_settings_t tv_settings;     // Data for the tvsettings page
volatile driver_page_t driver_page;     // Data for the driver page
volatile fault_page_t fault_page;       // Data for the faults page
volatile profile_page_t profile_page;   // Data for the profile page
race_page_t race_page_data;             // Data for the race page
extern lcd_t lcd_data;
uint8_t fault_time_displayed;           // Amount of units of time that the fault has been shown to the driver
extern driver_profile_t driver_profiles[4];


// Driver Page Functions
void update_driver_page();
void move_up_driver();
void move_down_driver();
void select_driver();

// Profile Page Functions
void update_profile_page();
void move_up_profile();
void move_down_profile();
void select_profile();

// Cooling Page Functions
void update_cooling_page();
void move_up_cooling();
void move_down_cooling();
void select_cooling();

// TV Page Functions
void update_tv_page();
void move_up_tv();
void move_down_tv();
void select_tv();

// Faults Page Functions
void update_faults_page();
void move_up_faults();
void move_down_faults();
void select_fault();

// Race Page Functions
void update_race_page();
void update_race_page_group1();
void update_race_page_group2();
void select_race();

void select_error_page();

// Utility Functions
void updateSDCStatus(uint8_t status, char *element);
void setFaultIndicator(uint16_t fault, char *element);

// page handlers array must match page_t enum order exactly
page_handler_t page_handlers[] = {
    [PAGE_RACE]      = {update_race_page, NULL, NULL, select_race},
    [PAGE_COOLING]   = {update_cooling_page, move_up_cooling, move_down_cooling, select_cooling},
    [PAGE_TVSETTINGS]= {update_tv_page, move_up_tv, move_down_tv, select_tv},
    [PAGE_FAULTS]    = {update_faults_page, move_up_faults, move_down_faults, select_fault},
    [PAGE_SDCINFO]   = {NULL, NULL, NULL, NULL},  // SDCINFO is passive
    [PAGE_DRIVER]    = {update_driver_page, move_up_driver, move_down_driver, select_driver},
    [PAGE_PROFILES]  = {update_profile_page, move_up_profile, move_down_profile, select_profile},
    [PAGE_LOGGING]   = {NULL, NULL, NULL, NULL},  // TODO: Implement logging handlers
    [PAGE_DATA]      = {NULL, NULL, NULL, NULL}, // TODO Implement data handlers
    [PAGE_PREFLIGHT] = {NULL, NULL, NULL, NULL}, // Preflight is passive
    [PAGE_WARNING]   = {NULL, NULL, NULL, select_error_page}, // Error pages share a select handler
    [PAGE_ERROR]     = {NULL, NULL, NULL, select_error_page},  
    [PAGE_FATAL]     = {NULL, NULL, NULL, select_error_page}
};


// Call initially to ensure the LCD is initialized to the proper value -
// should be replaced with the struct prev page stuff eventually
int zeroEncoder(volatile int8_t* start_pos) {
    // ! uncomment this when encoder is implemented
    // Collect initial raw reading from encoder
    // uint8_t raw_enc_a = PHAL_readGPIO(ENC_A_GPIO_Port, ENC_A_Pin);
    // uint8_t raw_enc_b = PHAL_readGPIO(ENC_B_GPIO_Port, ENC_B_Pin);
    // uint8_t raw_res = (raw_enc_b | (raw_enc_a << 1));
    // *start_pos = raw_res;
    lcd_data.encoder_position = 0;

    // Set page (leave preflight)
    updatePage();
    return 1;
}

// Initialize the LCD screen
// Preflight will be shown on power on, then reset to RACE
void initLCD() {
    curr_page = PAGE_RACE;
    prev_page = PAGE_PREFLIGHT;
    errorText = 0;
    cooling = (cooling_page_t) {0, 0, 0, 0, 0, 0, 0, 0};
    sendFirsthalf = true;
    tv_settings = (tv_settings_t) {true, 0, 12, 100, 40 };
    fault_page_t fault_config = {FAULT1}; // Default to first fault
    set_baud(115200);
    set_brightness(100);

    readProfiles();
    profile_page.saved = true;
}

void updatePage() {
    // Only update the encoder if we are on a "selectable" page
    bool is_error_page = (curr_page == PAGE_ERROR) || (curr_page == PAGE_WARNING) || (curr_page == PAGE_FATAL);
    
    if (!is_error_page) {
        curr_page = lcd_data.encoder_position;
        fault_time_displayed = 0;
    }

    // If we do not detect a page update, do nothing
    if (curr_page == prev_page) {
        return;
    }

    // Only update prev_page for non-error pages
    if (!is_error_page) {
        prev_page = curr_page;
    }

    // Set the page on display
    switch (curr_page) {
        case PAGE_LOGGING: set_page(LOGGING_STRING); break;
        case PAGE_DRIVER: set_page(DRIVER_STRING); break;
        case PAGE_PROFILES: set_page(DRIVER_CONFIG_STRING); break;
        case PAGE_SDCINFO: set_page(SDCINFO_STRING); break;
        case PAGE_TVSETTINGS: set_page(TVSETTINGS_STRING); break;
        case PAGE_ERROR:
            set_page(ERR_STRING);
            set_text(ERR_TXT, errorText);
            return;
        case PAGE_WARNING:
            set_page(WARN_STRING);
            set_text(ERR_TXT, errorText);
            return;
        case PAGE_FATAL:
            set_page(FATAL_STRING);
            set_text(ERR_TXT, errorText);
            return;
        case PAGE_RACE: set_page(RACE_STRING); break;
        case PAGE_DATA: set_page(DATA_STRING); break;
        case PAGE_COOLING: set_page(COOLING_STRING); break;
        case PAGE_FAULTS: set_page(FAULT_STRING); break;
    }

    // Call update handler if available
    if (page_handlers[curr_page].update != NULL) {
        page_handlers[curr_page].update();
    }
}

void moveUp() {
    if (page_handlers[curr_page].move_up != NULL) {
        page_handlers[curr_page].move_up();
    }
}

void moveDown() {
    if (page_handlers[curr_page].move_down != NULL) {
        page_handlers[curr_page].move_down();
    }
}

void selectItem() {
    if (page_handlers[curr_page].select != NULL) {
        page_handlers[curr_page].select();
    }
}

void clear_fault(int index) {
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

void select_fault() {
    switch (fault_page.curr_hover) {
        case FAULT1:
            clear_fault(0);
            break;
        case FAULT2:
            clear_fault(1);
            break;
        case FAULT3:
            clear_fault(2);
            break;
        case FAULT4:
            clear_fault(3);
            break;
        case FAULT5:
            clear_fault(4);
            break;
        case CLEAR:
            for (int i = 4; i >= 0; i--) {
                clear_fault(i);
            }
            break;
    }
}

void update_data_page() {
    set_value(POW_LIM_BAR, 0);
    set_value(THROT_BAR, (int) ((filtered_pedals / 4095.0) * 100));
}

void updateDataPages() {
    switch (curr_page) {
        case PAGE_RACE:
            update_race_page();
            break;
        case PAGE_DATA:
            update_data_page();
            break;
    }
}

void sendTVParameters() {
    SEND_DASHBOARD_TV_PARAMETERS(tv_settings.tv_enable_selected, tv_settings.tv_deadband_val, tv_settings.tv_intensity_val, tv_settings.tv_p_val);
}

void updateFaultDisplay() {
    if ((curr_page == PAGE_ERROR || (curr_page == PAGE_WARNING) || (curr_page == PAGE_FATAL)))
    {
        if (++fault_time_displayed > 8)
        {
            curr_page = prev_page;
            prev_page = PAGE_PREFLIGHT;
            updatePage();
        }
    }
    else
    {
        fault_time_displayed = 0;
    }

    // No new fault to display
    if (qIsEmpty(&q_fault_history) && (most_recent_latched == 0xFFFF))
    {
        return;
    }

    // Track if we alrady have this fault in the display buffer
    bool faultAlreadyInBuffer = false;
    bool pageUpdateRequired = false;
    bool faultWasInserted = false;

    // Process up to 5 faults each time for now
    for (int i = 0; i < 5; i++)
    {
        faultAlreadyInBuffer = false;
        uint16_t next_to_check = 0xFFFF;
        faultWasInserted = false;

        if (qReceive(&q_fault_history, &next_to_check))
        {
            // Iterate through fault buffer for existance of fault already
            for (int j = 0; j < 5; j++)
            {
                // This should be based off of the queue item not anything else
                if (fault_buf[j] == next_to_check)
                {
                    faultAlreadyInBuffer = true;
                    break;
                }
            }

            // New fault to add to the display, if room
            if (false == faultAlreadyInBuffer)
            {
                // try all the slots for inserting the fault
                for (uint8_t k = 0; k < 5; k++)
                {
                    // If fault is currently not in our fault buffer, replace it if the current fault is cleared,
                    //  or if the new fault has higher priority
                    if (fault_buf[cur_fault_buf_ndx] != 0xFFFF)
                    {
                        if ((checkFault(fault_buf[cur_fault_buf_ndx]) == false ) ||
                        (faultArray[next_to_check].priority > faultArray[fault_buf[cur_fault_buf_ndx]].priority))
                        {
                            fault_buf[cur_fault_buf_ndx] = next_to_check;
                            faultWasInserted = true;
                            pageUpdateRequired = true;
                            break;
                        }
                    }
                    else
                    {
                        // Empty slot just insert
                        fault_buf[cur_fault_buf_ndx] = next_to_check;
                        faultWasInserted = true;
                        pageUpdateRequired = true;
                        break;
                    }
                    cur_fault_buf_ndx = (cur_fault_buf_ndx + 1) % 5;
                }

                // Put back in the queue if it wasn't processed
                if (false == faultWasInserted)
                {
                    qSendToBack(&q_fault_history, &next_to_check);
                }

            }
        }
        else
        {
            // Break out if issue or the queue is empty
            break;
        }

    }

    // Set the alert page to show based on most_recent_latched
    if ((most_recent_latched != 0xFFFF))
    {
        curr_page = faultArray[most_recent_latched].priority + 9;
        errorText = faultArray[most_recent_latched].screen_MSG;
        pageUpdateRequired = true;
    }

    // Update page if required
    if (pageUpdateRequired)
    {
        updatePage();
    }

    // Await next fault
    most_recent_latched = 0xFFFF;
}

void updateFaultPageIndicators() {
    static uint8_t update_group = 0U;
    if (curr_page != PAGE_FAULTS) {
        return;
    }

    update_group++;
    switch (update_group) { // Split into two update groups, otherwise the fifth element does not get updated
        case 1:
            setFaultIndicator(fault_buf[0], FAULT_1_TXT);
            setFaultIndicator(fault_buf[1], FAULT_2_TXT);
            setFaultIndicator(fault_buf[2], FAULT_3_TXT);
            break;
        case 2:
            setFaultIndicator(fault_buf[3], FAULT_4_TXT);
            setFaultIndicator(fault_buf[4], FAULT_5_TXT);
            update_group = 0U;
            break;
        default:
            update_group = 0U;
            break;
    }
}

void updateSDCDashboard() {
    static uint8_t update_group = 0U;
    if (curr_page != PAGE_SDCINFO) {
        return;
    }

    // cycle through the update groups (5 elements each)
    update_group++;
    switch (update_group) {
        case 1:
            updateSDCStatus(can_data.precharge_hb.IMD, SDC_IMD_STAT_TXT); // IMD from ABOX
            updateSDCStatus(can_data.precharge_hb.BMS, SDC_BMS_STAT_TXT);
            updateSDCStatus(!checkFault(ID_BSPD_LATCHED_FAULT), SDC_BSPD_STAT_TXT);
            updateSDCStatus(can_data.sdc_status.BOTS, SDC_BOTS_STAT_TXT);
            updateSDCStatus(can_data.sdc_status.inertia, SDC_INER_STAT_TXT);
            break;
        case 2:
            updateSDCStatus(can_data.sdc_status.c_estop, SDC_CSTP_STAT_TXT);
            updateSDCStatus(can_data.sdc_status.main, SDC_MAIN_STAT_TXT);
            updateSDCStatus(can_data.sdc_status.r_estop, SDC_RSTP_STAT_TXT);
            updateSDCStatus(can_data.sdc_status.l_estop, SDC_LSTP_STAT_TXT);
            updateSDCStatus(can_data.sdc_status.HVD, SDC_HVD_STAT_TXT);
            break;
        case 3:
            updateSDCStatus(can_data.sdc_status.hub, SDC_RHUB_STAT_TXT);
            updateSDCStatus(can_data.sdc_status.TSMS, SDC_TSMS_STAT_TXT);
            updateSDCStatus(can_data.sdc_status.pchg_out, SDC_PCHG_STAT_TXT);
            //todo set first trip from latest change in the sdc
            update_group = 0U;
            break;
        default:
            update_group = 0U;
            break;
    }
}

// ! Helper function definitions

void select_error_page() {
    fault_time_displayed = 0;   // Reset fault timer first
    curr_page = prev_page;      // Return to previous page 
    prev_page = PAGE_PREFLIGHT; // so select item doesnt't break
    updatePage();               // Important: Update the page before returning
    return;
}

void update_driver_page() {
    driver_page.curr_hover = DRIVER1;
    set_background(DRIVER1_TXT, TV_HOVER_BG);
    set_background(DRIVER2_TXT, TV_BG);
    set_background(DRIVER3_TXT, TV_BG);
    set_background(DRIVER4_TXT, TV_BG);

    if (driver_page.curr_select == DRIVER1)
    {
        set_value(DRIVER1_OP, 1);
    }
    else
    {
        set_value(DRIVER1_OP, 0);
    }

    if (driver_page.curr_select == DRIVER2)
    {
        set_value(DRIVER2_OP, 1);
    }
    else
    {
        set_value(DRIVER2_OP, 0);
    }

    if (driver_page.curr_select == DRIVER3)
    {
        set_value(DRIVER3_OP, 1);
    }
    else
    {
        set_value(DRIVER3_OP, 0);
    }

    if (driver_page.curr_select == DRIVER4)
    {
        set_value(DRIVER4_OP, 1);
    }
    else
    {
        set_value(DRIVER4_OP, 0);
    }
}

void move_up_driver() {
    switch (driver_page.curr_hover) {
        case DRIVER1:
            driver_page.curr_hover = DRIVER4;
            set_background(DRIVER4_TXT, TV_HOVER_BG);
            set_background(DRIVER1_TXT, TV_BG);
            break;
        case DRIVER2:
            driver_page.curr_hover = DRIVER1;
            set_background(DRIVER1_TXT, TV_HOVER_BG);
            set_background(DRIVER2_TXT, TV_BG);
            break;
        case DRIVER3:
            driver_page.curr_hover = DRIVER2;
            set_background(DRIVER2_TXT, TV_HOVER_BG);
            set_background(DRIVER3_TXT, TV_BG);
            break;
        case DRIVER4:
            driver_page.curr_hover = DRIVER3;
            set_background(DRIVER3_TXT, TV_HOVER_BG);
            set_background(DRIVER4_TXT, TV_BG);
            break;
    }
}

void move_down_driver() {
    switch (driver_page.curr_hover) {
        case DRIVER1:
            driver_page.curr_hover = DRIVER2;
            set_background(DRIVER2_TXT, TV_HOVER_BG);
            set_background(DRIVER1_TXT, TV_BG);
            break;
        case DRIVER2:
            driver_page.curr_hover = DRIVER3;
            set_background(DRIVER3_TXT, TV_HOVER_BG);
            set_background(DRIVER2_TXT, TV_BG);
            break;
        case DRIVER3:
            driver_page.curr_hover = DRIVER4;
            set_background(DRIVER4_TXT, TV_HOVER_BG);
            set_background(DRIVER3_TXT, TV_BG);
            break;
        case DRIVER4:
            driver_page.curr_hover = DRIVER1;
            set_background(DRIVER1_TXT, TV_HOVER_BG);
            set_background(DRIVER4_TXT, TV_BG);
            break;
    }
}

void select_driver() {
    switch(driver_page.curr_hover)
    {
        case DRIVER1:
            driver_page.curr_select = DRIVER1;
            set_value(DRIVER1_OP, 1);
            set_value(DRIVER2_OP, 0);
            set_value(DRIVER3_OP, 0);
            set_value(DRIVER4_OP, 0);
            break;
        case DRIVER2:
            driver_page.curr_select = DRIVER2;
            set_value(DRIVER1_OP, 0);
            set_value(DRIVER2_OP, 1);
            set_value(DRIVER3_OP, 0);
            set_value(DRIVER4_OP, 0);
            break;
        case DRIVER3:
            driver_page.curr_select = DRIVER3;
            set_value(DRIVER1_OP, 0);
            set_value(DRIVER2_OP, 0);
            set_value(DRIVER3_OP, 1);
            set_value(DRIVER4_OP, 0);
            break;
        case DRIVER4:
            driver_page.curr_select = DRIVER4;
            set_value(DRIVER1_OP, 0);
            set_value(DRIVER2_OP, 0);
            set_value(DRIVER3_OP, 0);
            set_value(DRIVER4_OP, 1);
            break;
    }
}

void update_profile_page() {
    profile_page.curr_hover = BRAKE_HOVER;

    switch (driver_page.curr_select) {
        case DRIVER1:
            set_text(PROFILE_CURRENT_TXT, DRIVER1_NAME);
            break;
        case DRIVER2:
            set_text(PROFILE_CURRENT_TXT, DRIVER2_NAME);
            break;
        case DRIVER3:
            set_text(PROFILE_CURRENT_TXT, DRIVER3_NAME);
            break;
        case DRIVER4:
            set_text(PROFILE_CURRENT_TXT, DRIVER4_NAME);
            break;
    }

    // Set the initial background color
    set_background(PROFILE_BRAKE_FLT, TV_HOVER_BG);
    set_background(PROFILE_THROTTLE_FLT, TV_BG);
    set_background(PROFILE_SAVE_TXT, BLACK);

    readProfiles();

    profile_page.driver_id = (uint8_t)driver_page.curr_select;
    profile_page.brake_val = driver_profiles[profile_page.driver_id].brake_travel_threshold;
    profile_page.throttle_val = driver_profiles[profile_page.driver_id].throttle_travel_threshold;

    set_value(PROFILE_BRAKE_FLT, profile_page.brake_val);
    set_value(PROFILE_THROTTLE_FLT, profile_page.throttle_val);

    profile_page.driver_id = (uint8_t)driver_page.curr_select;
}

void move_up_profile() {
    switch (profile_page.curr_hover) {
        case BRAKE_HOVER:
            profile_page.curr_hover = SAVE_HOVER;
            set_background(PROFILE_SAVE_TXT, TV_HOVER_BG);
            set_background(PROFILE_BRAKE_FLT, TV_BG);
            break;
        case BRAKE_SELECTED:
            profile_page.saved = false;
            if (profile_page.brake_val >= 20) {
                profile_page.brake_val = 0;
            } else {
                profile_page.brake_val += 5;
            }
            set_value(PROFILE_BRAKE_FLT, profile_page.brake_val);
            break;
        case THROTTLE_HOVER:
            profile_page.curr_hover = BRAKE_HOVER;
            set_background(PROFILE_BRAKE_FLT, TV_HOVER_BG);
            set_background(PROFILE_THROTTLE_FLT, TV_BG);
            break;
        case THROTTLE_SELECTED:
            profile_page.saved = false;
            if (profile_page.throttle_val >= 20) {
              profile_page.throttle_val = 0;
            } else {
              profile_page.throttle_val += 5;
            }
            set_value(PROFILE_THROTTLE_FLT, profile_page.throttle_val);
            break;
        case SAVE_HOVER:
            profile_page.curr_hover = THROTTLE_HOVER;
            set_background(PROFILE_THROTTLE_FLT, TV_HOVER_BG);
            set_background(PROFILE_SAVE_TXT, BLACK);
            break;
    }
    set_font_color(PROFILE_STATUS_TXT, profile_page.saved ? GREEN : RED);
    set_text(PROFILE_STATUS_TXT, profile_page.saved ? "SAVED" : "UNSAVED");
}

void move_down_profile() {
    switch (profile_page.curr_hover) {
        case BRAKE_HOVER:
            profile_page.curr_hover = THROTTLE_HOVER;
            set_background(PROFILE_THROTTLE_FLT, TV_HOVER_BG);
            set_background(PROFILE_BRAKE_FLT, TV_BG);
            break;
        case BRAKE_SELECTED:
            profile_page.saved = false;
            if (profile_page.brake_val <= 0) {
              profile_page.brake_val = 20;
            } else {
              profile_page.brake_val -= 5;
            }
            set_value(PROFILE_BRAKE_FLT, profile_page.brake_val);
            break;
        case THROTTLE_HOVER:
            profile_page.curr_hover = SAVE_HOVER;
            set_background(PROFILE_SAVE_TXT, TV_HOVER_BG);
            set_background(PROFILE_THROTTLE_FLT, TV_BG);
            break;
        case THROTTLE_SELECTED:
            profile_page.saved = false;
            if (profile_page.throttle_val <= 0) {
              profile_page.throttle_val = 20;
            } else {
              profile_page.throttle_val -= 5;
            }
            set_value(PROFILE_THROTTLE_FLT, profile_page.throttle_val);
            break;
        case SAVE_HOVER:
            profile_page.curr_hover = BRAKE_HOVER;
            set_background(PROFILE_BRAKE_FLT, TV_HOVER_BG);
            set_background(PROFILE_SAVE_TXT, BLACK);
            break;
    }
    set_font_color(PROFILE_STATUS_TXT, profile_page.saved ? GREEN : RED);
    set_text(PROFILE_STATUS_TXT, profile_page.saved ? "SAVED" : "UNSAVED");
}

void select_profile() {
    switch (profile_page.curr_hover) {
        case BRAKE_HOVER:
            profile_page.curr_hover = BRAKE_SELECTED;
            set_background(PROFILE_BRAKE_FLT, ORANGE);
            break;
        case BRAKE_SELECTED:
            profile_page.curr_hover = BRAKE_HOVER;
            set_background(PROFILE_BRAKE_FLT, TV_HOVER_BG);
            break;
        case THROTTLE_HOVER:
            profile_page.curr_hover = THROTTLE_SELECTED;
            set_background(PROFILE_THROTTLE_FLT, ORANGE);
            break;
        case THROTTLE_SELECTED:
            profile_page.curr_hover = THROTTLE_HOVER;
            set_background(PROFILE_THROTTLE_FLT, TV_HOVER_BG);
            break;
        case SAVE_HOVER:
            // Modify the driver profile
            driver_profiles[profile_page.driver_id].brake_travel_threshold = profile_page.brake_val;
            driver_profiles[profile_page.driver_id].throttle_travel_threshold = profile_page.throttle_val;

            if (PROFILE_WRITE_SUCCESS != writeProfiles()) {
                profile_page.saved = false;
                set_font_color(PROFILE_STATUS_TXT, RED);
                set_text(PROFILE_STATUS_TXT, "FAILED");
            } else {
                profile_page.saved = true;
                set_font_color(PROFILE_STATUS_TXT, GREEN);
                set_text(PROFILE_STATUS_TXT, "SAVED");
            }
            break;
    }
}

void update_cooling_page() {
    cooling.curr_hover = DT_FAN_HOVER;                                     // Set hover
    set_background(DT_FAN_TXT, COOLING_HOVER_BG);         // Set t2 with cooling hover
    set_value(DT_FAN_BAR, cooling.d_fan_val);                   // Set progress bar for j0
    //set_value(DT_FAN_VAL, NXT_FONT_COLOR, COOLING_BAR_BG);                         // Set color for t8 (background of bar?)
    set_textf(DT_FAN_VAL, "%d", cooling.d_fan_val);  // Set fan value for t8

    // Set drivetrain pump selector color
    if (cooling.d_pump_selected) {
        set_font_color(DT_PUMP_OP, SETTINGS_UV_SELECT);
    }
    else {
        set_background(DT_PUMP_OP, COOLING_HOVER_BG);
    }

    // Set drivetrain pump selector status
    set_value(DT_PUMP_OP, cooling.d_pump_selected);

    // Set Battery fan c3 (Pump 1?)
    // todo: Why is this here?
    if (cooling.b_fan2_selected) {
        set_font_color(B_FAN2_OP, SETTINGS_UV_SELECT);
    }
    else {
        set_background(B_FAN2_OP, COOLING_HOVER_BG);
    }

    // Set value for c3 battery pump 1
    set_value(B_FAN2_OP, cooling.b_fan2_selected);
    if (cooling.b_pump_selected) {
        set_font_color(B_PUMP_OP, SETTINGS_UV_SELECT);
    }
    else {
        set_background(B_PUMP_OP, COOLING_HOVER_BG);
    }

    // Set Battery Pump 2 value
    set_value(B_PUMP_OP, cooling.b_pump_selected);

    // Set battery fan bar, text, color
    set_value(B_FAN1_BAR, cooling.b_fan_val);
    set_textf(B_FAN1_VAL, "%d", cooling.b_fan_val);
    set_font_color(B_FAN1_VAL, WHITE);
}

void move_up_cooling() {
    switch (cooling.curr_hover) {
        case DT_FAN_HOVER:
            cooling.curr_hover = PUMP_HOVER;
            set_background(DT_FAN_TXT, COOLING_BG);
            set_font_color(DT_FAN_TXT, COOLING_FG);
            set_background(B_PUMP_TXT, COOLING_HOVER_BG);
            set_font_color(B_PUMP_TXT, COOLING_HOVER_FG);
            break;
        case DT_PUMP_HOVER:
            cooling.curr_hover = DT_FAN_HOVER;
            set_background(DT_PUMP_TXT, COOLING_BG);
            set_font_color(DT_PUMP_TXT, COOLING_FG);
            set_background(DT_FAN_TXT, COOLING_HOVER_BG);
            set_font_color(DT_FAN_TXT, COOLING_HOVER_FG);
            break;
        case FAN1_HOVER:
            cooling.curr_hover = DT_PUMP_HOVER;
            set_background(B_FAN1_TXT, COOLING_BG);
            set_font_color(B_FAN1_TXT, COOLING_FG);
            set_background(DT_PUMP_TXT, COOLING_HOVER_BG);
            set_font_color(DT_PUMP_TXT, COOLING_HOVER_FG);
            break;
        case FAN2_HOVER:
            cooling.curr_hover = FAN1_HOVER;
            set_background(B_FAN2_TXT, COOLING_BG);
            set_font_color(B_FAN2_TXT, COOLING_FG);
            set_background(B_FAN1_TXT, COOLING_HOVER_BG);
            set_font_color(B_FAN1_TXT, COOLING_HOVER_FG);
            break;
        case PUMP_HOVER:
            cooling.curr_hover = FAN2_HOVER;
            set_background(B_PUMP_TXT, COOLING_BG);
            set_font_color(B_PUMP_TXT, COOLING_FG);
            set_background(B_FAN2_TXT, COOLING_HOVER_BG);
            set_font_color(B_FAN2_TXT, COOLING_HOVER_FG);
            break;
        case DT_FAN_SELECT:
            cooling.d_fan_val /= BAR_INTERVAL;
            cooling.d_fan_val *= BAR_INTERVAL;
            cooling.d_fan_val = (cooling.d_fan_val == 100) ? 0 : cooling.d_fan_val + BAR_INTERVAL;
            set_value(DT_FAN_BAR, cooling.d_fan_val);
            set_textf(DT_FAN_VAL, "%d", cooling.d_fan_val);
            //set_value(DT_FAN_VAL, NXT_FONT_COLOR, BLACK);
            break;
        case FAN1_SELECT:
            cooling.b_fan_val /= BAR_INTERVAL;
            cooling.b_fan_val *= BAR_INTERVAL;
            cooling.b_fan_val = (cooling.b_fan_val == 100) ? 0 : cooling.b_fan_val + BAR_INTERVAL;
            set_value(B_FAN1_BAR, cooling.b_fan_val);
            set_textf(B_FAN1_VAL, "%d", cooling.b_fan_val);
            //set_value(B_FAN1_VAL, NXT_FONT_COLOR, BLACK);
            break;
    }
}

void move_down_cooling() {
    char parsed_value[3] = "\0";
    switch (cooling.curr_hover) {
        case DT_FAN_HOVER:
            cooling.curr_hover = DT_PUMP_HOVER;
            set_background(DT_FAN_TXT, COOLING_BG);
            set_font_color(DT_FAN_TXT, COOLING_FG);
            set_background(DT_PUMP_TXT, COOLING_HOVER_BG);
            set_font_color(DT_PUMP_TXT, COOLING_HOVER_FG);
            break;
        case DT_PUMP_HOVER:
            cooling.curr_hover = FAN1_HOVER;
            set_background(DT_PUMP_TXT, COOLING_BG);
            set_font_color(DT_PUMP_TXT, COOLING_FG);
            set_background(B_FAN1_TXT, COOLING_HOVER_BG);
            set_font_color(B_FAN1_TXT, COOLING_HOVER_FG);
            break;
        case FAN1_HOVER:
            cooling.curr_hover = FAN2_HOVER;
            set_background(B_FAN1_TXT, COOLING_BG);
            set_font_color(B_FAN1_TXT, COOLING_FG);
            set_background(B_FAN2_TXT, COOLING_HOVER_BG);
            set_font_color(B_FAN2_TXT, COOLING_HOVER_FG);
            break;
        case FAN2_HOVER:
            cooling.curr_hover = PUMP_HOVER;
            set_background(B_FAN2_TXT, COOLING_BG);
            set_font_color(B_FAN2_TXT, COOLING_FG);
            set_background(B_PUMP_TXT, COOLING_HOVER_BG);
            set_font_color(B_PUMP_TXT, COOLING_HOVER_FG);
            break;
        case PUMP_HOVER:
            cooling.curr_hover = DT_FAN_HOVER;
            set_background(B_PUMP_TXT, COOLING_BG);
            set_font_color(B_PUMP_TXT, COOLING_FG);
            set_background(DT_FAN_TXT, COOLING_HOVER_BG);
            set_font_color(DT_FAN_TXT, COOLING_HOVER_FG);
            break;
        case DT_FAN_SELECT:
            cooling.d_fan_val /= BAR_INTERVAL;
            cooling.d_fan_val *= BAR_INTERVAL;
            cooling.d_fan_val = (cooling.d_fan_val == 0) ? 100 : cooling.d_fan_val - BAR_INTERVAL;
            set_value(DT_FAN_BAR, cooling.d_fan_val);
            set_textf(DT_FAN_VAL, "%d", cooling.d_fan_val);
            break;
        case FAN1_SELECT:
            cooling.b_fan_val /= BAR_INTERVAL;
            cooling.b_fan_val *= BAR_INTERVAL;
            cooling.b_fan_val = (cooling.b_fan_val == 0) ? 100 : cooling.b_fan_val - BAR_INTERVAL;
            set_value(B_FAN1_BAR, cooling.b_fan_val);
            set_textf(B_FAN1_VAL, "%d", cooling.b_fan_val);
            break;
    }
}

void select_cooling() {
    switch (cooling.curr_hover) {
    case DT_FAN_HOVER:
        // cooling.d_fan_selected = !cooling.d_fan_selected;
        // set_value(DT_FAN_BAR, NXT_VALUE, cooling.d_fan_selected);
        cooling.curr_hover = DT_FAN_SELECT;
        set_value(DT_FAN_TXT, COOLING_BG);
        set_background(DT_FAN_BAR, WHITE);
        //set_value(DT_FAN_BAR, NXT_FONT_COLOR, BLACK);
        return;
    case DT_PUMP_HOVER:
        cooling.d_pump_selected = !cooling.d_pump_selected;
        if (cooling.d_pump_selected) {
            set_font_color(DT_PUMP_OP, SETTINGS_UV_SELECT);
            set_background(DT_PUMP_OP, COOLING_BG);
        }
        else {
            set_background(DT_PUMP_OP, COOLING_HOVER_BG);
        }
        set_value(DT_PUMP_OP, cooling.d_pump_selected);
        break;
    case FAN1_HOVER:
        // cooling.b_fan1_selected = !cooling.b_fan1_selected;
        // set_value(B_FAN1_BAR, NXT_VALUE, cooling.b_fan1_selected);
        cooling.curr_hover = FAN1_SELECT;
        set_value(B_FAN1_TXT, COOLING_BG);
        set_background(B_FAN1_BAR, WHITE);
        //set_value(B_FAN1_BAR, NXT_FONT_COLOR, BLACK);
        break;
    case FAN2_HOVER:
        cooling.b_fan2_selected = !cooling.b_fan2_selected;
        if (cooling.b_fan2_selected) {
            set_font_color(B_FAN2_OP, SETTINGS_UV_SELECT);
            set_background(B_FAN2_OP, COOLING_BG);
        }
        else {
            set_background(B_FAN2_OP, COOLING_HOVER_BG);
        }
        set_value(B_FAN2_OP, cooling.b_fan2_selected);
        break;
    case PUMP_HOVER:
        cooling.b_pump_selected = !cooling.b_pump_selected;
        if (cooling.b_pump_selected) {
        set_font_color(B_PUMP_OP, SETTINGS_UV_SELECT);
        set_background(B_PUMP_OP, COOLING_BG);
        }
        else {
            set_background(B_PUMP_OP, COOLING_HOVER_BG);
        }
        set_value(B_PUMP_OP, cooling.b_pump_selected);
        break;
    case DT_FAN_SELECT:
        cooling.curr_hover = DT_FAN_HOVER;
        set_value(DT_FAN_TXT, COOLING_HOVER_BG);
        set_background(DT_FAN_BAR, COOLING_BAR_BG);
        set_font_color(DT_FAN_BAR, COOLING_BAR_FG);
        //set_value(DT_FAN_VAL, NXT_FONT_COLOR, COOLING_BAR_BG);
        break;
    case FAN1_SELECT:
        cooling.curr_hover = FAN1_HOVER;
        set_value(B_FAN1_TXT, COOLING_HOVER_BG);
        set_background(B_FAN1_BAR, COOLING_BAR_BG);
        set_font_color(B_FAN1_BAR, COOLING_BAR_FG);
        //set_value(B_FAN1_VAL, NXT_FONT_COLOR, COOLING_BAR_BG);
        break;
    }
    SEND_COOLING_DRIVER_REQUEST(cooling.d_pump_selected, cooling.d_fan_val, cooling.b_fan2_selected, cooling.b_pump_selected, cooling.b_fan_val);
}

void coolant_out_CALLBACK(CanParsedData_t* msg_data_a) {
    if (curr_page != PAGE_COOLING) {
        cooling.d_pump_selected = msg_data_a->coolant_out.dt_pump;
        cooling.b_fan2_selected = msg_data_a->coolant_out.bat_pump;
        cooling.b_pump_selected = msg_data_a->coolant_out.bat_pump_aux;
        return;
    }

    if (cooling.curr_hover != DT_FAN_SELECT) {
        cooling.d_fan_val = msg_data_a->coolant_out.dt_fan;
        set_value(DT_FAN_BAR, cooling.d_fan_val);
        set_textf(DT_FAN_VAL, "%d", cooling.d_fan_val);
    }

    if (cooling.curr_hover != FAN1_SELECT) {
        cooling.b_fan_val = msg_data_a->coolant_out.bat_fan;
        set_value(B_FAN1_BAR, cooling.b_fan_val);
        set_value(B_FAN1_VAL, cooling.b_fan_val);
        set_textf(B_FAN1_VAL, "%d", cooling.b_fan_val);
    }

    set_font_color(DT_PUMP_OP, COOLING_FG);
    set_font_color(B_FAN2_OP, COOLING_FG);
    set_font_color(B_PUMP_OP, COOLING_FG);
    set_background(DT_PUMP_OP, COOLING_BG);
    set_background(B_FAN2_OP, COOLING_BG);
    set_background(B_PUMP_OP, COOLING_BG);
    cooling.d_pump_selected = msg_data_a->coolant_out.dt_pump;
    cooling.b_fan2_selected = msg_data_a->coolant_out.bat_pump;
    cooling.b_pump_selected = msg_data_a->coolant_out.bat_pump_aux;
    set_value(DT_PUMP_OP, cooling.d_pump_selected);
    set_value(B_FAN2_OP, cooling.b_fan2_selected);
    set_value(B_PUMP_OP, cooling.b_pump_selected);

}

void update_tv_page() {
    // Establish hover position
    tv_settings.curr_hover = TV_INTENSITY_HOVER;

    // Set background colors
    set_background(TV_INTENSITY_FLT, TV_HOVER_BG);
    set_background(TV_PROPORTION_FLT, TV_BG);
    set_background(TV_DEAD_TXT, TV_BG);
    set_background(TV_ENABLE_OP, TV_BG);

    // Set displayed data
    set_value(TV_INTENSITY_FLT, tv_settings.tv_intensity_val);
    set_value(TV_PROPORTION_FLT, tv_settings.tv_p_val);
    set_textf(TV_DEAD_TXT, "%d", tv_settings.tv_deadband_val);
    set_value(TV_ENABLE_OP, tv_settings.tv_enable_selected);
}

void move_up_tv() {
    switch(tv_settings.curr_hover) {
        case TV_INTENSITY_SELECTED:
            // Increase the intensity value
            tv_settings.tv_intensity_val = (tv_settings.tv_intensity_val + 5) % 1000;
            // Update the page items
            set_value(TV_INTENSITY_FLT, tv_settings.tv_intensity_val);
            break;
        case TV_INTENSITY_HOVER:
            // Wrap around to enable
            tv_settings.curr_hover = TV_ENABLE_HOVER;
            // Update the background
            set_background(TV_ENABLE_OP, TV_HOVER_BG);
            set_background(TV_INTENSITY_FLT, TV_BG);
            break;
        case TV_P_SELECTED:
            // Increase the p value
            tv_settings.tv_p_val = (tv_settings.tv_p_val + 5) % 1000;
            // Update the page items
            set_value(TV_PROPORTION_FLT, tv_settings.tv_p_val);
            break;
        case TV_P_HOVER:
            // Scroll up to Intensity
            tv_settings.curr_hover = TV_INTENSITY_HOVER;
            // Update the background
            set_background(TV_INTENSITY_FLT, TV_HOVER_BG);
            set_background(TV_PROPORTION_FLT, TV_BG);
            break;
        case TV_DEADBAND_SELECTED:
            // Increase the deadband value
            tv_settings.tv_deadband_val = (tv_settings.tv_deadband_val + 1) % 30;

            // Update the page items
            set_textf(TV_DEAD_TXT, "%d", tv_settings.tv_deadband_val);
            break;
        case TV_DEADBAND_HOVER:
            // Scroll up to P
            tv_settings.curr_hover = TV_P_HOVER;
            // Update the background
            set_background(TV_PROPORTION_FLT, TV_HOVER_BG);
            set_background(TV_DEAD_TXT, TV_BG);
            break;
        case TV_ENABLE_HOVER:
            // Scroll up to deadband
            tv_settings.curr_hover = TV_DEADBAND_HOVER;
            set_background(TV_DEAD_TXT, TV_HOVER_BG);
            set_background(TV_ENABLE_OP, TV_BG);
            break;
    }
}

void move_down_tv() {
    switch (tv_settings.curr_hover) {
        case TV_INTENSITY_SELECTED:
            // Decrease the intensity value
            if (tv_settings.tv_intensity_val == 0)
            {
                tv_settings.tv_intensity_val = 100;
            }
            else
            {
                tv_settings.tv_intensity_val-= 5;
            }

            // Update the page item
            set_value(TV_INTENSITY_FLT, tv_settings.tv_intensity_val);
            break;
        case TV_INTENSITY_HOVER:
            // Scroll down to P
            tv_settings.curr_hover = TV_P_HOVER;

            // Update the background
            set_background(TV_PROPORTION_FLT, TV_HOVER_BG);
            set_background(TV_INTENSITY_FLT, TV_BG);
            break;
        case TV_P_SELECTED:
            // Decrease the P value
            if (tv_settings.tv_p_val == 0)
            {
                tv_settings.tv_p_val = 100;
            }
            else
            {
                tv_settings.tv_p_val-= 5;
            }

            // Update the page items
            set_value(TV_PROPORTION_FLT, tv_settings.tv_p_val);
            break;
        case TV_P_HOVER:
            // Scroll down to deadband
            tv_settings.curr_hover = TV_DEADBAND_HOVER;

            // Update the background
            set_background(TV_DEAD_TXT, TV_HOVER_BG);
            set_background(TV_PROPORTION_FLT, TV_BG);
            break;
        case TV_DEADBAND_SELECTED:
            // Decrease the deadband value
            if (tv_settings.tv_deadband_val == 0)
            {
                tv_settings.tv_deadband_val = 30;
            }
            else
            {
                tv_settings.tv_deadband_val--;
            }

            // Update the page items
            set_textf(TV_DEAD_TXT, "%d", tv_settings.tv_deadband_val);
            break;
        case TV_DEADBAND_HOVER:
            // Scroll down to enable
            tv_settings.curr_hover = TV_ENABLE_HOVER;

            // Update the background
            set_background(TV_ENABLE_OP, TV_HOVER_BG);
            set_background(TV_DEAD_TXT, TV_BG);
            break;
        case TV_ENABLE_HOVER:
            // Scroll down to intensity
            tv_settings.curr_hover = TV_INTENSITY_HOVER;
            set_background(TV_INTENSITY_FLT, TV_HOVER_BG);
            set_background(TV_ENABLE_OP, TV_BG);
            break;
    }
}

void select_tv() {
    // So if we hit select on an already selected item, unselect it (switch to hover)
    switch (tv_settings.curr_hover) {
        case TV_INTENSITY_HOVER:
            tv_settings.curr_hover = TV_INTENSITY_SELECTED;
            set_background(TV_INTENSITY_FLT, ORANGE);
            // todo Rot encoder state should let us scroll through value options
            // for now just use buttons for move up and move down
            break;
        case TV_INTENSITY_SELECTED:
            // "submit" -> CAN payload will update automatically? decide
            // Think about edge case when the user leaves the page? Can they without unselecting -> no. What if fault?
            tv_settings.curr_hover = TV_INTENSITY_HOVER;
            set_background(TV_INTENSITY_FLT, TV_HOVER_BG);
            // rot encoder state goes back to page move instead of value move
            break;
        case TV_P_HOVER:
            tv_settings.curr_hover = TV_P_SELECTED;
            set_background(TV_PROPORTION_FLT, ORANGE);
            break;
        case TV_P_SELECTED:
            tv_settings.curr_hover = TV_P_HOVER;
            set_background(TV_PROPORTION_FLT, TV_HOVER_BG);
            break;
        case TV_DEADBAND_HOVER:
            tv_settings.curr_hover = TV_DEADBAND_SELECTED;
            set_background(TV_DEAD_TXT, ORANGE);
            break;
        case TV_DEADBAND_SELECTED:
            tv_settings.curr_hover = TV_DEADBAND_HOVER;
            set_background(TV_DEAD_TXT, TV_HOVER_BG);
            break;
        case TV_ENABLE_HOVER:
            // Don't change the curr_hover
            // Toggle the option
            tv_settings.tv_enable_selected = (tv_settings.tv_enable_selected == 0);

            // Set the option
            set_value(TV_ENABLE_OP, tv_settings.tv_enable_selected);

            // Update CAN as necessary
            break;
    }
}

void update_faults_page() {
    if (fault_buf[0] == 0xFFFF)
    {
        set_text(FAULT_1_TXT, FAULT_NONE_STRING);
    }
    else
    {
        set_text(FAULT_1_TXT, faultArray[fault_buf[0]].screen_MSG);
    }
    if (fault_buf[1] == 0xFFFF)
    {
        set_text(FAULT_2_TXT, FAULT_NONE_STRING);
    }
    else
    {
        set_text(FAULT_2_TXT, faultArray[fault_buf[1]].screen_MSG);
    }

    if (fault_buf[2] == 0xFFFF)
    {
        set_text(FAULT_3_TXT, FAULT_NONE_STRING);
    }
    else
    {
        set_text(FAULT_3_TXT, faultArray[fault_buf[2]].screen_MSG);
    }

    if (fault_buf[3] == 0xFFFF)
    {
        set_text(FAULT_4_TXT, FAULT_NONE_STRING);
    }
    else
    {
        set_text(FAULT_4_TXT, faultArray[fault_buf[3]].screen_MSG);
    }

    if (fault_buf[4] == 0xFFFF)
    {
        set_text(FAULT_5_TXT, FAULT_NONE_STRING);
    }
    else
    {
        set_text(FAULT_5_TXT, faultArray[fault_buf[4]].screen_MSG);
    }
}

void move_up_faults() {
    switch (fault_page.curr_hover) {
        case FAULT1:
            // Wrap around to the last item
            fault_page.curr_hover = CLEAR;
            set_background(FAULT_1_TXT, BLACK);
            set_background(CLEAR_FAULTS_TXT, TV_HOVER_BG);
            break;
        case FAULT2:
            fault_page.curr_hover = FAULT1;
            set_background(FAULT_2_TXT, BLACK);
            set_background(FAULT_1_TXT, TV_HOVER_BG);
            break;
        case FAULT3:
            fault_page.curr_hover = FAULT2;
            set_background(FAULT_3_TXT, BLACK);
            set_background(FAULT_2_TXT, TV_HOVER_BG);
            break;
        case FAULT4:
            fault_page.curr_hover = FAULT3;
            set_background(FAULT_4_TXT, BLACK);
            set_background(FAULT_3_TXT, TV_HOVER_BG);
            break;
        case FAULT5:
            fault_page.curr_hover = FAULT4;
            set_background(FAULT_5_TXT, BLACK);
            set_background(FAULT_4_TXT, TV_HOVER_BG);
            break;
        case CLEAR:
            fault_page.curr_hover = FAULT5;
            set_background(CLEAR_FAULTS_TXT, BLACK);
            set_background(FAULT_5_TXT, TV_HOVER_BG);
            break;
    }
}

void move_down_faults() {
    switch (fault_page.curr_hover) {
        case FAULT1:
            fault_page.curr_hover = FAULT2;
            set_background(FAULT_1_TXT, BLACK);
            set_background(FAULT_2_TXT, TV_HOVER_BG);
            break;
        case FAULT2:
            fault_page.curr_hover = FAULT3;
            set_background(FAULT_2_TXT, BLACK);
            set_background(FAULT_3_TXT, TV_HOVER_BG);
            break;
        case FAULT3:
            fault_page.curr_hover = FAULT4;
            set_background(FAULT_3_TXT, BLACK);
            set_background(FAULT_4_TXT, TV_HOVER_BG);
            break;
        case FAULT4:
            fault_page.curr_hover = FAULT5;
            set_background(FAULT_4_TXT, BLACK);
            set_background(FAULT_5_TXT, TV_HOVER_BG);
            break;
        case FAULT5:
            fault_page.curr_hover = CLEAR;
            set_background(FAULT_5_TXT, BLACK);
            set_background(CLEAR_FAULTS_TXT, TV_HOVER_BG);
            break;
        case CLEAR:
            fault_page.curr_hover = FAULT1;
            set_background(CLEAR_FAULTS_TXT, BLACK);
            set_background(FAULT_1_TXT, TV_HOVER_BG);
            break;
    }
}

void update_race_page() {
    static uint8_t update_group = 0U;
    if (curr_page != PAGE_RACE) {
        return;
    }

    set_value(BRK_BAR, 0); // TODO BRK BAR
    set_value(THROT_BAR, (int) ((filtered_pedals / 4095.0) * 100));

    // update the speed
    if (can_data.rear_wheel_speeds.stale) {
        set_text(SPEED, "S");
    }
    else {
        // Vehicle Speed [m/s] = Wheel Speed [RPM] * 16 [in] * PI * 0.0254 / 60
        // set_text(SPEED, NXT_TEXT, int_to_char((uint16_t)((float)MAX(can_data.rear_wheel_speeds.left_speed_sensor, can_data.rear_wheel_speeds.right_speed_sensor) * 0.01 * 0.4474), parsed_value));
        uint16_t speed = ((float)can_data.gps_speed.gps_speed * 0.02237); // TODO macro this magic number
        set_textf(SPEED, "%d", speed);
    }

    //cycle the update groups
    update_group++;
    switch (update_group) {
        case 1:
            update_race_page_group1();
            break;
        case 2:
            update_race_page_group2();
            update_group = 0U;
            break;
        default:
            update_group = 0U;
            break;
    }
}

void update_race_page_group1() {
    if (can_data.rear_motor_temps.stale) {
        set_text(MOT_TEMP, "S");
    }
    else {
        uint8_t motor_temp = MAX(can_data.rear_motor_temps.left_mot_temp, can_data.rear_motor_temps.right_mot_temp);
        set_textf(MOT_TEMP, "%dC", motor_temp);
    }

    if (can_data.max_cell_temp.stale) {
        set_text(BATT_TEMP, "S");
    }
    else {
        uint16_t batt_temp = can_data.max_cell_temp.max_temp / 10;
        set_textf(BATT_TEMP, "%dC", batt_temp);
    }

    // TODO update MC_TEMP
}

void update_race_page_group2() {
    if (can_data.main_hb.stale) {
        set_text(CAR_STAT, "S");
        set_background(CAR_STAT, BLACK);
    }
    else {
        switch(can_data.main_hb.car_state) {
            case CAR_STATE_PRECHARGING:
                set_font_color(CAR_STAT, ORANGE);
                set_text(CAR_STAT, "PRCHG");
                break;
            case CAR_STATE_ENERGIZED:
                set_font_color(CAR_STAT, ORANGE);
                set_text(CAR_STAT, "ENER");
                break;
            case CAR_STATE_IDLE:
                set_font_color(CAR_STAT, INFO_GRAY);
                set_text(CAR_STAT, "INIT");
                break;
            case CAR_STATE_READY2DRIVE:
                set_font_color(CAR_STAT, RACE_GREEN);
                set_text(CAR_STAT, "ON");
                break;
            case CAR_STATE_ERROR:
                set_font_color(CAR_STAT, YELLOW);
                set_text(CAR_STAT, "ERR");
                break;
            case CAR_STATE_FATAL:
                set_font_color(CAR_STAT, RED);
                set_text(CAR_STAT, "FATAL");
                break;
        }
    }

    // Update the voltage and current
    if (can_data.orion_currents_volts.stale) {
        set_text(BATT_VOLT, "S");
        set_text(BATT_CURR, "S");
    }
    else {
        uint16_t voltage = (can_data.orion_currents_volts.pack_voltage / 10);
        set_textf(BATT_VOLT, "%dV", voltage);

        uint16_t current = (can_data.orion_currents_volts.pack_current / 10);
        set_textf(BATT_CURR, "%dA", current);  // Note: Changed 'V' to 'A' for current
    }
}

void select_race() {
    // there is only one option for now
    tv_settings.tv_enable_selected = (tv_settings.tv_enable_selected == 0);
    set_value(RACE_TV_ON, tv_settings.tv_enable_selected);
}

void setFaultIndicator(uint16_t fault, char *element) {
    if (fault == 0xFFFF) {
        set_font_color(element, WHITE);
    } else if (checkFault(fault)) {
        set_font_color(element, RED);
    }
    set_font_color(element, RACE_GREEN);
}

void updateSDCStatus(uint8_t status, char *element) {
    if (status)
    {
        set_background(element, GREEN);
    }
    else
    {
        set_background(element, RED);
    }
}
