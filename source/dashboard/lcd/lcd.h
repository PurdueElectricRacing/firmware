#ifndef __LCD_H__
#define __LCD_H__

#include "menu_system.h"
#include "nextion.h"

// Page Strings (must match Nextion page names)
#define PREFLIGHT_STRING     "preflight"
#define RACE_STRING          "race"
// #define COOLING_STRING       "cooling"
#define APPS_STRING          "calibration"
// #define ERR_STRING           "error"
// #define WARN_STRING          "warning"
// #define FATAL_STRING         "critical"
#define FAULT_STRING         "fault"
// #define TVSETTINGS_STRING    "tvsettings"
// #define DRIVER_STRING        "driver"
// #define DRIVER_CONFIG_STRING "profile"
// #define SDCINFO_STRING       "sdcinfo"
// #define LOGGING_STRING       "logging"

// Info
#define MPS_TO_MPH   (2.237F)
#define RPM_TO_MPH   (0.00595F)
#define WHEEL_RADIUS (8) // inches

// Driver Configuration Page
#define DRIVER1_LIST "t1"
#define DRIVER2_LIST "t2"
#define DRIVER3_LIST "t3"
#define DRIVER4_LIST "t4"
#define DRIVER1_NAME "Default"
#define DRIVER2_NAME "Tyler"
#define DRIVER3_NAME "Luca"
#define DRIVER4_NAME "Luke"

// Profile Page
#define PROFILE_CURRENT_TXT  "curr"
#define PROFILE_BRAKE_FLT    "brake"
#define PROFILE_THROTTLE_FLT "throt"
#define PROFILE_SAVE_BUTTON  "save"
#define PROFILE_STATUS_TXT   "stat"

//Error/Knob page values
#define TIME_BAR "j0" // todo
#define ERR_TXT  "t1"

// SDC Info Page
#define SDC_IMD_STAT_TXT   "t2"
#define SDC_BMS_STAT_TXT   "t8"
#define SDC_BSPD_STAT_TXT  "t14"
#define SDC_BOTS_STAT_TXT  "t20"
#define SDC_INER_STAT_TXT  "t26"
#define SDC_CSTP_STAT_TXT  "t4"
#define SDC_MAIN_STAT_TXT  "t10"
#define SDC_RSTP_STAT_TXT  "t16"
#define SDC_LSTP_STAT_TXT  "t22"
#define SDC_HVD_STAT_TXT   "t6"
#define SDC_RHUB_STAT_TXT  "t12"
#define SDC_TSMS_STAT_TXT  "t18"
#define SDC_PCHG_STAT_TXT  "t24"
#define SDC_FIRST_TRIP_TXT "t28"

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
#define CLEAR_BUTTON      "clear_stale"
#define FAULT_NONE_STRING "NONE\0"

// TV Settings Page Values
#define TV_PERMIT_MODE_TXT  "permit"
#define TV_CONTROL_MODE_TXT "control"
#define TV_DEADBAND_TXT     "dead"
#define TV_P_GAIN_FLT       "pgain"
#define TV_TORQUE_DROP_FLT  "tdrop"
#define TV_MAX_SLIP_FLT     "slip"
#define TV_CAN_STATUS       "can"
// Default to Tyler's settings
#define TV_DEADBAND_DEFAULT_VALUE    5U // 5
#define TV_P_GAIN_DEFAULT_VALUE      250U // 2.50
#define TV_TORQUE_DROP_DEFAULT_VALUE 100U // 1.00
#define TV_SLIP_DEFAULT_VALUE        100U // 1.00

// Cooling Page
#define DT_FAN_VAL         "DFan"
#define DT_FAN_BAR         "DBar"
#define DT_PUMP_OP         "DPump"
#define B_FAN_VAL          "BFan"
#define B_FAN_BAR          "BBar"
#define B_PUMP_OP          "BPump"
#define BAR_INTERVAL       (25)
#define COOLING_CAN_STATUS "can"

// Race Page
// #define THROT_BAR          "throt"
// #define BRK_BAR            "brake"
// #define BATT_TEMP          "BTemp"
// #define BATT_VOLT          "volt"
// #define BATT_CURR          "amp"
// #define MOT_TEMP           "MTemp"
// #define MC_TEMP            "IGBTemp" // IGBT Temp now
// #define CAR_STAT           "stat"
// #define SPEED              "speed"
// #define RACE_TV_ON         "tv"
// #define AMK_MOTOR_OVERLOAD "amk"
#define THROT_BAR          "throt"
#define BRK_BAR            "brake"
#define RGN_BAR            "regen"
#define FL_BAR             "FL"
#define FR_BAR             "FR"
#define RL_BAR             "RL"
#define RR_BAR             "RR"
#define PUMP_CHECK         "pump"
#define BATT_TEMP          "BTemp"
#define BATT_VOLT          "volt"
#define BATT_CURR          "amp"
#define MOT_TEMP           "MTemp"
#define MC_TEMP            "IGBTemp" // IGBT Temp now
#define CAR_STAT           "stat"
#define SPEED              "speed"
#define RACE_TV_ON         "tv"
// #define AMK_MOTOR_OVERLOAD "amk"
#define CURRENT_TIME       "current"
#define BEST_TIME          "best"
#define LAP_TIME           "lap"

// Logging Page
#define LOG_OP             "log"
#define LOGGING_STATUS_TXT "stat"

// Calibration Page
// #define CALIBRATION_BRAKE1_VAL       "B1"
// #define CALIBRATION_BRAKE2_VAL       "B2"
// #define CALIBRATION_THROTTLE1_VAL    "T1"
// #define CALIBRATION_THROTTLE2_VAL    "T2"
// #define CALIBRATION_BRAKE_DEV_VAL    "BDev"
// #define CALIBRATION_THROTTLE_DEV_VAL "TDev"
// #define CALIBRATION_BRAKE_BAR        "brake"
// #define CALIBRATION_THROTTLE_BAR     "throt"
// #define CALIBRATION_BRAKE_STAT       "stat"
// #define CALIBRATION_BRAKE_FAIL       "fail"
// #define CALIBRATION_BRAKE1_THRESHOLD "B1T"
// #define CALIBRATION_BRAKE2_THRESHOLD "B2T"
#define CALIBRATION_BRAKE1_VAL       "brk1"
#define CALIBRATION_BRAKE2_VAL       "brk2"
#define CALIBRATION_THROTTLE1_VAL    "thr1"
#define CALIBRATION_THROTTLE2_VAL    "thr2"
#define CALIBRATION_BRAKE_PRS1_VAL   "brkprs1"
#define CALIBRATION_BRAKE_PRS2_VAL   "brkprs2"

typedef enum {
    // Should corresspond with the page count in main.h
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
