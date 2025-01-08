#ifndef __LCD_H__
#define __LCD_H__

#include "nextion.h"
#include <stdint.h>
#include "menu_system.h"

//Page Strings / info
#define RACE_STRING "race"
#define COOLING_STRING "cooling"
#define APPS_STRING "apps"
#define ERR_STRING "error"
#define WARN_STRING "warning"
#define FATAL_STRING "critical"
#define FAULT_STRING "faults"
#define TVSETTINGS_STRING "tvsettings"
#define DRIVER_STRING "driver"
#define DRIVER_CONFIG_STRING "profile"
#define SDCINFO_STRING "sdcinfo"
#define LOGGING_STRING "logging"
#define GEAR_RATIO ((49.0F * 111.0F / 27.0F / 26.0F) + 1U) // TODO remove?


// Driver Configuration Page
#define DRIVER1_TXT "t1"
#define DRIVER2_TXT "t2"
#define DRIVER3_TXT "t3"
#define DRIVER4_TXT "t4"
#define DRIVER1_NAME "Default"
#define DRIVER2_NAME "Tyler"
#define DRIVER3_NAME "Luca"
#define DRIVER4_NAME "Luke"

// Profile Page
#define PROFILE_CURRENT_TXT "cur_driver"
#define PROFILE_BRAKE_FLT "brk_val"
#define PROFILE_THROTTLE_FLT "thrt_val"
#define PROFILE_SAVE_TXT "save"
#define PROFILE_STATUS_TXT "status"


//Error/Knob page values
#define TIME_BAR "j0"
#define ERR_TXT "t1"

// SDC Info Page Values
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


//Fault Page Values
#define FAULT_1_TXT "fault1"
#define FAULT_2_TXT "fault2"
#define FAULT_3_TXT "fault3"
#define FAULT_4_TXT "fault4"
#define FAULT_5_TXT "fault5"
#define CLEAR_FAULTS_TXT "clear"
#define FAULT_NONE_STRING "NONE\0"

//TV Settings Page Values
#define TV_INTENSITY_FLT "int_flt"
#define TV_PROPORTION_FLT "p_flt"
#define TV_DEAD_TXT "dead_val"
#define TV_ENABLE_OP "tv_op"

//Setings page values
#define DT_FAN_VAL "DFan_val"
#define DT_FAN_BAR "DFan_bar"
#define DT_PUMP_OP "DPump_op"
#define B_FAN_VAL "BFan_val"
#define B_FAN_BAR "BFan_bar"
#define B_PUMP_OP "BPump_op"
#define BAR_INTERVAL 25

//Colors
#define TV_BG 38066
#define TV_HOVER_BG 52857
#define INFO_GRAY 48631

//Race/Data pages
#define POW_LIM_BAR "j0"
#define BRAKE_BIAS_FLT "x0"
#define TV_RL_FLT "x1"
#define TV_RR_FLT "x2"

#define THROT_BAR "throt_bar"
#define BRK_BAR "brk_bar"
#define TV_ON "tv"
#define FLT_TO_DISPLAY_INT_2_DEC (100U)
#define FLT_TO_PERCENTAGE (100U)

//Race specific Values
#define BATT_TEMP "batt_temp"
#define BATT_VOLT "volts"
#define BATT_CURR "amps"
#define MOT_TEMP "motor_temp"
#define MC_TEMP "mc_temp"
#define CAR_STAT "status"
#define SPEED "speed"
#define RACE_TV_ON "tv_op"

typedef enum {

  // Pages selectable with the rot encoder
  // Should corresspond with the page count in main.h
  PAGE_RACE,
  PAGE_COOLING,
  PAGE_TVSETTINGS,
  PAGE_FAULTS,
  PAGE_SDCINFO,
  PAGE_DRIVER,
  PAGE_PROFILES,
  PAGE_LOGGING,
  PAGE_APPS,

  // Pages that can be displayed but not selected with the encoder
  PAGE_PREFLIGHT,
  PAGE_WARNING,
  PAGE_ERROR,
  PAGE_FATAL,
} page_t;

typedef struct {
  uint16_t brake_bias_adj;  // 0 to 10000 for the page
} race_page_t;

typedef enum {
  FAULT1,
  FAULT2,
  FAULT3,
  FAULT4,
  FAULT5,
  CLEAR
} fault_hover_state_t;
typedef struct {
  fault_hover_state_t curr_hover;
} fault_page_t;

typedef struct {
  volatile int8_t encoder_position;
} lcd_t;

typedef struct {
  void (*update)(void);
  void (*move_up)(void);
  void (*move_down)(void);
  void (*select)(void);
} page_handler_t;

void initLCD();                     // Initialize LCD data structures and configuration
void updatePage();                  // Change the current page of the LCD
void moveUp();                      // Upward UI input detected (up button or in some cases encoder)
void moveDown();                    // Downward UI input detected (down button or in some cases encoder)
void selectItem();                  // Selection UI input detected
void updateFaultDisplay();          // Periodically poll recent faults and update the fault buffer and page as needed
void updateTelemetryPages();        // Periodically poll recent telemetry and update the race/apps page as needd
void sendTVParameters();            // Periodically send updates to the TV configuration to TV board
void updateFaultPageIndicators();   // Update the fault page indicators
void updateSDCDashboard();          // Update the SDC info page

extern menu_page_t tv_page;

#endif // __LCD_H__
