#ifndef __LCD_H__
#define __LCD_H__
#include "common/phal_F4_F7/spi/spi.h"
#include "nextion.h"
#include "pedals.h"
#include "can_parse.h"
#include "main.h"

//Page Strings / info
#define RACE_STRING "race"
#define SETTINGS_STRING "settings"
#define DATA_STRING "data"
#define ERR_STRING "error"
#define WARN_STRING "warning"
#define FATAL_STRING "critical"
#define FAULT_STRING "faults"
#define TVSETTINGS_STRING "tvsettings"
#define DRIVER_STRING "driver"
#define SDCINFO_STRING "sdcinfo"
#define LOGGING_STRING "logging"
#define GEAR_RATIO ((49.0F * 111.0F / 27.0F / 26.0F) + 1U)


// Driver Configuration Page
#define DRIVER_DEFAULT_OP "r0"
#define DRIVER_TYLER_OP "r1"
#define DRIVER_RUHAAN_OP "r2"
#define DRIVER_LUKE_OP "r3"
#define DRIVER_DEFAULT_TXT "t1"
#define DRIVER_TYLER_TXT "t2"
#define DRIVER_RUHAAN_TXT "t3"
#define DRIVER_LUKE_TXT "t4"

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
#define FAULT_1_TXT "t1"
#define FAULT_2_TXT "t2"
#define FAULT_3_TXT "t3"
#define FAULT_4_TXT "t4"
#define FAULT_5_TXT "t5"
#define FLT_STAT_1_TXT "t6"
#define FLT_STAT_2_TXT "t7"
#define FLT_STAT_3_TXT "t8"
#define FLT_STAT_4_TXT "t9"
#define FLT_STAT_5_TXT "t10"
#define FAULT_NONE_STRING "NONE\0"

//TV Settings Page Values
#define TV_INTENSITY_FLT "x0"
#define TV_PROPORTION_FLT "x1"
#define TV_DEAD_TXT "t6"
#define TV_ENABLE_OP "c0"

//Setings page values
#define DT_FAN_TXT "t2"
#define DT_PUMP_TXT "t3"
#define B_FAN1_TXT "t5"
#define B_FAN2_TXT "t6"
#define B_PUMP_TXT "t7"
#define DT_FAN_BAR "j0"
#define DT_FAN_VAL "t8"
#define DT_PUMP_OP "c1"
#define B_FAN1_BAR "j1"
#define B_FAN1_VAL "t9"
#define B_FAN2_OP "c3"
#define B_PUMP_OP "c4"

//Colors
#define TV_BG 38066
#define TV_HOVER_BG 52857
#define SETTINGS_HOVER_BG 28223
#define SETTINGS_BAR_BG 48631
#define SETTINGS_BAR_FG 495
#define SETTINGS_BG 1214
#define SETTINGS_FG BLACK
#define INFO_GRAY 48631
#define SETTINGS_UV_SELECT 31727
#define ORANGE 64512

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
#define GEAR_TEMP "t21"

#define BATT_TEMP "batt_temp"
#define BATT_VOLT "volts"
#define BATT_CURR "amps"
#define MOT_TEMP "motor_temp"
#define MC_TEMP "mc_temp"
#define CAR_STAT "status"
#define SPEED "speed"


typedef enum {

  // Pages selectable with the rot encoder
  // Should corresspond with the page count in main.h
  PAGE_RACE,
  PAGE_SETTINGS,
  PAGE_FAULTS,
  PAGE_TVSETTINGS,
  PAGE_SDCINFO,    
  PAGE_DRIVER,
  PAGE_LOGGING,
  PAGE_DATA,

  // Pages that can be displayed but not selected with the encoder
  PAGE_PREFLIGHT,
  PAGE_WARNING,
  PAGE_ERROR,
  PAGE_FATAL,
} page_t;

typedef enum {
  DT_FAN_HOVER,
  DT_PUMP_HOVER,
  FAN1_HOVER,
  FAN2_HOVER,
  PUMP_HOVER,
  DT_FAN_SELECT,
  FAN1_SELECT
} hover_state_t;

typedef enum {
  TV_INTENSITY_HOVER,
  TV_P_HOVER,
  TV_DEADBAND_HOVER,
  TV_ENABLE_HOVER,
  TV_INTENSITY_SELECTED,
  TV_P_SELECTED,
  TV_DEADBAND_SELECTED,
  TV_NONE_SELECTED,

} tv_hover_state_t;

typedef struct {
  bool tv_enable_selected;
  tv_hover_state_t curr_hover;
  uint8_t  tv_deadband_val;
  
  // intensity and p are 10x the float equivalent
  uint16_t tv_intensity_val;
  uint16_t tv_p_val;

} tv_settings_t;

typedef struct {
  uint16_t brake_bias_adj;  // 0 to 10000 for the page
} race_page_t;

typedef enum {
  DRIVER_DEFAULT_SELECT,
  DRIVER_TYLER_SELECT,
  DRIVER_RUHAAN_SELECT,
  DRIVER_LUKE_SELECT,

} driver_select_state_t;

typedef struct {
  driver_select_state_t curr_hover;
  driver_select_state_t curr_select;
} driver_config_t;

typedef struct {
  bool d_fan_selected;
  bool d_pump_selected;
  bool b_fan1_selected;
  bool b_fan2_selected;
  bool b_pump_selected;
  uint8_t d_fan_val;
  uint8_t b_fan_val;
  hover_state_t curr_hover;
} settings_t;

typedef struct {
  volatile int8_t encoder_position;

} lcd_t;

void initLCD();                                     // Initialize LCD data structures and configuration
void updatePage();                                  // Change the current page of the LCD
void moveUp();                                      // Upward UI input detected (up button or in some cases encoder)
void moveDown();                                    // Downward UI input detected (down button or in some cases encoder)
void selectItem();                                  // Selection UI input detected
void updateFaultDisplay();                          // Periodically poll recent faults and update the fault buffer and page as needed
void update_data_pages();                           // Periodically poll recent telemetry and update the data page as needd
void append_char(char *str, char ch, size_t max_len);
char *int_to_char(int16_t val, char *val_to_send);  // Convert integer value to character for the nextion interface
bool zeroEncoder(volatile int8_t* start_pos);       // Zero the encoder position for page selection
void sendTVParameters();                            // Periodically send updates to the TV configuration to TV board
void updateFaultPageIndicators();
void updateSDCDashboard();

#endif
