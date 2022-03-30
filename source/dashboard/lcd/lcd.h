#ifndef __LCD_H__
#define __LCD_H__

#include "common/phal_L4/spi/spi.h"
#include "nextion.h"
#include "pedals.h"
#include "can_parse.h"
#include "main.h"

#define BTN_SELECT_TIMEOUT_MS 5000

typedef enum
{
    P_SPLASH,
    P_MAIN,
    P_INFO,
    P_TOTAL
} pages_t;

typedef enum
{
    B_M_BUTTON,
    B_MAIN_TOTAL
} main_buttons_t;

#define B_M_BUTTON_NORM_PIC 12
#define B_M_BUTTON_HIGH_PIC 13

typedef enum
{
    A_SPEED,
    A_VOLTAGE,
    A_BATTERY,
    A_POWER,
    A_TV_STATUS,
    A_MAIN_TOTAL
} main_attributes_t;

#define POWER_OFF_PIC 11
#define POWER_ON_PIC 10
#define TV_STAT_OFF_PIC 16
#define TV_STAT_ON_PIC  17

typedef enum
{
    B_B_BUTTON,
    B_INFO_TOTAL
} info_buttons_t;

#define B_BUTTON_NORM_PIC 19
#define B_BUTTON_HIGH_PIC 18

void joystickUpdatePeriodic();
void changePage(uint8_t new_page);
void valueUpdatePeriodic();

#endif