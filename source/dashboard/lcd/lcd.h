#ifndef __LCD_H__
#define __LCD_H__

#include "menu_system.h"
#include "nextion.h"

// Page Strings (must match Nextion page names)
#define PREFLIGHT_STRING   "preflight"
#define RACE_STRING        "race"
#define CALIBRATION_STRING "calibration"
#define FAULT_STRING       "fault"

// Info
constexpr float WHEEL_RADIUS_IN = 8.0f;
constexpr float GEAR_RATIO = 12.51f;
constexpr float PI = 3.14159f;

constexpr float WHEEL_CIRCUMFERENCE_IN = 2.0f * PI * WHEEL_RADIUS_IN;
constexpr float OUTPUT_REV_PER_MOTOR_REV = 1.0f / GEAR_RATIO;
constexpr float INCHES_PER_MOTOR_REV = WHEEL_CIRCUMFERENCE_IN * OUTPUT_REV_PER_MOTOR_REV;

constexpr float MINUTES_PER_HOUR = 60.0f;
constexpr float INCHES_PER_MILE = 63360.0f;

constexpr float RPM_TO_MPH = INCHES_PER_MOTOR_REV * MINUTES_PER_HOUR / INCHES_PER_MILE;

// Fault Page
#define FAULT1_BUTTON     "ERROR1"
#define FAULT2_BUTTON     "ERROR2"
#define FAULT3_BUTTON     "ERROR3"
#define FAULT4_BUTTON     "ERROR4"
#define FAULT5_BUTTON     "ERROR5"
#define FAULT6_BUTTON     "ERROR6"
#define FAULT7_BUTTON     "ERROR7"
#define FAULT8_BUTTON     "ERROR8"
#define FAULT1_TXT        "ERROR1"
#define FAULT2_TXT        "ERROR2"
#define FAULT3_TXT        "ERROR3"
#define FAULT4_TXT        "ERROR4"
#define FAULT5_TXT        "ERROR5"
#define FAULT6_TXT        "ERROR6"
#define FAULT7_TXT        "ERROR7"
#define FAULT8_TXT        "ERROR8"
#define FAULT_NONE_STRING "NONE\0"

// Race Page
#define THROT_BAR    "throt"
#define BRK_BAR      "brake"
#define RGN_BAR      "regen"
#define FL_BAR       "FL"
#define FR_BAR       "FR"
#define RL_BAR       "RL"
#define RR_BAR       "RR"
#define PUMP_CHECK   "pump"
#define BATT_TEMP    "BTemp"
#define BATT_VOLT    "volt"
#define BATT_CURR    "amp"
#define MOT_TEMP     "MTemp"
#define MC_TEMP      "IGBTemp" // IGBT Temp now
#define CAR_STAT     "stat"
#define SPEED        "speed"
#define RACE_TV_ON   "tv"
#define CURRENT_TIME "current"
#define BEST_TIME    "best"
#define LAP_TIME     "lap"

// Logging Page
#define LOG_OP             "log"
#define LOGGING_STATUS_TXT "stat"

// Calibration Page
#define CALIBRATION_BRAKE1_VAL       "brk1"
#define CALIBRATION_BRAKE2_VAL       "brk2"
#define CALIBRATION_THROTTLE1_VAL    "thr1"
#define CALIBRATION_THROTTLE2_VAL    "thr2"
#define CALIBRATION_BRAKE_PRS1_VAL   "brkprs1"
#define CALIBRATION_BRAKE_PRS2_VAL   "brkprs2"

typedef enum {
    // Should correspond with the page count in main.h
    PAGE_PREFLIGHT    = 0,
    PAGE_RACE        = 1,
    // PAGE_SDC_INFO    = 2,
    PAGE_CALIBRATION = 2,
    PAGE_FAULTS      = 3,
    NUM_PAGES       = 4,
} page_t;

typedef struct {
    void (*update)(void);
    void (*move_up)(void);
    void (*move_down)(void);
    void (*select)(void);
    void (*telemetry)(void);
} page_handler_t;

void initLCD(); // Initialize LCD data structures and configuration
void updatePage(); // Change the current page of the LCD
void advancePage(); // Advance to the next selectable page
void backPage(); // Move to the previous selectable page
void moveUp(); // Upward UI input detected (up button or in some cases encoder)
void moveDown(); // Downward UI input detected (down button or in some cases encoder)
void selectItem(); // Selection UI input detected
void updateFaultDisplay(); // Periodically poll recent faults and update the fault buffer and page as needed
void updateTelemetryPages(); // Periodically poll recent telemetry and update the page as needd
void sendTVParameters(); // Periodically send updates to the TV configuration to TV board
void sendCoolingParameters(); // Periodically send updates to the cooling configuration to the cooling board
void sendLoggingParameters(); // Periodically send updates to the logging configuration to the daq board

#endif // __LCD_H__
