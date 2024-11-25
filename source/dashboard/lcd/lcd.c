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
volatile cooling_t cooling;           // Data for the cooling page
volatile tv_settings_t tv_settings;     // Data for the tvsettings page
volatile driver_config_t driver_config; // Data for the driver page
volatile fault_page_t fault_page;       // Data for the faults page
race_page_t race_page_data;             // Data for the race page
extern lcd_t lcd_data;
uint8_t fault_time_displayed;           // Amount of units of time that the fault has been shown to the driver


// Driver Page Functions
void update_driver_page();
void move_up_driver();
void move_down_driver();
void select_driver();

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

// Race Page Functions
void update_race_page();
void update_race_page_group1();
void update_race_page_group2();
void select_race();

// Utility Functions
void updateSDCStatus(uint8_t status, char *element);
void setFaultIndicator(uint16_t fault, char *element);
// string helper prototypes
void append_char(char *str, char ch, size_t max_len); // Append a character to a string
char *int_to_char(int16_t val, char *val_to_send);  // Convert integer value to character for the nextion interface


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
    cooling = (cooling_t) {0, 0, 0, 0, 0, 0, 0, 0};
    sendFirsthalf = true;
    tv_settings = (tv_settings_t) {true, 0, 12, 100, 40 };
    fault_page_t fault_config = {FAULT1}; // Default to first fault
    set_baud(115200);
    set_brightness(100);
}

void updatePage() {
    // Only update the encoder if we are on a "selectable" page
    if ((curr_page != PAGE_ERROR) && (curr_page != PAGE_WARNING) && (curr_page != PAGE_FATAL))
    {
        curr_page = lcd_data.encoder_position;
        fault_time_displayed = 0;
    }

    // If we do not detect a page update (most notably detect if encoder did not move), do nothing
    if (curr_page == prev_page) {
        return;
    }

    // Parse the page that was passed into the function
    switch (curr_page) {
        case PAGE_LOGGING:
            prev_page = PAGE_LOGGING;
            set_page(LOGGING_STRING);
            break;
        case PAGE_DRIVER:
            prev_page = PAGE_DRIVER;
            set_page(DRIVER_STRING);
            update_driver_page();
            break;
        case PAGE_DRIVER_CALIBRATION:
            prev_page = PAGE_DRIVER_CALIBRATION;
            set_page(DRIVER_CALIBRATION_STRING);
            break;
        case PAGE_SDCINFO:
            prev_page = PAGE_SDCINFO;
            set_page(SDCINFO_STRING);
            break;
        case PAGE_TVSETTINGS:
            prev_page = PAGE_TVSETTINGS;
            set_page(TVSETTINGS_STRING);
            update_tv_page();
            break;
        case PAGE_ERROR:
            set_page(ERR_STRING);
            set_text(ERR_TXT, NXT_TEXT, errorText);
            break;
        case PAGE_WARNING:
            set_page(WARN_STRING);
            set_text(ERR_TXT, NXT_TEXT, errorText);
            break;
        case PAGE_FATAL:
            set_page(FATAL_STRING);
            set_text(ERR_TXT, NXT_TEXT, errorText);
            break;
        case PAGE_RACE:
            prev_page = PAGE_RACE;
            set_page(RACE_STRING);
            break;
        case PAGE_DATA:
            prev_page = PAGE_DATA;
            set_page(DATA_STRING);
            break;
        case PAGE_COOLING:
            prev_page = PAGE_COOLING;
            set_page(COOLING_STRING);
            update_cooling_page();
            break;
        case PAGE_FAULTS:
            prev_page = PAGE_FAULTS;
            set_page(FAULT_STRING);
            update_faults_page();
            break;
    }
}

void moveUp() {
    switch (curr_page) {
        case PAGE_LOGGING:
            //TODO
            break;
        case PAGE_DRIVER:
            move_up_driver();
            break;
        case PAGE_TVSETTINGS:
            move_up_tv();
            break;
        case PAGE_COOLING:
            move_up_cooling();
            break;
        case PAGE_FAULTS:
            move_up_faults();
            break;
    }
}

void moveDown() {
    switch (curr_page) {
        case PAGE_LOGGING:
            //TODO
            break;
        case PAGE_DRIVER:
            move_down_driver();
            break;
        case PAGE_TVSETTINGS:
            move_down_tv();
            break;
        case PAGE_COOLING:
            move_down_cooling();
            break;
        case PAGE_FAULTS:
            move_down_faults();
            break;
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

void selectItem() {

    // User has selected to clear the current fault screen
    if ((curr_page == PAGE_ERROR) || (curr_page == PAGE_FATAL) || (curr_page == PAGE_WARNING))
    {
        // Go back to where we were before
        curr_page = prev_page;
        // so select item doesnt't break
        prev_page = PAGE_PREFLIGHT;
        fault_time_displayed = 0;
        updatePage();
        return;
    }

    switch (curr_page) {
        case PAGE_LOGGING:
            SEND_DASHBOARD_START_LOGGING(1);
            break;
        case PAGE_DRIVER:
            select_driver();
            break;
        case PAGE_TVSETTINGS:
            select_tv();
            break;
        case PAGE_COOLING:
            select_cooling();
            break;
        case PAGE_RACE:
            select_race();
            break;
        case PAGE_FAULTS:
            select_fault();
            break;
    }
}

void updateDataPages() {
    if (curr_page == PAGE_RACE) {
        update_race_page();
        return;
    }

    if (curr_page == PAGE_DATA) {
        set_value(POW_LIM_BAR, NXT_VALUE, 0);
        set_value(THROT_BAR, NXT_VALUE, (int) ((filtered_pedals / 4095.0) * 100));
    }

    return;
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
    if (curr_page != PAGE_FAULTS) {
        return;
    }

    setFaultIndicator(fault_buf[0], FAULT_1_TXT);
    setFaultIndicator(fault_buf[1], FAULT_2_TXT);
    setFaultIndicator(fault_buf[2], FAULT_3_TXT);
    setFaultIndicator(fault_buf[3], FAULT_4_TXT);
    setFaultIndicator(fault_buf[4], FAULT_5_TXT);
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

void update_driver_page() {
    driver_config.curr_hover = DRIVER_DEFAULT_SELECT;
    set_value(DRIVER_DEFAULT_TXT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
    set_value(DRIVER_TYLER_TXT, NXT_BACKGROUND_COLOR, TV_BG);
    set_value(DRIVER_RUHAAN_TXT, NXT_BACKGROUND_COLOR, TV_BG);
    set_value(DRIVER_LUKE_TXT, NXT_BACKGROUND_COLOR, TV_BG);

    if (driver_config.curr_select == DRIVER_DEFAULT_SELECT)
    {
        set_value(DRIVER_DEFAULT_OP, NXT_VALUE, 1);
    }
    else
    {
        set_value(DRIVER_DEFAULT_OP, NXT_VALUE, 0);
    }

    if (driver_config.curr_select == DRIVER_TYLER_SELECT)
    {
        set_value(DRIVER_TYLER_OP, NXT_VALUE, 1);
    }
    else
    {
        set_value(DRIVER_TYLER_OP, NXT_VALUE, 0);
    }

    if (driver_config.curr_select == DRIVER_LUCA_SELECT)
    {
        set_value(DRIVER_RUHAAN_OP, NXT_VALUE, 1);
    }
    else
    {
        set_value(DRIVER_RUHAAN_OP, NXT_VALUE, 0);
    }

    if (driver_config.curr_select == DRIVER_LUKE_SELECT)
    {
        set_value(DRIVER_LUKE_OP, NXT_VALUE, 1);
    }
    else
    {
        set_value(DRIVER_LUKE_OP, NXT_VALUE, 0);
    }
}

void move_up_driver() {
    if (driver_config.curr_hover == DRIVER_DEFAULT_SELECT)
    {
        // Wrap around to enable
        driver_config.curr_hover = DRIVER_LUKE_SELECT;
        driver_config.curr_select = DRIVER_LUKE_SELECT;
        // Update the background
        set_value(DRIVER_DEFAULT_TXT, NXT_BACKGROUND_COLOR, TV_BG);
        set_value(DRIVER_TYLER_TXT, NXT_BACKGROUND_COLOR, TV_BG);
        set_value(DRIVER_RUHAAN_TXT, NXT_BACKGROUND_COLOR, TV_BG);
        set_value(DRIVER_LUKE_TXT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
    }
    else if (driver_config.curr_hover == DRIVER_TYLER_SELECT)
    {
        driver_config.curr_hover = DRIVER_DEFAULT_SELECT;
        driver_config.curr_select = DRIVER_DEFAULT_SELECT;

        set_value(DRIVER_DEFAULT_TXT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
        set_value(DRIVER_TYLER_TXT, NXT_BACKGROUND_COLOR, TV_BG);
        set_value(DRIVER_RUHAAN_TXT, NXT_BACKGROUND_COLOR, TV_BG);
        set_value(DRIVER_LUKE_TXT, NXT_BACKGROUND_COLOR, TV_BG);
    }
    else if (driver_config.curr_hover == DRIVER_LUCA_SELECT)
    {
        driver_config.curr_hover = DRIVER_TYLER_SELECT;
        driver_config.curr_select = DRIVER_TYLER_SELECT;
        set_value(DRIVER_DEFAULT_TXT, NXT_BACKGROUND_COLOR, TV_BG);
        set_value(DRIVER_TYLER_TXT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
        set_value(DRIVER_RUHAAN_TXT, NXT_BACKGROUND_COLOR, TV_BG);
        set_value(DRIVER_LUKE_TXT, NXT_BACKGROUND_COLOR, TV_BG);
    }
    else if (driver_config.curr_hover == DRIVER_LUKE_SELECT)
    {
        driver_config.curr_hover = DRIVER_LUCA_SELECT;
        driver_config.curr_select = DRIVER_LUCA_SELECT;
        set_value(DRIVER_DEFAULT_TXT, NXT_BACKGROUND_COLOR, TV_BG);
        set_value(DRIVER_TYLER_TXT, NXT_BACKGROUND_COLOR, TV_BG);
        set_value(DRIVER_RUHAAN_TXT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
        set_value(DRIVER_LUKE_TXT, NXT_BACKGROUND_COLOR, TV_BG);
    }
}

void move_down_driver() {
    if (driver_config.curr_hover == DRIVER_DEFAULT_SELECT)
    {
        driver_config.curr_hover = DRIVER_TYLER_SELECT;
        driver_config.curr_select = DRIVER_TYLER_SELECT;
        // Update the background
        set_value(DRIVER_DEFAULT_TXT, NXT_BACKGROUND_COLOR, TV_BG);
        set_value(DRIVER_TYLER_TXT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
        set_value(DRIVER_RUHAAN_TXT, NXT_BACKGROUND_COLOR, TV_BG);
        set_value(DRIVER_LUKE_TXT, NXT_BACKGROUND_COLOR, TV_BG);
    }
    else if (driver_config.curr_hover == DRIVER_TYLER_SELECT)
    {
        driver_config.curr_hover = DRIVER_LUCA_SELECT;
        driver_config.curr_select = DRIVER_LUCA_SELECT;
        set_value(DRIVER_DEFAULT_TXT, NXT_BACKGROUND_COLOR, TV_BG);
        set_value(DRIVER_TYLER_TXT, NXT_BACKGROUND_COLOR, TV_BG);
        set_value(DRIVER_RUHAAN_TXT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
        set_value(DRIVER_LUKE_TXT, NXT_BACKGROUND_COLOR, TV_BG);
    }
    else if (driver_config.curr_hover == DRIVER_LUCA_SELECT)
    {
        driver_config.curr_hover = DRIVER_LUKE_SELECT;
        driver_config.curr_select = DRIVER_LUKE_SELECT;
        set_value(DRIVER_DEFAULT_TXT, NXT_BACKGROUND_COLOR, TV_BG);
        set_value(DRIVER_TYLER_TXT, NXT_BACKGROUND_COLOR, TV_BG);
        set_value(DRIVER_RUHAAN_TXT, NXT_BACKGROUND_COLOR, TV_BG);
        set_value(DRIVER_LUKE_TXT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
    }
    else if (driver_config.curr_hover == DRIVER_LUKE_SELECT)
    {
        driver_config.curr_hover = DRIVER_DEFAULT_SELECT;
        driver_config.curr_select = DRIVER_DEFAULT_SELECT;
        set_value(DRIVER_DEFAULT_TXT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
        set_value(DRIVER_TYLER_TXT, NXT_BACKGROUND_COLOR, TV_BG);
        set_value(DRIVER_RUHAAN_TXT, NXT_BACKGROUND_COLOR, TV_BG);
        set_value(DRIVER_LUKE_TXT, NXT_BACKGROUND_COLOR, TV_BG);
    }
}

void select_driver() {
    switch(driver_config.curr_hover)
    {
        case DRIVER_DEFAULT_SELECT:
            set_value(DRIVER_DEFAULT_OP, NXT_VALUE, 1);
            set_value(DRIVER_TYLER_OP, NXT_VALUE, 0);
            set_value(DRIVER_RUHAAN_OP, NXT_VALUE, 0);
            set_value(DRIVER_LUKE_OP, NXT_VALUE, 0);
            break;
        case DRIVER_TYLER_SELECT:
            set_value(DRIVER_DEFAULT_OP, NXT_VALUE, 0);
            set_value(DRIVER_TYLER_OP, NXT_VALUE, 1);
            set_value(DRIVER_RUHAAN_OP, NXT_VALUE, 0);
            set_value(DRIVER_LUKE_OP, NXT_VALUE, 0);
            break;
        case DRIVER_LUCA_SELECT:
            set_value(DRIVER_DEFAULT_OP, NXT_VALUE, 0);
            set_value(DRIVER_TYLER_OP, NXT_VALUE, 0);
            set_value(DRIVER_RUHAAN_OP, NXT_VALUE, 1);
            set_value(DRIVER_LUKE_OP, NXT_VALUE, 0);
            break;
        case DRIVER_LUKE_SELECT:
            set_value(DRIVER_DEFAULT_OP, NXT_VALUE, 0);
            set_value(DRIVER_TYLER_OP, NXT_VALUE, 0);
            set_value(DRIVER_RUHAAN_OP, NXT_VALUE, 0);
            set_value(DRIVER_LUKE_OP, NXT_VALUE, 1);
            break;
    }
}

void update_cooling_page() {
    // Parsed value represents:
    char parsed_value[3] = "\0";
    cooling.curr_hover = DT_FAN_HOVER;                                     // Set hover
    set_value(DT_FAN_TXT, NXT_BACKGROUND_COLOR, COOLING_HOVER_BG);         // Set t2 with cooling hover
    set_value(DT_FAN_BAR, NXT_VALUE, cooling.d_fan_val);                   // Set progress bar for j0
    //set_value(DT_FAN_VAL, NXT_FONT_COLOR, COOLING_BAR_BG);                         // Set color for t8 (background of bar?)
    set_text(DT_FAN_VAL, NXT_TEXT, int_to_char(cooling.d_fan_val, parsed_value));  // Set fan value for t8
    bzero(parsed_value, 3);                                                         // Clear our char buffer

    // Set drivetrain pump selector color
    if (cooling.d_pump_selected) {
        set_value(DT_PUMP_OP, NXT_FONT_COLOR, SETTINGS_UV_SELECT);
    }
    else {
        set_value(DT_PUMP_OP, NXT_BACKGROUND_COLOR, COOLING_HOVER_BG);
    }

    // Set drivetrain pump selector status
    set_value(DT_PUMP_OP, NXT_VALUE, cooling.d_pump_selected);

    // Set Battery fan c3 (Pump 1?)
    // todo: Why is this here?
    if (cooling.b_fan2_selected) {
        set_value(B_FAN2_OP, NXT_FONT_COLOR, SETTINGS_UV_SELECT);
    }
    else {
        set_value(B_FAN2_OP, NXT_BACKGROUND_COLOR, COOLING_HOVER_BG);
    }

    // Set value for c3 battery pump 1
    set_value(B_FAN2_OP, NXT_VALUE, cooling.b_fan2_selected);
    if (cooling.b_pump_selected) {
        set_value(B_PUMP_OP, NXT_FONT_COLOR, SETTINGS_UV_SELECT);
    }
    else {
        set_value(B_PUMP_OP, NXT_BACKGROUND_COLOR, COOLING_HOVER_BG);
    }

    // Set Battery Pump 2 value
    set_value(B_PUMP_OP, NXT_VALUE, cooling.b_pump_selected);

    // Set battery fan bar, text, color
    set_value(B_FAN1_BAR, NXT_VALUE, cooling.b_fan_val);
    set_text(B_FAN1_VAL, NXT_TEXT, int_to_char(cooling.b_fan_val, parsed_value));
    bzero(parsed_value, 3);
    set_value(B_FAN1_VAL, NXT_FONT_COLOR, WHITE);
}

void move_up_cooling() {
    char parsed_value[3] = "\0";
    switch (cooling.curr_hover) {
        case DT_FAN_HOVER:
            cooling.curr_hover = PUMP_HOVER;
            set_value(DT_FAN_TXT, NXT_BACKGROUND_COLOR, COOLING_BG);
            set_value(DT_FAN_TXT, NXT_FONT_COLOR, COOLING_FG);
            set_value(B_PUMP_TXT, NXT_BACKGROUND_COLOR, COOLING_HOVER_BG);
            set_value(B_PUMP_TXT, NXT_FONT_COLOR, COOLING_HOVER_FG);
            break;
        case DT_PUMP_HOVER:
            cooling.curr_hover = DT_FAN_HOVER;
            set_value(DT_PUMP_TXT, NXT_BACKGROUND_COLOR, COOLING_BG);
            set_value(DT_PUMP_TXT, NXT_FONT_COLOR, COOLING_FG);
            set_value(DT_FAN_TXT, NXT_BACKGROUND_COLOR, COOLING_HOVER_BG);
            set_value(DT_FAN_TXT, NXT_FONT_COLOR, COOLING_HOVER_FG);
            break;
        case FAN1_HOVER:
            cooling.curr_hover = DT_PUMP_HOVER;
            set_value(B_FAN1_TXT, NXT_BACKGROUND_COLOR, COOLING_BG);
            set_value(B_FAN1_TXT, NXT_FONT_COLOR, COOLING_FG);
            set_value(DT_PUMP_TXT, NXT_BACKGROUND_COLOR, COOLING_HOVER_BG);
            set_value(DT_PUMP_TXT, NXT_FONT_COLOR, COOLING_HOVER_FG);
            break;
        case FAN2_HOVER:
            cooling.curr_hover = FAN1_HOVER;
            set_value(B_FAN2_TXT, NXT_BACKGROUND_COLOR, COOLING_BG);
            set_value(B_FAN2_TXT, NXT_FONT_COLOR, COOLING_FG);
            set_value(B_FAN1_TXT, NXT_BACKGROUND_COLOR, COOLING_HOVER_BG);
            set_value(B_FAN1_TXT, NXT_FONT_COLOR, COOLING_HOVER_FG);
            break;
        case PUMP_HOVER:
            cooling.curr_hover = FAN2_HOVER;
            set_value(B_PUMP_TXT, NXT_BACKGROUND_COLOR, COOLING_BG);
            set_value(B_PUMP_TXT, NXT_FONT_COLOR, COOLING_FG);
            set_value(B_FAN2_TXT, NXT_BACKGROUND_COLOR, COOLING_HOVER_BG);
            set_value(B_FAN2_TXT, NXT_FONT_COLOR, COOLING_HOVER_FG);
            break;
        case DT_FAN_SELECT:
            cooling.d_fan_val /= BAR_INTERVAL;
            cooling.d_fan_val *= BAR_INTERVAL;
            cooling.d_fan_val = (cooling.d_fan_val == 100) ? 0 : cooling.d_fan_val + BAR_INTERVAL;
            set_value(DT_FAN_BAR, NXT_VALUE, cooling.d_fan_val);
            set_text(DT_FAN_VAL, NXT_TEXT, int_to_char(cooling.d_fan_val, parsed_value));
            bzero(parsed_value, 3);
            //set_value(DT_FAN_VAL, NXT_FONT_COLOR, BLACK);
            break;
        case FAN1_SELECT:
            cooling.b_fan_val /= BAR_INTERVAL;
            cooling.b_fan_val *= BAR_INTERVAL;
            cooling.b_fan_val = (cooling.b_fan_val == 100) ? 0 : cooling.b_fan_val + BAR_INTERVAL;
            set_value(B_FAN1_BAR, NXT_VALUE, cooling.b_fan_val);
            set_text(B_FAN1_VAL, NXT_TEXT, int_to_char(cooling.b_fan_val, parsed_value));
            bzero(parsed_value, 3);
            //set_value(B_FAN1_VAL, NXT_FONT_COLOR, BLACK);
            break;
    }
}

void move_down_cooling() {
    char parsed_value[3] = "\0";
    switch (cooling.curr_hover) {
        case DT_FAN_HOVER:
            cooling.curr_hover = DT_PUMP_HOVER;
            set_value(DT_FAN_TXT, NXT_BACKGROUND_COLOR, COOLING_BG);
            set_value(DT_FAN_TXT, NXT_FONT_COLOR, COOLING_FG);
            set_value(DT_PUMP_TXT, NXT_BACKGROUND_COLOR, COOLING_HOVER_BG);
            set_value(DT_PUMP_TXT, NXT_FONT_COLOR, COOLING_HOVER_FG);
            break;
        case DT_PUMP_HOVER:
            cooling.curr_hover = FAN1_HOVER;
            set_value(DT_PUMP_TXT, NXT_BACKGROUND_COLOR, COOLING_BG);
            set_value(DT_PUMP_TXT, NXT_FONT_COLOR, COOLING_FG);
            set_value(B_FAN1_TXT, NXT_BACKGROUND_COLOR, COOLING_HOVER_BG);
            set_value(B_FAN1_TXT, NXT_FONT_COLOR, COOLING_HOVER_FG);
            break;
        case FAN1_HOVER:
            cooling.curr_hover = FAN2_HOVER;
            set_value(B_FAN1_TXT, NXT_BACKGROUND_COLOR, COOLING_BG);
            set_value(B_FAN1_TXT, NXT_FONT_COLOR, COOLING_FG);
            set_value(B_FAN2_TXT, NXT_BACKGROUND_COLOR, COOLING_HOVER_BG);
            set_value(B_FAN2_TXT, NXT_FONT_COLOR, COOLING_HOVER_FG);
            break;
        case FAN2_HOVER:
            cooling.curr_hover = PUMP_HOVER;
            set_value(B_FAN2_TXT, NXT_BACKGROUND_COLOR, COOLING_BG);
            set_value(B_FAN2_TXT, NXT_FONT_COLOR, COOLING_FG);
            set_value(B_PUMP_TXT, NXT_BACKGROUND_COLOR, COOLING_HOVER_BG);
            set_value(B_PUMP_TXT, NXT_FONT_COLOR, COOLING_HOVER_FG);
            break;
        case PUMP_HOVER:
            cooling.curr_hover = DT_FAN_HOVER;
            set_value(B_PUMP_TXT, NXT_BACKGROUND_COLOR, COOLING_BG);
            set_value(B_PUMP_TXT, NXT_FONT_COLOR, COOLING_FG);
            set_value(DT_FAN_TXT, NXT_BACKGROUND_COLOR, COOLING_HOVER_BG);
            set_value(DT_FAN_TXT, NXT_FONT_COLOR, COOLING_HOVER_FG);
            break;
        case DT_FAN_SELECT:
            cooling.d_fan_val /= BAR_INTERVAL;
            cooling.d_fan_val *= BAR_INTERVAL;
            cooling.d_fan_val = (cooling.d_fan_val == 0) ? 100 : cooling.d_fan_val - BAR_INTERVAL;
            set_value(DT_FAN_BAR, NXT_VALUE, cooling.d_fan_val);
            set_text(DT_FAN_VAL, NXT_TEXT, int_to_char(cooling.d_fan_val, parsed_value));
            bzero(parsed_value, 3);
            break;
        case FAN1_SELECT:
            cooling.b_fan_val /= BAR_INTERVAL;
            cooling.b_fan_val *= BAR_INTERVAL;
            cooling.b_fan_val = (cooling.b_fan_val == 0) ? 100 : cooling.b_fan_val - BAR_INTERVAL;
            set_value(B_FAN1_BAR, NXT_VALUE, cooling.b_fan_val);
            set_text(B_FAN1_VAL, NXT_TEXT, int_to_char(cooling.b_fan_val, parsed_value));
            bzero(parsed_value, 3);
            break;
    }
}

void select_cooling() {
    switch (cooling.curr_hover) {
    case DT_FAN_HOVER:
        // cooling.d_fan_selected = !cooling.d_fan_selected;
        // set_value(DT_FAN_BAR, NXT_VALUE, cooling.d_fan_selected);
        cooling.curr_hover = DT_FAN_SELECT;
        set_value(DT_FAN_TXT, NXT_VALUE, COOLING_BG);
        set_value(DT_FAN_BAR, NXT_BACKGROUND_COLOR, WHITE);
        //set_value(DT_FAN_BAR, NXT_FONT_COLOR, BLACK);
        return;
    case DT_PUMP_HOVER:
        cooling.d_pump_selected = !cooling.d_pump_selected;
        if (cooling.d_pump_selected) {
            set_value(DT_PUMP_OP, NXT_FONT_COLOR, SETTINGS_UV_SELECT);
            set_value(DT_PUMP_OP, NXT_BACKGROUND_COLOR, COOLING_BG);
        }
        else {
            set_value(DT_PUMP_OP, NXT_BACKGROUND_COLOR, COOLING_HOVER_BG);
        }
        set_value(DT_PUMP_OP, NXT_VALUE, cooling.d_pump_selected);
        break;
    case FAN1_HOVER:
        // cooling.b_fan1_selected = !cooling.b_fan1_selected;
        // set_value(B_FAN1_BAR, NXT_VALUE, cooling.b_fan1_selected);
        cooling.curr_hover = FAN1_SELECT;
        set_value(B_FAN1_TXT, NXT_VALUE, COOLING_BG);
        set_value(B_FAN1_BAR, NXT_BACKGROUND_COLOR, WHITE);
        //set_value(B_FAN1_BAR, NXT_FONT_COLOR, BLACK);
        break;
    case FAN2_HOVER:
        cooling.b_fan2_selected = !cooling.b_fan2_selected;
        if (cooling.b_fan2_selected) {
            set_value(B_FAN2_OP, NXT_FONT_COLOR, SETTINGS_UV_SELECT);
            set_value(B_FAN2_OP, NXT_BACKGROUND_COLOR, COOLING_BG);
        }
        else {
            set_value(B_FAN2_OP, NXT_BACKGROUND_COLOR, COOLING_HOVER_BG);
        }
        set_value(B_FAN2_OP, NXT_VALUE, cooling.b_fan2_selected);
        break;
    case PUMP_HOVER:
        cooling.b_pump_selected = !cooling.b_pump_selected;
        if (cooling.b_pump_selected) {
        set_value(B_PUMP_OP, NXT_FONT_COLOR, SETTINGS_UV_SELECT);
        set_value(B_PUMP_OP, NXT_BACKGROUND_COLOR, COOLING_BG);
        }
        else {
            set_value(B_PUMP_OP, NXT_BACKGROUND_COLOR, COOLING_HOVER_BG);
        }
        set_value(B_PUMP_OP, NXT_VALUE, cooling.b_pump_selected);
        break;
    case DT_FAN_SELECT:
        cooling.curr_hover = DT_FAN_HOVER;
        set_value(DT_FAN_TXT, NXT_VALUE, COOLING_HOVER_BG);
        set_value(DT_FAN_BAR, NXT_BACKGROUND_COLOR, COOLING_BAR_BG);
        set_value(DT_FAN_BAR, NXT_FONT_COLOR, COOLING_BAR_FG);
        //set_value(DT_FAN_VAL, NXT_FONT_COLOR, COOLING_BAR_BG);
        break;
    case FAN1_SELECT:
        cooling.curr_hover = FAN1_HOVER;
        set_value(B_FAN1_TXT, NXT_VALUE, COOLING_HOVER_BG);
        set_value(B_FAN1_BAR, NXT_BACKGROUND_COLOR, COOLING_BAR_BG);
        set_value(B_FAN1_BAR, NXT_FONT_COLOR, COOLING_BAR_FG);
        //set_value(B_FAN1_VAL, NXT_FONT_COLOR, COOLING_BAR_BG);
        break;
    }
    SEND_COOLING_DRIVER_REQUEST(cooling.d_pump_selected, cooling.d_fan_val, cooling.b_fan2_selected, cooling.b_pump_selected, cooling.b_fan_val);
}

void coolant_out_CALLBACK(CanParsedData_t* msg_data_a) {
    char parsed_value[3] = "\0";
    if (curr_page != PAGE_COOLING) {
        cooling.d_pump_selected = msg_data_a->coolant_out.dt_pump;
        cooling.b_fan2_selected = msg_data_a->coolant_out.bat_pump;
        cooling.b_pump_selected = msg_data_a->coolant_out.bat_pump_aux;
        return;
    }

    if (cooling.curr_hover != DT_FAN_SELECT) {
        cooling.d_fan_val = msg_data_a->coolant_out.dt_fan;
        set_value(DT_FAN_BAR, NXT_VALUE, cooling.d_fan_val);
        //set_value(DT_FAN_VAL, NXT_FONT_COLOR, BLACK);
        set_text(DT_FAN_VAL, NXT_TEXT, int_to_char(cooling.d_fan_val, parsed_value));
        bzero(parsed_value, 3);
    }

    if (cooling.curr_hover != FAN1_SELECT) {
        cooling.b_fan_val = msg_data_a->coolant_out.bat_fan;
        set_value(B_FAN1_BAR, NXT_VALUE, cooling.b_fan_val);
        set_value(B_FAN1_VAL, NXT_FONT_COLOR, cooling.b_fan_val);
        set_text(B_FAN1_VAL, NXT_TEXT, int_to_char(cooling.b_fan_val, parsed_value));
        bzero(parsed_value, 3);
    }

    set_value(DT_PUMP_OP, NXT_FONT_COLOR, COOLING_FG);
    set_value(B_FAN2_OP, NXT_FONT_COLOR, COOLING_FG);
    set_value(B_PUMP_OP, NXT_FONT_COLOR, COOLING_FG);
    set_value(DT_PUMP_OP, NXT_BACKGROUND_COLOR, COOLING_BG);
    set_value(B_FAN2_OP, NXT_BACKGROUND_COLOR, COOLING_BG);
    set_value(B_PUMP_OP, NXT_BACKGROUND_COLOR, COOLING_BG);
    cooling.d_pump_selected = msg_data_a->coolant_out.dt_pump;
    cooling.b_fan2_selected = msg_data_a->coolant_out.bat_pump;
    cooling.b_pump_selected = msg_data_a->coolant_out.bat_pump_aux;
    set_value(DT_PUMP_OP, NXT_VALUE, cooling.d_pump_selected);
    set_value(B_FAN2_OP, NXT_VALUE, cooling.b_fan2_selected);
    set_value(B_PUMP_OP, NXT_VALUE, cooling.b_pump_selected);

}

void update_tv_page() {
    // Parsed value represents:
    char parsed_value[3] = "\0";

    // Establish hover position
    tv_settings.curr_hover = TV_INTENSITY_HOVER;

    // Set background colors
    set_value(TV_INTENSITY_FLT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
    set_value(TV_PROPORTION_FLT, NXT_BACKGROUND_COLOR, TV_BG);
    set_value(TV_DEAD_TXT, NXT_BACKGROUND_COLOR, TV_BG);
    set_value(TV_ENABLE_OP, NXT_BACKGROUND_COLOR, TV_BG);

    // Set displayed data
    set_value(TV_INTENSITY_FLT, NXT_VALUE, tv_settings.tv_intensity_val);
    set_value(TV_PROPORTION_FLT, NXT_VALUE, tv_settings.tv_p_val);
    set_text(TV_DEAD_TXT, NXT_TEXT, int_to_char(tv_settings.tv_deadband_val, parsed_value));
    bzero(parsed_value, 3);
    set_value(TV_ENABLE_OP, NXT_VALUE, tv_settings.tv_enable_selected);
}

void move_up_tv() {
    char parsed_value[3] = "\0";
    switch(tv_settings.curr_hover) {
        case TV_INTENSITY_SELECTED:
            // Increase the intensity value
            tv_settings.tv_intensity_val = (tv_settings.tv_intensity_val + 5) % 1000;

            // Update the page items
            set_value(TV_INTENSITY_FLT, NXT_VALUE, tv_settings.tv_intensity_val);
            break;
        case TV_INTENSITY_HOVER:
            // Wrap around to enable
            tv_settings.curr_hover = TV_ENABLE_HOVER;

            // Update the background
            set_value(TV_INTENSITY_FLT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_PROPORTION_FLT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_DEAD_TXT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_ENABLE_OP, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
            break;
        case TV_P_SELECTED:
            // Increase the p value
            tv_settings.tv_p_val = (tv_settings.tv_p_val + 5) % 1000;

            // Update the page items
            set_value(TV_PROPORTION_FLT, NXT_VALUE, tv_settings.tv_p_val);
            break;
        case TV_P_HOVER:
            // Scroll up to Intensity
            tv_settings.curr_hover = TV_INTENSITY_HOVER;

            // Update the background
            set_value(TV_INTENSITY_FLT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
            set_value(TV_PROPORTION_FLT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_DEAD_TXT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_ENABLE_OP, NXT_BACKGROUND_COLOR, TV_BG);
            break;
        case TV_DEADBAND_SELECTED:
            // Increase the deadband value
            tv_settings.tv_deadband_val = (tv_settings.tv_deadband_val + 1) % 30;

            // Update the page items
            set_text(TV_DEAD_TXT, NXT_TEXT, int_to_char(tv_settings.tv_deadband_val, parsed_value));
            bzero(parsed_value, 3);
            break;
        case TV_DEADBAND_HOVER:
            // Scroll up to P
            tv_settings.curr_hover = TV_P_HOVER;

            // Update the background
            set_value(TV_INTENSITY_FLT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_PROPORTION_FLT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
            set_value(TV_DEAD_TXT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_ENABLE_OP, NXT_BACKGROUND_COLOR, TV_BG);
            break;
        case TV_ENABLE_HOVER:
            // Scroll up to deadband
            tv_settings.curr_hover = TV_DEADBAND_HOVER;
            set_value(TV_INTENSITY_FLT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_PROPORTION_FLT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_DEAD_TXT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
            set_value(TV_ENABLE_OP, NXT_BACKGROUND_COLOR, TV_BG);
            break;
    }
}

void move_down_tv() {
    char parsed_value[3] = "\0";
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
            set_value(TV_INTENSITY_FLT, NXT_VALUE, tv_settings.tv_intensity_val);
            break;
        case TV_INTENSITY_HOVER:
            // Scroll down to P
            tv_settings.curr_hover = TV_P_HOVER;

            // Update the background
            set_value(TV_INTENSITY_FLT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_PROPORTION_FLT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
            set_value(TV_DEAD_TXT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_ENABLE_OP, NXT_BACKGROUND_COLOR, TV_BG);
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
            set_value(TV_PROPORTION_FLT, NXT_VALUE, tv_settings.tv_p_val);
            break;
        case TV_P_HOVER:
            // Scroll down to deadband
            tv_settings.curr_hover = TV_DEADBAND_HOVER;

            // Update the background
            set_value(TV_INTENSITY_FLT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_PROPORTION_FLT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_DEAD_TXT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
            set_value(TV_ENABLE_OP, NXT_BACKGROUND_COLOR, TV_BG);
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
            set_text(TV_DEAD_TXT, NXT_TEXT, int_to_char(tv_settings.tv_deadband_val, parsed_value));
            bzero(parsed_value, 3);
            break;
        case TV_DEADBAND_HOVER:
            // Scroll down to enable
            tv_settings.curr_hover = TV_ENABLE_HOVER;

            // Update the background
            set_value(TV_INTENSITY_FLT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_PROPORTION_FLT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_DEAD_TXT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_ENABLE_OP, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
            break;
        case TV_ENABLE_HOVER:
            // Scroll down to intensity
            tv_settings.curr_hover = TV_INTENSITY_HOVER;
            set_value(TV_INTENSITY_FLT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
            set_value(TV_PROPORTION_FLT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_DEAD_TXT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_ENABLE_OP, NXT_BACKGROUND_COLOR, TV_BG);
            break;
    }
}

void select_tv() {
    // So if we hit select on an already selected item, unselect it (switch to hover)
    switch (tv_settings.curr_hover) {
        case TV_INTENSITY_HOVER:
            tv_settings.curr_hover = TV_INTENSITY_SELECTED;
            set_value(TV_INTENSITY_FLT, NXT_BACKGROUND_COLOR, ORANGE);
            set_value(TV_PROPORTION_FLT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_DEAD_TXT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_ENABLE_OP, NXT_BACKGROUND_COLOR, TV_BG);
            // todo Rot encoder state should let us scroll through value options
            // for now just use buttons for move up and move down
            break;
        case TV_INTENSITY_SELECTED:
            // "submit" -> CAN payload will update automatically? decide
            // Think about edge case when the user leaves the page? Can they without unselecting -> no. What if fault?
            tv_settings.curr_hover = TV_INTENSITY_HOVER;
            set_value(TV_INTENSITY_FLT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
            set_value(TV_PROPORTION_FLT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_DEAD_TXT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_ENABLE_OP, NXT_BACKGROUND_COLOR, TV_BG);
            // rot encoder state goes back to page move instead of value move
            break;
        case TV_P_HOVER:
            tv_settings.curr_hover = TV_P_SELECTED;
            set_value(TV_PROPORTION_FLT, NXT_BACKGROUND_COLOR, ORANGE);
            set_value(TV_INTENSITY_FLT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_DEAD_TXT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_ENABLE_OP, NXT_BACKGROUND_COLOR, TV_BG);
            break;
        case TV_P_SELECTED:
            tv_settings.curr_hover = TV_P_HOVER;
            set_value(TV_PROPORTION_FLT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
            set_value(TV_INTENSITY_FLT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_DEAD_TXT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_ENABLE_OP, NXT_BACKGROUND_COLOR, TV_BG);
            break;
        case TV_DEADBAND_HOVER:
            tv_settings.curr_hover = TV_DEADBAND_SELECTED;
            set_value(TV_PROPORTION_FLT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_INTENSITY_FLT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_DEAD_TXT, NXT_BACKGROUND_COLOR, ORANGE);
            set_value(TV_ENABLE_OP, NXT_BACKGROUND_COLOR, TV_BG);
            break;
        case TV_DEADBAND_SELECTED:
            tv_settings.curr_hover = TV_DEADBAND_HOVER;
            set_value(TV_PROPORTION_FLT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_INTENSITY_FLT, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(TV_DEAD_TXT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
            set_value(TV_ENABLE_OP, NXT_BACKGROUND_COLOR, TV_BG);
            break;
        case TV_ENABLE_HOVER:
            // Don't change the curr_hover
            // Toggle the option
            tv_settings.tv_enable_selected = (tv_settings.tv_enable_selected == 0);

            // Set the option
            set_value(TV_ENABLE_OP, NXT_VALUE, tv_settings.tv_enable_selected);

            // Update CAN as necessary
            break;
    }
}

void update_faults_page() {
    if (fault_buf[0] == 0xFFFF)
    {
        set_text(FAULT_1_TXT, NXT_TEXT, FAULT_NONE_STRING);
    }
    else
    {
        set_text(FAULT_1_TXT, NXT_TEXT, faultArray[fault_buf[0]].screen_MSG);
    }
    if (fault_buf[1] == 0xFFFF)
    {
        set_text(FAULT_2_TXT, NXT_TEXT, FAULT_NONE_STRING);
    }
    else
    {
        set_text(FAULT_2_TXT, NXT_TEXT, faultArray[fault_buf[1]].screen_MSG);
    }

    if (fault_buf[2] == 0xFFFF)
    {
        set_text(FAULT_3_TXT, NXT_TEXT, FAULT_NONE_STRING);
    }
    else
    {
        set_text(FAULT_3_TXT, NXT_TEXT, faultArray[fault_buf[2]].screen_MSG);
    }

    if (fault_buf[3] == 0xFFFF)
    {
        set_text(FAULT_4_TXT, NXT_TEXT, FAULT_NONE_STRING);
    }
    else
    {
        set_text(FAULT_4_TXT, NXT_TEXT, faultArray[fault_buf[3]].screen_MSG);
    }

    if (fault_buf[4] == 0xFFFF)
    {
        set_text(FAULT_5_TXT, NXT_TEXT, FAULT_NONE_STRING);
    }
    else
    {
        set_text(FAULT_5_TXT, NXT_TEXT, faultArray[fault_buf[4]].screen_MSG);
    }
}

void move_up_faults() {
    switch (fault_page.curr_hover) {
        case FAULT1:
            // Wrap around to the last item
            fault_page.curr_hover = CLEAR;

            set_value(FAULT_1_TXT, NXT_BACKGROUND_COLOR, BLACK);
            set_value(CLEAR_FAULTS_TXT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
            break;
        case FAULT2:
            fault_page.curr_hover = FAULT1;

            set_value(FAULT_2_TXT, NXT_BACKGROUND_COLOR, BLACK);
            set_value(FAULT_1_TXT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
            break;
        case FAULT3:
            fault_page.curr_hover = FAULT2;

            set_value(FAULT_3_TXT, NXT_BACKGROUND_COLOR, BLACK);
            set_value(FAULT_2_TXT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
            break;
        case FAULT4:
            fault_page.curr_hover = FAULT3;

            set_value(FAULT_4_TXT, NXT_BACKGROUND_COLOR, BLACK);
            set_value(FAULT_3_TXT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
            break;
        case FAULT5:
            fault_page.curr_hover = FAULT4;

            set_value(FAULT_5_TXT, NXT_BACKGROUND_COLOR, BLACK);
            set_value(FAULT_4_TXT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
            break;
        case CLEAR:
            fault_page.curr_hover = FAULT5;

            set_value(CLEAR_FAULTS_TXT, NXT_BACKGROUND_COLOR, BLACK);
            set_value(FAULT_5_TXT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
            break;
    }
}

void move_down_faults() {
    switch (fault_page.curr_hover) {
        case FAULT1:
            fault_page.curr_hover = FAULT2;

            set_value(FAULT_1_TXT, NXT_BACKGROUND_COLOR, BLACK);
            set_value(FAULT_2_TXT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
            break;
        case FAULT2:
            fault_page.curr_hover = FAULT3;

            set_value(FAULT_2_TXT, NXT_BACKGROUND_COLOR, BLACK);
            set_value(FAULT_3_TXT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
            break;
        case FAULT3:
            fault_page.curr_hover = FAULT4;

            set_value(FAULT_3_TXT, NXT_BACKGROUND_COLOR, BLACK);
            set_value(FAULT_4_TXT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
            break;
        case FAULT4:
            fault_page.curr_hover = FAULT5;

            set_value(FAULT_4_TXT, NXT_BACKGROUND_COLOR, BLACK);
            set_value(FAULT_5_TXT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
            break;
        case FAULT5:
            fault_page.curr_hover = CLEAR;

            set_value(FAULT_5_TXT, NXT_BACKGROUND_COLOR, BLACK);
            set_value(CLEAR_FAULTS_TXT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
            break;
        case CLEAR:
            fault_page.curr_hover = FAULT1;

            set_value(CLEAR_FAULTS_TXT, NXT_BACKGROUND_COLOR, BLACK);
            set_value(FAULT_1_TXT, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
            break;
    }
}

void update_race_page() {
    static uint8_t update_group = 0U;
    if (curr_page != PAGE_RACE) {
        return;
    }

    set_value(BRK_BAR, NXT_VALUE, 0); // TODO BRK BAR
    set_value(THROT_BAR, NXT_VALUE, (int) ((filtered_pedals / 4095.0) * 100));

    // update the speed
    if (can_data.rear_wheel_speeds.stale) {
        set_text(SPEED, NXT_TEXT, "S");
    }
    else {
        // Vehicle Speed [m/s] = Wheel Speed [RPM] * 16 [in] * PI * 0.0254 / 60
        char speed_buf[3] = "\0";
        // set_text(SPEED, NXT_TEXT, int_to_char((uint16_t)((float)MAX(can_data.rear_wheel_speeds.left_speed_sensor, can_data.rear_wheel_speeds.right_speed_sensor) * 0.01 * 0.4474), parsed_value));
        uint16_t speed = ((float)can_data.gps_speed.gps_speed * 0.02237); // TODO macro this magic number
        set_text(SPEED, NXT_TEXT, int_to_char(speed, speed_buf));
        bzero(speed_buf, sizeof(speed_buf));
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
    char temps_buf[5] = "\0"; // TODO adjust buf to proper size (adjust for digits needed)

    if (can_data.rear_motor_temps.stale) {
        set_text(MOT_TEMP, NXT_TEXT, "S");
    }
    else {
        uint8_t motor_temp = MAX(can_data.rear_motor_temps.left_mot_temp, can_data.rear_motor_temps.right_mot_temp);
        int_to_char(motor_temp, temps_buf);
        append_char(temps_buf, 'C', sizeof(temps_buf));

        set_text(MOT_TEMP, NXT_TEXT, temps_buf);
        bzero(temps_buf, sizeof(temps_buf));
    }

    if (can_data.max_cell_temp.stale) {
        set_text(BATT_TEMP, NXT_TEXT, "S");
    }
    else {
        uint16_t batt_temp = can_data.max_cell_temp.max_temp / 10;
        int_to_char(batt_temp, temps_buf);
        append_char(temps_buf, 'C', sizeof(temps_buf));

        set_text(BATT_TEMP, NXT_TEXT, temps_buf);
        bzero(temps_buf, sizeof(temps_buf));
    }

    // TODO update MC_TEMP
}

void update_race_page_group2() {
    if (can_data.main_hb.stale) {
        set_text(CAR_STAT, NXT_TEXT, "S");
        set_value(CAR_STAT, NXT_BACKGROUND_COLOR, BLACK);
    }
    else {
        switch(can_data.main_hb.car_state) {
            case CAR_STATE_PRECHARGING:
                set_value(CAR_STAT, NXT_FONT_COLOR, ORANGE);
                set_text(CAR_STAT, NXT_TEXT, "PRCHG");
                break;
            case CAR_STATE_ENERGIZED:
                set_value(CAR_STAT, NXT_FONT_COLOR, ORANGE);
                set_text(CAR_STAT, NXT_TEXT, "ENER");
                break;
            case CAR_STATE_IDLE:
                set_value(CAR_STAT, NXT_FONT_COLOR, INFO_GRAY);
                set_text(CAR_STAT, NXT_TEXT, "INIT");
                break;
            case CAR_STATE_READY2DRIVE:
                set_value(CAR_STAT, NXT_FONT_COLOR, RACE_GREEN);
                set_text(CAR_STAT, NXT_TEXT, "ON");
                break;
            case CAR_STATE_ERROR:
                set_value(CAR_STAT, NXT_FONT_COLOR, YELLOW);
                set_text(CAR_STAT, NXT_TEXT, "ERR");
                break;
            case CAR_STATE_FATAL:
                set_value(CAR_STAT, NXT_FONT_COLOR, RED);
                set_text(CAR_STAT, NXT_TEXT, "FATAL");
                break;
        }
    }


    // Update the voltage and current
    char batt_buf[5] = "\0"; // 3 digits + 1 unit + \0
    if (can_data.orion_currents_volts.stale) {
        set_text(BATT_VOLT, NXT_TEXT, "S");
        set_text(BATT_CURR, NXT_TEXT, "S");
    }
    else {
        uint16_t voltage = (can_data.orion_currents_volts.pack_voltage / 10);
        int_to_char(voltage, batt_buf);
        append_char(batt_buf, 'V', sizeof(batt_buf));
        set_text(BATT_VOLT, NXT_TEXT, batt_buf);
        bzero(batt_buf, sizeof(batt_buf));

        uint16_t current = (can_data.orion_currents_volts.pack_current / 10);
        int_to_char(current, batt_buf);
        append_char(batt_buf, 'V', sizeof(batt_buf));
        set_text(BATT_CURR, NXT_TEXT, batt_buf);
        bzero(batt_buf, sizeof(batt_buf));
    }
}

void select_race() {
    // there is only one option for now
    tv_settings.tv_enable_selected = (tv_settings.tv_enable_selected == 0);
    set_value(RACE_TV_ON, NXT_VALUE, tv_settings.tv_enable_selected);
}

void append_char(char *str, char ch, size_t max_len) {
    size_t len = 0;
    while (*str != '\0' && len < max_len - 1) {
        str++;
        len++;
    }
    if (len < max_len - 1) {
        *str++ = ch;
        *str = '\0';
    };
}

char *int_to_char(int16_t val, char *val_to_send) {
    char *orig_ptr = val_to_send;
    if (val < 10) {
        if (val < 0) {
            *val_to_send++ = (char)('-');
            val *= -1;
        }
        *val_to_send = (char)(val + 48);
        return orig_ptr;
    }
    else if (val < 100) {
        *val_to_send++ = val / 10 + 48;
        *val_to_send = val % 10 + 48;
        return orig_ptr;
    }
    else {
        *val_to_send++ = val / 100 + 48;
        *val_to_send++ = val % 100 / 10 + 48;
        *val_to_send = val % 10 + 48;
        return orig_ptr;
    }
}

void setFaultIndicator(uint16_t fault, char *element) {
    if (fault == 0xFFFF) {
        set_value(element, NXT_FONT_COLOR, WHITE);
    } else if (checkFault(fault)) {
        set_value(element, NXT_FONT_COLOR, RED);
    }
    set_value(element, NXT_FONT_COLOR, RACE_GREEN);
}

void updateSDCStatus(uint8_t status, char *element) {
    if (status)
    {
        set_value(element, NXT_BACKGROUND_COLOR, GREEN);
    }
    else
    {
        set_value(element, NXT_BACKGROUND_COLOR, RED);
    }
}
