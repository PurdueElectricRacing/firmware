#ifndef __LCD_H__
#define __LCD_H__

#include "nextion.h"
#include "can_parse.h"

#define BTN_NORM_ID 4
#define BTN_HIGH_ID BTN_NORM_ID

typedef enum
{
    P_SPLASH,
    P_MAIN,
    P_SETTINGS,
    P_INFO,
    P_TOTAL
} pages_t;

typedef enum
{
    B_TC_BUTTON,
    B_DIAG_BUTTON,
    B_LAPS_BUTTON,
    B_START_BUTTON,
    B_SETTINGS,
    B_MAIN_TOTAL
} main_buttons_t;

typedef enum
{
    A_SPEED,
    A_VOLTAGE,
    A_BATTERY_BAR,
    A_STATUS_LABEL,
    A_MAIN_TOTAL
} main_attributes_t;

void joystick_update();
void action_update();
void value_update();
void change_page(uint8_t new_page);

#endif