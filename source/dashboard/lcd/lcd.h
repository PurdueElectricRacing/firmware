#ifndef __LCD_H__
#define __LCD_H__

#include "nextion.h"
#include "menu_system.h"

// Page Strings (must match Nextion page names)
#define PREFLIGHT_STRING "preflight"
#define RACE_STRING "race"
#define COOLING_STRING "cooling"
#define APPS_STRING "calibration"
#define ERR_STRING "error"
#define WARN_STRING "warning"
#define FATAL_STRING "critical"
#define FAULT_STRING "faults"
#define TVSETTINGS_STRING "tvsettings"
#define DRIVER_STRING "driver"
#define DRIVER_CONFIG_STRING "profile"
#define SDCINFO_STRING "sdcinfo"
#define LOGGING_STRING "logging"

// Info
#define MPS_TO_MPH 2.2369362921f

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
#define PROFILE_CURRENT_TXT "curr"
#define PROFILE_BRAKE_FLT "brake"
#define PROFILE_THROTTLE_FLT "throt"
#define PROFILE_SAVE_BUTTON "save"
#define PROFILE_STATUS_TXT "stat"

//Error/Knob page values
#define TIME_BAR "j0" // todo
#define ERR_TXT "t1"

// SDC Info Page
#define SDC_IMD_STAT_TXT "t2"
#define SDC_BMS_STAT_TXT "t8"
#define SDC_BSPD_STAT_TXT "t14"
#define SDC_BOTS_STAT_TXT "t20"
#define SDC_INER_STAT_TXT "t26"
#define SDC_CSTP_STAT_TXT "t4"
#define SDC_MAIN_STAT_TXT "t10"
#define SDC_RSTP_STAT_TXT "t16"
#define SDC_LSTP_STAT_TXT "t22"
#define SDC_HVD_STAT_TXT "t6"
#define SDC_RHUB_STAT_TXT "t12"
#define SDC_TSMS_STAT_TXT "t18"
#define SDC_PCHG_STAT_TXT "t24"
#define SDC_FIRST_TRIP_TXT "t28"

// Fault Page
#define FAULT1_BUTTON "t1"
#define FAULT2_BUTTON "t2"
#define FAULT3_BUTTON "t3"
#define FAULT4_BUTTON "t4"
#define FAULT5_BUTTON "t5"
#define FAULT1_TXT "fault1"
#define FAULT2_TXT "fault2"
#define FAULT3_TXT "fault3"
#define FAULT4_TXT "fault4"
#define FAULT5_TXT "fault5"
#define CLEAR_BUTTON "clear"
#define FAULT_NONE_STRING "NONE\0"

// TV Settings Page Values
#define TV_INTENSITY_FLT "inten"
#define TV_PROPORTION_FLT "pval"
#define TV_DEAD_TXT "dead"
#define TV_ENABLE_OP "tv"
#define TV_CAN_STATUS "can"

// Cooling Page
#define DT_FAN_VAL "DFan"
#define DT_FAN_BAR "DBar"
#define DT_PUMP_OP "DPump"
#define B_FAN_VAL "BFan"
#define B_FAN_BAR "BBar"
#define B_PUMP_OP "BPump"
#define BAR_INTERVAL 25
#define COOLING_CAN_STATUS "can"

// Race Page
#define THROT_BAR "throt"
#define BRK_BAR "brake"
#define BATT_TEMP "BTemp"
#define BATT_VOLT "volt"
#define BATT_CURR "amp"
#define MOT_TEMP "MTemp"
#define MC_TEMP "MCTemp"
#define CAR_STAT "stat"
#define SPEED "speed"
#define RACE_TV_ON "tv"

// Logging Page
#define LOG_OP "log"
#define LOGGING_STATUS_TXT "stat"

// Apps Page
#define CALIBRATION_BRAKE1_VAL "B1"
#define CALIBRATION_BRAKE2_VAL "B2"
#define CALIBRATION_THROTTLE1_VAL "T1"
#define CALIBRATION_THROTTLE2_VAL "T2"
#define CALIBRATION_BRAKE_DEV_VAL "BDev"
#define CALIBRATION_THROTTLE_DEV_VAL "TDev"
#define CALIBRATION_BRAKE_BAR "brake"
#define CALIBRATION_THROTTLE_BAR "throt"
#define CALIBRATION_BRAKE_STAT "stat"
#define CALIBRATION_BRAKE_FAIL "fail"
#define CALIBRATION_BRAKE1_THRESHOLD "B1T"
#define CALIBRATION_BRAKE2_THRESHOLD "B2T"

#define PAGE_COUNT 12

typedef enum {
  // Pages selectable with the rot encoder
  // Should corresspond with the page count in main.h
  PAGE_RACE        = 0,
  PAGE_COOLING     = 1,
  PAGE_TVSETTINGS  = 2,
  PAGE_FAULTS      = 3,
  PAGE_SDCINFO     = 4,
  PAGE_DRIVER      = 5,
  PAGE_PROFILES    = 6,
  PAGE_LOGGING     = 7,
  PAGE_CALIBRATION = 8,

  // Pages that can be displayed but not selected with the encoder
  PAGE_PREFLIGHT   = 9,
  PAGE_WARNING     = 10,
  PAGE_ERROR       = 11,
  PAGE_FATAL       = 12,
} page_t;

typedef struct {
  void (*update)(void);
  void (*move_up)(void);
  void (*move_down)(void);
  void (*select)(void);
  void (*telemetry)(void);
} page_handler_t;

void initLCD();                     // Initialize LCD data structures and configuration
void updatePage();                  // Change the current page of the LCD
void moveUp();                      // Upward UI input detected (up button or in some cases encoder)
void moveDown();                    // Downward UI input detected (down button or in some cases encoder)
void selectItem();                  // Selection UI input detected
void updateFaultDisplay();          // Periodically poll recent faults and update the fault buffer and page as needed
void updateTelemetryPages();        // Periodically poll recent telemetry and update the race/apps page as needd
void sendTVParameters();            // Periodically send updates to the TV configuration to TV board
void sendCoolingParameters();       // Periodically send updates to the cooling configuration to the cooling board
void sendLoggingParameters();       // Periodically send updates to the logging configuration to the daq board
void updateFaultPageIndicators();   // Update the fault page indicators

#endif // __LCD_H__
