#include "lcd.h"
#include "hdd.h"
#include "common/psched/psched.h"
#include "pedals.h"
#include "common/faults/faults.h"
#include "common_defs.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

volatile page_t curr_page;
volatile page_t prev_page;

uint8_t display_time;

char *errorText;

static uint8_t preflight;

extern hdd_value_t hdd;

volatile tv_options_t tv;

volatile settings_t settings;

//1 = deadband, 0 = intensity
bool knob;
bool knob_old;

bool midAdjustment;

void initLCD() {
    curr_page = PAGE_RACE;
    prev_page = PAGE_PREFLIGHT;
    display_time = 0;
    errorText = 0;
    knob = false;
    knob_old = false;
    midAdjustment = 0;
    tv = (tv_options_t) {0, 0, 0, 0, 0, 0, "\0"};
    settings = (settings_t) {0, 0, 0, 0, 0, 0};

}

void updatePage() {
    if (curr_page == prev_page) {
        return;
    }
    switch (curr_page) {
        case PAGE_RACE:
            prev_page = PAGE_RACE;
            set_page(RACE_STRING);
            break;
        case PAGE_DATA:
            prev_page = PAGE_DATA;
            set_page(DATA_STRING);
            break;
        case PAGE_SETTINGS:
            prev_page = PAGE_SETTINGS;
            set_page(SETTINGS_STRING);
            settings.curr_hover = DT_FAN_HOVER;
            set_value(DT_FAN_TXT, NXT_BACKGROUND_COLOR, SETTINGS_HOVER_BG);
            set_value(DT_FAN_OP, NXT_VALUE, settings.d_fan_selected);
            set_value(DT_PUMP_OP, NXT_VALUE, settings.d_pump_selected);
            set_value(B_FAN1_OP, NXT_VALUE, settings.b_fan1_selected);
            set_value(B_FAN2_OP, NXT_VALUE, settings.b_fan2_selected);
            set_value(B_PUMP_OP, NXT_VALUE, settings.b_pump_selected);
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
        case PAGE_KNOBS:
            switch (display_time++) {
                case 0:
                    if (knob) {
                        set_page(DEADBAND_STRING);
                        tv.deadband_msg = get_deadband();
                        set_text(KNB_TXT, NXT_TEXT, tv.deadband_msg);
                    }
                    else {
                        set_page(INTENSITY_STRING);
                        char value_to_send[2] = "\0";
                        if (hdd.intensity_pos > 9) {
                            value_to_send[0] = (hdd.intensity_pos / 10) + 48;
                            value_to_send[1] = (hdd.intensity_pos % 10) + 48;
                        }
                        else {
                            value_to_send[0] = hdd.intensity_pos + 48;
                        }
                        set_text(KNB_TXT, NXT_TEXT, value_to_send);
                    }
                    break;
                case 5:
                    set_value(TIME_BAR, NXT_VALUE, 100);
                    display_time = 0;
                    curr_page = prev_page;
                    prev_page = PAGE_PREFLIGHT;
                    break;
                default:
                    set_value(TIME_BAR, NXT_VALUE, (display_time * 20));
                    break;
            }
            break;
        default:
            switch(display_time++) {
                case 0:
                    switch(curr_page) {
                        case PAGE_ERROR:
                            set_page(ERR_STRING);
                            break;
                        case PAGE_WARNING:
                            set_page(WARN_STRING);
                            break;
                        case PAGE_FATAL:
                            set_page(FATAL_STRING);
                            break;
                    }
                    set_value(TIME_BAR, NXT_VALUE, 0);
                    set_text(ERR_TXT, NXT_TEXT, errorText);
                    break;
                case 10:
                    set_value(TIME_BAR, NXT_VALUE, 100);
                    curr_page = prev_page;
                    display_time = 0;
                    prev_page = PAGE_PREFLIGHT;
                    break;
                default:
                    set_value(TIME_BAR, NXT_VALUE, (display_time * 10));
                    break;
            }
    }
}

void moveLeft() {
    switch(curr_page) {
        case PAGE_RACE:
            curr_page = PAGE_TV;
            updatePage();
            break;
        case PAGE_SETTINGS:
            curr_page = PAGE_DATA;
            updatePage();
            break;
        case PAGE_DATA:
            curr_page = PAGE_RACE;
            updatePage();
            break;
        case PAGE_TV:
            if (tv.p_selected == NONE_SELECTED) {
                curr_page = PAGE_SETTINGS;
                updatePage();
            }
            break;
        default:
            curr_page = prev_page;
            prev_page = PAGE_PREFLIGHT;
            display_time = 0;
            updatePage();
            break;
    }
}

void moveRight() {
    switch(curr_page) {
        case PAGE_RACE:
            curr_page = PAGE_DATA;
            updatePage();
            break;
        case PAGE_SETTINGS:
            curr_page = PAGE_TV;
            updatePage();
            break;
        case PAGE_DATA:
            curr_page = PAGE_SETTINGS;
            updatePage();
            break;
        case PAGE_TV:
            if (tv.p_selected == NONE_SELECTED) {
                curr_page = PAGE_RACE;
                updatePage();
            }
            break;
        default:
            curr_page = prev_page;
            prev_page = PAGE_PREFLIGHT;
            display_time = 0;
            updatePage();
            break;
    }
}

void moveUp() {
    if (((curr_page != PAGE_SETTINGS) || (curr_page != PAGE_TV))&& curr_page > PAGE_TV) {
        curr_page = prev_page;
        prev_page = PAGE_PREFLIGHT;
        display_time = 0;
        updatePage();
        return;
    }
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
    else {
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
        }
    }
}

void moveDown() {
    if (((curr_page != PAGE_SETTINGS) || (curr_page != PAGE_TV))&& curr_page > PAGE_TV) {
        curr_page = prev_page;
        prev_page = PAGE_PREFLIGHT;
        display_time = 0;
        updatePage();
        return;
    }
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
    else {
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
        }
    }
}

void selectItem() {
    if (((curr_page != PAGE_SETTINGS) || (curr_page != PAGE_TV))&& curr_page > PAGE_TV) {
        curr_page = prev_page;
        prev_page = PAGE_PREFLIGHT;
        display_time = 0;
        updatePage();
        return;
    }
    if (curr_page == PAGE_TV) {
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
    else {
        switch (settings.curr_hover) {
            case DT_FAN_HOVER:
                settings.d_fan_selected = !settings.d_fan_selected;
                set_value(DT_FAN_OP, NXT_VALUE, settings.d_fan_selected);
                break;
            case DT_PUMP_HOVER:
                settings.d_pump_selected = !settings.d_pump_selected;
                set_value(DT_PUMP_OP, NXT_VALUE, settings.d_pump_selected);
                break;
            case FAN1_HOVER:
                settings.b_fan1_selected = !settings.b_fan1_selected;
                set_value(B_FAN1_OP, NXT_VALUE, settings.b_fan1_selected);
                break;
            case FAN2_HOVER:
                settings.b_fan2_selected = !settings.b_fan2_selected;
                set_value(B_FAN2_OP, NXT_VALUE, settings.b_fan2_selected);
                break;
            case PUMP_HOVER:
                settings.b_pump_selected = !settings.b_pump_selected;
                set_value(B_PUMP_OP, NXT_VALUE, settings.b_pump_selected);
                break;
        }
        // SEND_
    }
}

void updateFaultDisplay() {
    if (most_recent_latched == 0xFFFF)
    {
        return;
    }
    display_time = 0;
    curr_page = faultArray[most_recent_latched].priority + 5;
    errorText = faultArray[most_recent_latched].screen_MSG;
    most_recent_latched = 0xFFFF;
    updatePage();
}

//1 = deadband, 0 = intensity
void knobDisplay() {
    if (preflight < 2) {
        preflight++;
        return;
    }
    if (curr_page == PAGE_KNOBS) {
        if (knob_old == knob && knob) {
            display_time = 1;
            tv.deadband_msg = get_deadband();
            set_text(KNB_TXT, NXT_TEXT, tv.deadband_msg);
            return;
        }
        else if (knob_old == knob && !knob) {
            display_time = 1;
            tv.intensity = hdd.intensity_pos;
            char value_to_send[2] = "\0";
            if (hdd.intensity_pos > 9) {
                value_to_send[0] = (hdd.intensity_pos / 10) + 48;
                value_to_send[1] = (hdd.intensity_pos % 10) + 48;
            }
            else {
                value_to_send[0] = hdd.intensity_pos + 48;
            }
            set_text(KNB_TXT, NXT_TEXT, value_to_send);
            knob_old = knob;
            return;
        }
    }
    curr_page = PAGE_KNOBS;
    display_time = 0;
    knob_old = knob;
    updatePage();
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