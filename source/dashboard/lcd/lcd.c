#include "lcd.h"
#include "hdd.h"
#include "common/psched/psched.h"
#include "pedals.h"
#include "common/faults/faults.h"
#include "common_defs.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

volatile page_t curr_page;  // Current page displayed on the LCD
volatile page_t prev_page;  // Previous page displayed on the LCD
uint16_t cur_fault_buf_ndx;
uint16_t fault_buf[5] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};

bool sendFirsthalf;

uint8_t display_time;

char *errorText;

static uint8_t preflight;

extern hdd_value_t hdd;

extern uint16_t filtered_pedals;

extern q_handle_t q_tx_can;
extern q_handle_t q_fault_history;

volatile tv_options_t tv;

volatile settings_t settings;   // Data for the settings page 

extern lcd_t lcd_data;

//1 = deadband, 0 = intensity
bool knob;
bool knob_old;

bool midAdjustment;


// Call initially to ensure the LCD is initialized to the proper value - 
// should be replaced with the struct prev page stuff eventually
bool zeroEncoder(volatile int8_t* start_pos)
{
    // Collect initial raw reading from encoder
    uint8_t raw_enc_a = PHAL_readGPIO(ENC_A_GPIO_Port, ENC_A_Pin);
    uint8_t raw_enc_b = PHAL_readGPIO(ENC_B_GPIO_Port, ENC_B_Pin);
    uint8_t raw_res = (raw_enc_b | (raw_enc_a << 1));
    *start_pos = raw_res;
    lcd_data.encoder_position = 0;

    // Set page (leave preflight)
    updatePage();
    return true;
}

// Initialize the LCD screen
// Preflight will be shown on power on, then reset to RACE
void initLCD() {
    curr_page = PAGE_RACE;
    prev_page = PAGE_PREFLIGHT; 
    display_time = 0;
    errorText = 0;
    knob = false;
    knob_old = false;
    midAdjustment = 0;
    tv = (tv_options_t) {0, 0, 0, 0, 0, 0, "\0"};
    settings = (settings_t) {0, 0, 0, 0, 0, 0, 0, 0};
    sendFirsthalf = true;

}

void updatePage() {
    // Only update the encoder if we are on a "selectable" page
    if ((curr_page != PAGE_ERROR) && (curr_page != PAGE_WARNING) && (curr_page != PAGE_FATAL))
    {
        curr_page = lcd_data.encoder_position;
    }

    // If we do not detect a page update (most notably detect if encoder did not move), do nothing
    if (curr_page == prev_page) {
        return;
    }

    // Parsed value represents: 
    char parsed_value[3] = "\0";

    // Parse the page that was passed into the function
    switch (curr_page) {
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
        case PAGE_SETTINGS:
            // Show page
            prev_page = PAGE_SETTINGS;
            set_page(SETTINGS_STRING);

            settings.curr_hover = DT_FAN_HOVER;                                     // Set hover
            set_value(DT_FAN_TXT, NXT_BACKGROUND_COLOR, SETTINGS_HOVER_BG);         // Set t2 with settings hover
            set_value(DT_FAN_BAR, NXT_VALUE, settings.d_fan_val);                   // Set progress bar for j0
            set_value(DT_FAN_VAL, NXT_FONT_COLOR, SETTINGS_BAR_BG);                         // Set color for t8 (background of bar?)
            set_text(DT_FAN_VAL, NXT_TEXT, int_to_char(settings.d_fan_val, parsed_value));  // Set fan value for t8
            bzero(parsed_value, 3);                                                         // Clear our char buffer
            
            // Set drivetrain pump selector color 
            if (settings.d_pump_selected) {     
                set_value(DT_PUMP_OP, NXT_FONT_COLOR, SETTINGS_UV_SELECT);
            }
            else {
                set_value(DT_PUMP_OP, NXT_BACKGROUND_COLOR, SETTINGS_HOVER_BG);
            }

            // Set drivetrain pump selector status
            set_value(DT_PUMP_OP, NXT_VALUE, settings.d_pump_selected);

            // Set Battery fan c3 (Pump 1?)
            // todo: Why is this here?
            if (settings.b_fan2_selected) {
                set_value(B_FAN2_OP, NXT_FONT_COLOR, SETTINGS_UV_SELECT);
            }
            else {
                set_value(B_FAN2_OP, NXT_BACKGROUND_COLOR, SETTINGS_HOVER_BG);
            }

            // Set value for c3 battery pump 1
            set_value(B_FAN2_OP, NXT_VALUE, settings.b_fan2_selected);
            if (settings.b_pump_selected) {
                set_value(B_PUMP_OP, NXT_FONT_COLOR, SETTINGS_UV_SELECT);
            }
            else {
                set_value(B_PUMP_OP, NXT_BACKGROUND_COLOR, SETTINGS_HOVER_BG);
            }

            // Set Battery Pump 2 value
            set_value(B_PUMP_OP, NXT_VALUE, settings.b_pump_selected);

            // Set battery fan bar, text, color
            set_value(B_FAN1_BAR, NXT_VALUE, settings.b_fan_val);
            set_text(B_FAN1_VAL, NXT_TEXT, int_to_char(settings.b_fan_val, parsed_value));
            set_value(B_FAN1_VAL, NXT_FONT_COLOR, SETTINGS_BAR_BG);

            break;
        case PAGE_FAULTS:
            prev_page = PAGE_FAULTS;
            set_page(FAULT_STRING);
            if (fault_buf[0] == 0xFFFF)
            {
                set_text(FAULT_1_TXT, NXT_TEXT, FAULT_NONE_STRING);
                set_value(FLT_STAT_1_TXT, NXT_BACKGROUND_COLOR, WHITE);
            }
            else
            {
                set_text(FAULT_1_TXT, NXT_TEXT, faultArray[fault_buf[0]].screen_MSG);
                if (checkFault(fault_buf[fault_buf[0]]) == false)
                {
                    set_value(FLT_STAT_1_TXT, NXT_BACKGROUND_COLOR, RACE_GREEN);
                }
                else
                {
                    set_value(FLT_STAT_1_TXT, NXT_BACKGROUND_COLOR, RED);
                }
            }
            if (fault_buf[1] == 0xFFFF)
            {
                set_text(FAULT_2_TXT, NXT_TEXT, FAULT_NONE_STRING);
                set_value(FLT_STAT_2_TXT, NXT_BACKGROUND_COLOR, WHITE);
            }
            else
            {
                set_text(FAULT_2_TXT, NXT_TEXT, faultArray[fault_buf[1]].screen_MSG);
                if (checkFault(fault_buf[fault_buf[1]]) == false)
                {
                    set_value(FLT_STAT_2_TXT, NXT_BACKGROUND_COLOR, RACE_GREEN);
                }
                else
                {
                    set_value(FLT_STAT_2_TXT, NXT_BACKGROUND_COLOR, RED);
                }
            }

            if (fault_buf[2] == 0xFFFF)
            {
                set_text(FAULT_3_TXT, NXT_TEXT, FAULT_NONE_STRING);
                set_value(FLT_STAT_3_TXT, NXT_BACKGROUND_COLOR, WHITE);
            }
            else
            {
                set_text(FAULT_3_TXT, NXT_TEXT, faultArray[fault_buf[2]].screen_MSG);
                if (checkFault(fault_buf[fault_buf[2]]) == false)
                {
                    set_value(FLT_STAT_3_TXT, NXT_BACKGROUND_COLOR, RACE_GREEN);
                }
                else
                {
                    set_value(FLT_STAT_3_TXT, NXT_BACKGROUND_COLOR, RED);
                }
            }

            if (fault_buf[3] == 0xFFFF)
            {
                set_text(FAULT_4_TXT, NXT_TEXT, FAULT_NONE_STRING);
                set_value(FLT_STAT_4_TXT, NXT_BACKGROUND_COLOR, WHITE);
            }
            else
            {
                set_text(FAULT_4_TXT, NXT_TEXT, faultArray[fault_buf[3]].screen_MSG);
                if (checkFault(fault_buf[fault_buf[3]]) == false)
                {
                    set_value(FLT_STAT_4_TXT, NXT_BACKGROUND_COLOR, RACE_GREEN);
                }
                else
                {
                    set_value(FLT_STAT_4_TXT, NXT_BACKGROUND_COLOR, RED);
                }
            }

            if (fault_buf[4] == 0xFFFF)
            {
                set_text(FAULT_5_TXT, NXT_TEXT, FAULT_NONE_STRING);
                set_value(FLT_STAT_5_TXT, NXT_BACKGROUND_COLOR, WHITE);
            }
            else
            {
                set_text(FAULT_5_TXT, NXT_TEXT, faultArray[fault_buf[4]].screen_MSG);
                if (checkFault(fault_buf[fault_buf[4]]) == false)
                {
                    set_value(FLT_STAT_5_TXT, NXT_BACKGROUND_COLOR, RACE_GREEN);
                }
                else
                {
                    set_value(FLT_STAT_5_TXT, NXT_BACKGROUND_COLOR, RED);
                }
            }
            break;
        case PAGE_TV:
            prev_page = PAGE_TV;
            set_page(TV_STRING);
            tv.p_hover = true;
            set_value(P_BAR, NXT_VALUE, tv.yaw_p_val);
            set_value(I_BAR, NXT_VALUE, tv.yaw_i_val);
            switch (tv.p_selected) {
                case NONE_SELECTED:
                    set_value(P_BAR, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
                    set_value(P_BAR, NXT_FONT_COLOR, TV_HOVER_FG_P);
                    break;
                case P_SELECTED:
                    set_value(P_BAR, NXT_BACKGROUND_COLOR, BLACK);
                    set_value(P_BAR, NXT_FONT_COLOR, WHITE);
                    break;
                case I_SELECTED:
                    set_value(I_BAR, NXT_BACKGROUND_COLOR, BLACK);
                    set_value(I_BAR, NXT_FONT_COLOR, WHITE);
                    break;
            }
            send_p_val();
            send_i_val();
            char value_to_send[2] = "\0";
            if (hdd.intensity_pos > 9) {
                value_to_send[0] = (hdd.intensity_pos / 10) + 48;
                value_to_send[1] = (hdd.intensity_pos % 10) + 48;
            }
            else {
                value_to_send[0] = hdd.intensity_pos + 48;
            }
            set_text(TV_IN_TXT, NXT_TEXT, value_to_send);
            tv.deadband_msg = get_deadband();
            set_text(TV_DB_TXT, NXT_TEXT, tv.deadband_msg);
            break;
        // case PAGE_KNOBS:
        //     switch (display_time++) {
        //         case 0:
        //             if (knob) {
        //                 set_page(DEADBAND_STRING);
        //                 tv.deadband_msg = get_deadband();
        //                 set_text(KNB_TXT, NXT_TEXT, tv.deadband_msg);
        //             }
        //             else {
        //                 set_page(INTENSITY_STRING);
        //                 char value_to_send[2] = "\0";
        //                 if (hdd.intensity_pos > 9) {
        //                     value_to_send[0] = (hdd.intensity_pos / 10) + 48;
        //                     value_to_send[1] = (hdd.intensity_pos % 10) + 48;
        //                 }
        //                 else {
        //                     value_to_send[0] = hdd.intensity_pos + 48;
        //                 }
        //                 set_text(KNB_TXT, NXT_TEXT, value_to_send);
        //             }
        //             break;
        //         case 5:
        //             set_value(TIME_BAR, NXT_VALUE, 100);
        //             display_time = 0;
        //             curr_page = prev_page;
        //             prev_page = PAGE_PREFLIGHT;
        //             updatePage();
        //             break;
        //         default:
        //             set_value(TIME_BAR, NXT_VALUE, (display_time * 20));
        //             break;
        //     }
        //     break;
        //default:
            // switch(display_time++) {
            //     case 0:
            //         switch(curr_page) {
            //             case PAGE_ERROR:
            //                 set_page(ERR_STRING);
            //                 break;
            //             case PAGE_WARNING:
            //                 set_page(WARN_STRING);
            //                 break;
            //             case PAGE_FATAL:
            //                 set_page(FATAL_STRING);
            //                 break;
            //         }
            //         set_value(TIME_BAR, NXT_VALUE, 0);
            //         set_text(ERR_TXT, NXT_TEXT, errorText);
            //         break;
            //     case 10:
            //         set_value(TIME_BAR, NXT_VALUE, 100);
            //         curr_page = prev_page;
            //         prev_page = PAGE_PREFLIGHT;
            //         updatePage();
            //         display_time = 0;
            //         break;
            //     default:
            //         set_value(TIME_BAR, NXT_VALUE, (display_time * 10));
            //         break;
            // }

    }
}

// void moveLeft() {
//     switch(curr_page) {
//         case PAGE_RACE:
//             curr_page = PAGE_TV;
//             updatePage();
//             break;
//         case PAGE_SETTINGS:
//             if (settings.curr_hover < DT_FAN_SELECT) {
//                 curr_page = PAGE_DATA;
//                 updatePage();
//             }
//             break;
//         case PAGE_DATA:
//             curr_page = PAGE_RACE;
//             updatePage();
//             break;
//         case PAGE_TV:
//             if (tv.p_selected == NONE_SELECTED) {
//                 curr_page = PAGE_SETTINGS;
//                 updatePage();
//             }
//             break;
//         default:
//             curr_page = prev_page;
//             prev_page = PAGE_PREFLIGHT;
//             display_time = 0;
//             updatePage();
//             break;
//     }
// }

// void moveRight() {
//     switch(curr_page) {
//         case PAGE_RACE:
//             curr_page = PAGE_DATA;
//             updatePage();
//             break;
//         case PAGE_SETTINGS:
//             if (settings.curr_hover < DT_FAN_SELECT) {
//                 curr_page = PAGE_TV;
//                 updatePage();
//             }
//             break;
//         case PAGE_DATA:
//             curr_page = PAGE_SETTINGS;
//             updatePage();
//             break;
//         case PAGE_TV:
//             if (tv.p_selected == NONE_SELECTED) {
//                 curr_page = PAGE_RACE;
//                 updatePage();
//             }
//             break;
//         default:
//             curr_page = prev_page;
//             prev_page = PAGE_PREFLIGHT;
//             display_time = 0;
//             updatePage();
//             break;
//     }
// }

void moveUp() {
    // if (((curr_page != PAGE_SETTINGS) || (curr_page != PAGE_TV))&& curr_page > PAGE_TV) {
    //     curr_page = prev_page;
    //     prev_page = PAGE_RACE; 
    //     display_time = 0;
    //     updatePage();
    //     return;
    // }
    if (curr_page == PAGE_TV) {
        if (tv.p_hover && tv.p_selected == NONE_SELECTED) {
            tv.p_hover = false;
            set_value(P_BAR, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(P_BAR, NXT_FONT_COLOR, TV_P_FG);
            set_value(I_BAR, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
            set_value(I_BAR, NXT_FONT_COLOR, TV_HOVER_FG_I);
        }
        else if (tv.p_selected == NONE_SELECTED) {
            tv.p_hover = true;
            set_value(P_BAR, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
            set_value(P_BAR, NXT_FONT_COLOR, TV_HOVER_FG_P);
            set_value(I_BAR, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(I_BAR, NXT_FONT_COLOR, TV_I_FG);
        }
        else if (tv.p_selected == P_SELECTED){
            tv.yaw_p_val = ((tv.yaw_p_val >= 100) ? 0 : tv.yaw_p_val + 10);
            set_value(P_BAR, NXT_VALUE, tv.yaw_p_val);
            send_p_val();
        }
        else {
            tv.yaw_i_val = ((tv.yaw_i_val >= 100) ? 0 : tv.yaw_i_val + 10);
            set_value(I_BAR, NXT_VALUE, tv.yaw_i_val);
            send_i_val();
        }
    }
    else if (curr_page == PAGE_SETTINGS) {
        char parsed_value[3] = "\0";
        switch (settings.curr_hover) {
            case DT_FAN_HOVER:
                set_value(DT_FAN_TXT, NXT_BACKGROUND_COLOR, SETTINGS_BG);
                set_value(B_PUMP_TXT, NXT_BACKGROUND_COLOR, SETTINGS_HOVER_BG);
                settings.curr_hover = PUMP_HOVER;
                break;
            case DT_PUMP_HOVER:
                set_value(DT_PUMP_TXT, NXT_BACKGROUND_COLOR, SETTINGS_BG);
                set_value(DT_FAN_TXT, NXT_BACKGROUND_COLOR, SETTINGS_HOVER_BG);
                settings.curr_hover = DT_FAN_HOVER;
                break;
            case FAN1_HOVER:
                set_value(B_FAN1_TXT, NXT_BACKGROUND_COLOR, SETTINGS_BG);
                set_value(DT_PUMP_TXT, NXT_BACKGROUND_COLOR, SETTINGS_HOVER_BG);
                settings.curr_hover = DT_PUMP_HOVER;
                break;
            case FAN2_HOVER:
                set_value(B_FAN2_TXT, NXT_BACKGROUND_COLOR, SETTINGS_BG);
                set_value(B_FAN1_TXT, NXT_BACKGROUND_COLOR, SETTINGS_HOVER_BG);
                settings.curr_hover = FAN1_HOVER;
                break;
            case PUMP_HOVER:
                set_value(B_PUMP_TXT, NXT_BACKGROUND_COLOR, SETTINGS_BG);
                set_value(B_FAN2_TXT, NXT_BACKGROUND_COLOR, SETTINGS_HOVER_BG);
                settings.curr_hover = FAN2_HOVER;
                break;
            case DT_FAN_SELECT:
                settings.d_fan_val /= 10;
                settings.d_fan_val *= 10;
                settings.d_fan_val = (settings.d_fan_val == 100) ? 0 : settings.d_fan_val + 10;
                set_value(DT_FAN_BAR, NXT_VALUE, settings.d_fan_val);
                set_text(DT_FAN_VAL, NXT_TEXT, int_to_char(settings.d_fan_val, parsed_value));
                set_value(DT_FAN_VAL, NXT_FONT_COLOR, BLACK);
                break;
            case FAN1_SELECT:
                settings.b_fan_val /= 10;
                settings.b_fan_val *= 10;
                settings.b_fan_val = (settings.b_fan_val == 100) ? 0 : settings.b_fan_val + 10;
                set_value(B_FAN1_BAR, NXT_VALUE, settings.b_fan_val);
                set_text(B_FAN1_VAL, NXT_TEXT, int_to_char(settings.b_fan_val, parsed_value));
                set_value(B_FAN1_VAL, NXT_FONT_COLOR, BLACK);
                break;
        }
    }
}

void moveDown() {
    // if (((curr_page != PAGE_SETTINGS) || (curr_page != PAGE_TV))&& curr_page > PAGE_TV) {
    //     curr_page = prev_page;
    //     prev_page = PAGE_RACE; 
    //     display_time = 0;
    //     updatePage();
    //     return;
    // }
    if (curr_page == PAGE_TV) {
        if (tv.p_hover && tv.p_selected == NONE_SELECTED) {
            tv.p_hover = false;
            set_value(P_BAR, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(P_BAR, NXT_FONT_COLOR, TV_P_FG);
            set_value(I_BAR, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
            set_value(I_BAR, NXT_FONT_COLOR, TV_HOVER_FG_I);
        }
        else if (tv.p_selected == NONE_SELECTED) {
            tv.p_hover = true;
            set_value(P_BAR, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
            set_value(P_BAR, NXT_FONT_COLOR, TV_HOVER_FG_P);
            set_value(I_BAR, NXT_BACKGROUND_COLOR, TV_BG);
            set_value(I_BAR, NXT_FONT_COLOR, TV_I_FG);
        }
        else if (tv.p_selected == P_SELECTED){
            tv.yaw_p_val = ((tv.yaw_p_val <= 0) ? 100 : tv.yaw_p_val - 10);
            set_value(P_BAR, NXT_VALUE, tv.yaw_p_val);
            send_p_val();
        }
        else {
            tv.yaw_i_val = ((tv.yaw_i_val <= 0) ? 100 : tv.yaw_i_val - 10);
            set_value(I_BAR, NXT_VALUE, tv.yaw_i_val);
            send_i_val();
        }
    }
    else if (curr_page == PAGE_SETTINGS) {
        char parsed_value[3] = "\0";
        switch (settings.curr_hover) {
            case DT_FAN_HOVER:
                set_value(DT_FAN_TXT, NXT_BACKGROUND_COLOR, SETTINGS_BG);
                set_value(DT_PUMP_TXT, NXT_BACKGROUND_COLOR, SETTINGS_HOVER_BG);
                settings.curr_hover = DT_PUMP_HOVER;
                break;
            case DT_PUMP_HOVER:
                set_value(DT_PUMP_TXT, NXT_BACKGROUND_COLOR, SETTINGS_BG);
                set_value(B_FAN1_TXT, NXT_BACKGROUND_COLOR, SETTINGS_HOVER_BG);
                settings.curr_hover = FAN1_HOVER;
                break;
            case FAN1_HOVER:
                set_value(B_FAN1_TXT, NXT_BACKGROUND_COLOR, SETTINGS_BG);
                set_value(B_FAN2_TXT, NXT_BACKGROUND_COLOR, SETTINGS_HOVER_BG);
                settings.curr_hover = FAN2_HOVER;
                break;
            case FAN2_HOVER:
                set_value(B_FAN2_TXT, NXT_BACKGROUND_COLOR, SETTINGS_BG);
                set_value(B_PUMP_TXT, NXT_BACKGROUND_COLOR, SETTINGS_HOVER_BG);
                settings.curr_hover = PUMP_HOVER;
                break;
            case PUMP_HOVER:
                set_value(B_PUMP_TXT, NXT_BACKGROUND_COLOR, SETTINGS_BG);
                set_value(DT_FAN_TXT, NXT_BACKGROUND_COLOR, SETTINGS_HOVER_BG);
                settings.curr_hover = DT_FAN_HOVER;
                break;
            case DT_FAN_SELECT:
                settings.d_fan_val /= 10;
                settings.d_fan_val *= 10;
                settings.d_fan_val = (settings.d_fan_val == 0) ? 100 : settings.d_fan_val - 10;
                set_value(DT_FAN_BAR, NXT_VALUE, settings.d_fan_val);
                set_text(DT_FAN_VAL, NXT_TEXT, int_to_char(settings.d_fan_val, parsed_value));
                set_value(DT_FAN_VAL, NXT_FONT_COLOR, BLACK);
                break;
            case FAN1_SELECT:
                settings.b_fan_val /= 10;
                settings.b_fan_val *= 10;
                settings.b_fan_val = (settings.b_fan_val == 0) ? 100 : settings.b_fan_val - 10;
                set_value(B_FAN1_BAR, NXT_VALUE, settings.b_fan_val);
                set_text(B_FAN1_VAL, NXT_TEXT, int_to_char(settings.b_fan_val, parsed_value));
                set_value(B_FAN1_VAL, NXT_FONT_COLOR, BLACK);
                break;
        }
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
        updatePage();
    }
    // if (((curr_page != PAGE_SETTINGS) || (curr_page != PAGE_TV))&& curr_page > PAGE_TV) {
    //     curr_page = prev_page;
    //     prev_page = PAGE_RACE;  
    //     display_time = 0;
    //     updatePage();
    //     return;
    // }
    else if (curr_page == PAGE_TV) {
        if (tv.p_hover && tv.p_selected == NONE_SELECTED) {
            set_value(P_BAR, NXT_BACKGROUND_COLOR, BLACK);
            set_value(P_BAR, NXT_FONT_COLOR, WHITE);
            tv.p_selected = P_SELECTED;
        }
        else if (tv.p_selected == NONE_SELECTED) {
            set_value(I_BAR, NXT_BACKGROUND_COLOR, BLACK);
            set_value(I_BAR, NXT_FONT_COLOR, WHITE);
            tv.p_selected = I_SELECTED;
        }
        else if (tv.p_selected == P_SELECTED){
            tv.p_selected = NONE_SELECTED;
            tv.p_hover = true;
            set_value(P_BAR, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
            set_value(P_BAR, NXT_FONT_COLOR, TV_HOVER_FG_P);
        }
        else {
            tv.p_selected = NONE_SELECTED;
            set_value(I_BAR, NXT_BACKGROUND_COLOR, TV_HOVER_BG);
            set_value(I_BAR, NXT_FONT_COLOR, TV_HOVER_FG_I);
        }
    }
    else if (curr_page == PAGE_SETTINGS) {
        switch (settings.curr_hover) {
            case DT_FAN_HOVER:
                // settings.d_fan_selected = !settings.d_fan_selected;
                // set_value(DT_FAN_BAR, NXT_VALUE, settings.d_fan_selected);
                settings.curr_hover = DT_FAN_SELECT;
                set_value(DT_FAN_TXT, NXT_VALUE, SETTINGS_BG);
                set_value(DT_FAN_BAR, NXT_BACKGROUND_COLOR, WHITE);
                set_value(DT_FAN_BAR, NXT_FONT_COLOR, BLACK);
                return;
            case DT_PUMP_HOVER:
                settings.d_pump_selected = !settings.d_pump_selected;
                if (settings.d_pump_selected) {
                    set_value(DT_PUMP_OP, NXT_FONT_COLOR, SETTINGS_UV_SELECT);
                    set_value(DT_PUMP_OP, NXT_BACKGROUND_COLOR, SETTINGS_BG);
                }
                else {
                    set_value(DT_PUMP_OP, NXT_BACKGROUND_COLOR, SETTINGS_HOVER_BG);
                }
                set_value(DT_PUMP_OP, NXT_VALUE, settings.d_pump_selected);
                break;
            case FAN1_HOVER:
                // settings.b_fan1_selected = !settings.b_fan1_selected;
                // set_value(B_FAN1_BAR, NXT_VALUE, settings.b_fan1_selected);
                settings.curr_hover = FAN1_SELECT;
                set_value(B_FAN1_TXT, NXT_VALUE, SETTINGS_BG);
                set_value(B_FAN1_BAR, NXT_BACKGROUND_COLOR, WHITE);
                set_value(B_FAN1_BAR, NXT_FONT_COLOR, BLACK);
                break;
            case FAN2_HOVER:
                settings.b_fan2_selected = !settings.b_fan2_selected;
                if (settings.b_fan2_selected) {
                    set_value(B_FAN2_OP, NXT_FONT_COLOR, SETTINGS_UV_SELECT);
                    set_value(B_FAN2_OP, NXT_BACKGROUND_COLOR, SETTINGS_BG);
                }
                else {
                    set_value(B_FAN2_OP, NXT_BACKGROUND_COLOR, SETTINGS_HOVER_BG);
                }
                set_value(B_FAN2_OP, NXT_VALUE, settings.b_fan2_selected);
                break;
            case PUMP_HOVER:
                settings.b_pump_selected = !settings.b_pump_selected;
                if (settings.b_pump_selected) {
                set_value(B_PUMP_OP, NXT_FONT_COLOR, SETTINGS_UV_SELECT);
                set_value(B_PUMP_OP, NXT_BACKGROUND_COLOR, SETTINGS_BG);
                }
                else {
                    set_value(B_PUMP_OP, NXT_BACKGROUND_COLOR, SETTINGS_HOVER_BG);
                }
                set_value(B_PUMP_OP, NXT_VALUE, settings.b_pump_selected);
                break;
            case DT_FAN_SELECT:
                settings.curr_hover = DT_FAN_HOVER;
                set_value(DT_FAN_TXT, NXT_VALUE, SETTINGS_HOVER_BG);
                set_value(DT_FAN_BAR, NXT_BACKGROUND_COLOR, SETTINGS_BAR_BG);
                set_value(DT_FAN_BAR, NXT_FONT_COLOR, SETTINGS_BAR_FG);
                set_value(DT_FAN_VAL, NXT_FONT_COLOR, SETTINGS_BAR_BG);
                break;
            case FAN1_SELECT:
                settings.curr_hover = FAN1_HOVER;
                set_value(B_FAN1_TXT, NXT_VALUE, SETTINGS_HOVER_BG);
                set_value(B_FAN1_BAR, NXT_BACKGROUND_COLOR, SETTINGS_BAR_BG);
                set_value(B_FAN1_BAR, NXT_FONT_COLOR, SETTINGS_BAR_FG);
                set_value(B_FAN1_VAL, NXT_FONT_COLOR, SETTINGS_BAR_BG);
                break;
        }
        SEND_COOLING_DRIVER_REQUEST(q_tx_can, settings.d_pump_selected, settings.d_fan_val, settings.b_fan2_selected, settings.b_pump_selected, settings.b_fan_val);
    }
}

void updateShutdownCircuitDisplay() {
    // Grab status from the CAN message
    // Update local struct 
    // Display changes 
    


}

void updateFaultDisplay() {

    // No new fault to display
    if (qIsEmpty(&q_fault_history) && (most_recent_latched == 0xFFFF))
    {
        return;
    }

    // Track if we alrady have this fault in the display buffer
    bool faultAlreadyInBuffer = false;
    bool pageUpdateRequired = false;   

    // Display time can likely go
    display_time = 0;

    // Process up to 5 faults each time for now
    for (int i = 0; i < 5; i++)
    {
        faultAlreadyInBuffer = false;
        uint16_t next_to_check = 0xFFFF;

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
                // If fault is currently not in our fault buffer, replace it if the current fault is cleared, 
                //  or if the new fault has higher priority
                if (fault_buf[cur_fault_buf_ndx] != 0xFFFF)
                {
                    if ((checkFault(fault_buf[cur_fault_buf_ndx]) == false ) || 
                    (faultArray[next_to_check].priority > faultArray[fault_buf[cur_fault_buf_ndx]].priority))
                    {
                        fault_buf[cur_fault_buf_ndx] = next_to_check; 
                        pageUpdateRequired = true;           
                    }
                    else
                    {
                        // Put back in the queue if it wasn't processed
                        qSendToBack(&q_fault_history, &next_to_check);
                    }
                }
                else
                {
                    // Empty slot just insert
                    fault_buf[cur_fault_buf_ndx] = next_to_check;
                    pageUpdateRequired = true;   
                }
                cur_fault_buf_ndx = (cur_fault_buf_ndx + 1) % 5;
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
        curr_page = faultArray[most_recent_latched].priority + 6;
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

void update_data_pages() {
    char parsed_value[3] = "\0";
    switch (curr_page) {
        case PAGE_RACE:
            set_value(POW_LIM_BAR, NXT_VALUE, 0);
            set_value(THROT_BAR, NXT_VALUE, (int) ((filtered_pedals / 4095.0) * 100));
            if (can_data.rear_wheel_speeds.stale) {
                set_text(SPEED, NXT_TEXT, "S");
            }
            else {
                set_text(SPEED, NXT_TEXT, int_to_char((uint16_t)((float)MAX(can_data.rear_wheel_speeds.left_speed_mc, can_data.rear_wheel_speeds.right_speed_mc) / GEAR_RATIO), parsed_value));
                bzero(parsed_value, 3);
            }
            if (sendFirsthalf) {
                if (can_data.rear_motor_currents_temps.stale) {
                    set_text(MOT_TEMP, NXT_TEXT, "S");
                }
                else {
                    set_text(MOT_TEMP, NXT_TEXT, int_to_char(MAX(can_data.rear_motor_currents_temps.left_temp, can_data.rear_motor_currents_temps.right_temp), parsed_value));
                    bzero(parsed_value, 3);
                }
                if (can_data.gearbox.stale) {
                    set_text(GEAR_TEMP, NXT_TEXT, "S");
                }
                else {
                    set_text(GEAR_TEMP, NXT_TEXT, int_to_char(MAX(can_data.gearbox.l_temp, can_data.gearbox.r_temp), parsed_value));
                    bzero(parsed_value, 3);
                }
                set_text(TV_FL, NXT_TEXT, "S");
                set_text(TV_FR, NXT_TEXT, "S");
                set_text(TV_LR, NXT_TEXT, "S");
                set_text(TV_RR, NXT_TEXT, "S");
                sendFirsthalf = false;
            }
            else {
                if (can_data.main_hb.stale) {
                    set_text(CAR_STAT, NXT_TEXT, "S");
                    set_value(CAR_STAT, NXT_BACKGROUND_COLOR, BLACK);
                }
                else {
                    switch(can_data.main_hb.car_state) {
                        case CAR_STATE_IDLE:
                            if (can_data.main_hb.precharge_state == 0) {
                                set_value(CAR_STAT, NXT_BACKGROUND_COLOR, INFO_GRAY);
                                set_text(CAR_STAT, NXT_TEXT, "INIT");
                            }
                            else {
                                set_value(CAR_STAT, NXT_BACKGROUND_COLOR, ORANGE);
                                set_text(CAR_STAT, NXT_TEXT, "PRCHG");
                            }
                            break;
                        case CAR_STATE_READY2DRIVE:
                            set_value(CAR_STAT, NXT_BACKGROUND_COLOR, RACE_GREEN);
                            set_text(CAR_STAT, NXT_TEXT, "ON");
                            break;
                        case CAR_STATE_ERROR:
                            set_value(CAR_STAT, NXT_BACKGROUND_COLOR, YELLOW);
                            set_text(CAR_STAT, NXT_TEXT, "ERR");
                            break;
                        case CAR_STATE_FATAL:
                            set_value(CAR_STAT, NXT_BACKGROUND_COLOR, RED);
                            set_text(CAR_STAT, NXT_TEXT, "FATAL");
                            break;
                    }
                }
                if (can_data.max_cell_temp.stale) {
                    set_text(BATT_TEMP, NXT_TEXT, "S");
                }
                else {
                    set_text(BATT_TEMP, NXT_TEXT, int_to_char((can_data.max_cell_temp.max_temp / 100), parsed_value));
                    bzero(parsed_value, 3);
                }
                if (can_data.orion_currents_volts.stale) {
                    set_text(BATT_VOLT, NXT_TEXT, "S");
                    set_text(BATT_CURR, NXT_TEXT, "S");
                }
                else {
                    set_text(BATT_VOLT, NXT_TEXT, int_to_char((can_data.orion_currents_volts.pack_voltage / 10), parsed_value));
                    bzero(parsed_value, 3);
                    set_text(BATT_CURR, NXT_TEXT, int_to_char((can_data.orion_currents_volts.pack_current / 10), parsed_value));
                    bzero(parsed_value, 3);
                }
                sendFirsthalf = true;
            }
            // set_text(GEAR_TEMP, NXT_TEXT, "S");
            break;
        case PAGE_DATA:
            set_value(POW_LIM_BAR, NXT_VALUE, 0);
            set_value(THROT_BAR, NXT_VALUE, (int) ((filtered_pedals / 4095.0) * 100));
            break;
    }
}

//1 = deadband, 0 = intensity
// void knobDisplay() {
//     if (preflight < 2) {
//         preflight++;
//         return;
//     }
//     if (curr_page == PAGE_KNOBS) {
//         if (knob_old == knob && knob) {
//             display_time = 1;
//             tv.deadband_msg = get_deadband();
//             set_text(KNB_TXT, NXT_TEXT, tv.deadband_msg);
//             return;
//         }
//         else if (knob_old == knob && !knob) {
//             display_time = 1;
//             tv.intensity = hdd.intensity_pos;
//             char value_to_send[2] = "\0";
//             if (hdd.intensity_pos > 9) {
//                 value_to_send[0] = (hdd.intensity_pos / 10) + 48;
//                 value_to_send[1] = (hdd.intensity_pos % 10) + 48;
//             }
//             else {
//                 value_to_send[0] = hdd.intensity_pos + 48;
//             }
//             set_text(KNB_TXT, NXT_TEXT, value_to_send);
//             knob_old = knob;
//             return;
//         }
//     }
//     curr_page = PAGE_KNOBS;
//     display_time = 0;
//     knob_old = knob;
//     updatePage();
// }

void coolant_out_CALLBACK(CanParsedData_t* msg_data_a) {
    char parsed_value[3] = "\0";
    if (curr_page != PAGE_SETTINGS) {
        settings.d_pump_selected = msg_data_a->coolant_out.dt_pump;
        settings.b_fan2_selected = msg_data_a->coolant_out.bat_pump;
        settings.b_pump_selected = msg_data_a->coolant_out.bat_pump_aux;
        return;
    }
    if (settings.curr_hover != DT_FAN_SELECT) {
        settings.d_fan_val = msg_data_a->coolant_out.dt_fan;
        set_value(DT_FAN_BAR, NXT_VALUE, settings.d_fan_val);
        set_value(DT_FAN_VAL, NXT_FONT_COLOR, BLACK);
        set_text(DT_FAN_VAL, NXT_TEXT, int_to_char(settings.d_fan_val, parsed_value));
        bzero(parsed_value, 3);
    }
    if (settings.curr_hover != FAN1_SELECT) {
        settings.b_fan_val = msg_data_a->coolant_out.bat_fan;
        set_value(B_FAN1_BAR, NXT_VALUE, settings.b_fan_val);
        set_value(B_FAN1_VAL, NXT_FONT_COLOR, settings.b_fan_val);
        set_text(B_FAN1_VAL, NXT_TEXT, int_to_char(settings.b_fan_val, parsed_value));
        bzero(parsed_value, 3);
    }
    set_value(DT_PUMP_OP, NXT_FONT_COLOR, BLACK);
    set_value(B_FAN2_OP, NXT_FONT_COLOR, BLACK);
    set_value(B_PUMP_OP, NXT_FONT_COLOR, BLACK);
    set_value(DT_PUMP_OP, NXT_BACKGROUND_COLOR, SETTINGS_BG);
    set_value(B_FAN2_OP, NXT_BACKGROUND_COLOR, SETTINGS_BG);
    set_value(B_PUMP_OP, NXT_BACKGROUND_COLOR, SETTINGS_BG);
    settings.d_pump_selected = msg_data_a->coolant_out.dt_pump;
    settings.b_fan2_selected = msg_data_a->coolant_out.bat_pump;
    settings.b_pump_selected = msg_data_a->coolant_out.bat_pump_aux;
    set_value(DT_PUMP_OP, NXT_VALUE, settings.d_pump_selected);
    set_value(B_FAN2_OP, NXT_VALUE, settings.b_fan2_selected);
    set_value(B_PUMP_OP, NXT_VALUE, settings.b_pump_selected);

}


void send_p_val() {
    if (tv.yaw_p_val == 100) {
        set_text(P_TXT, NXT_TEXT, "100");
    }
    else if (tv.yaw_p_val == 0) {
        set_text(P_TXT, NXT_TEXT, "0");
    }
    else {
        char msg_to_send[2] = "\0";
        msg_to_send[0] = (tv.yaw_p_val / 10) + 48;
        msg_to_send[1] = (tv.yaw_p_val % 10) + 48;
        set_text(P_TXT, NXT_TEXT, msg_to_send);
    }
}

void send_i_val() {
    if (tv.yaw_i_val == 100) {
        set_text(I_TXT, NXT_TEXT, "100");
    }
    else if (tv.yaw_i_val == 0) {
        set_text(I_TXT, NXT_TEXT, "0");
    }
    else {
        char msg_to_send[2] = "\0";
        msg_to_send[0] = (tv.yaw_i_val / 10) + 48;
        msg_to_send[1] = (tv.yaw_i_val % 10) + 48;
        set_text(I_TXT, NXT_TEXT, msg_to_send);
    }
}

char *get_deadband() {
    switch (hdd.deadband_pos) {
        case 0:
            return "0";
        case 1:
            return "2.2";
        case 2:
            return "4.4";
        case 3:
            return "6.6";
        case 4:
            return "8.8";
        case 5:
            return "11";
        case 6:
            return "13.2";
        case 7:
            return "15.4";
        case 8:
            return "17.6";
        case 9:
            return "19.8";
        case 10:
            return "22";
        case 11:
            return "24";
    }
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
