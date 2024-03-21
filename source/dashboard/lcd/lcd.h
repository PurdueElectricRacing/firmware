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
#define GEAR_RATIO ((49.0F * 111.0F / 27.0F / 26.0F) + 1U)

//Error/Knob page values
#define TIME_BAR "j0"
#define ERR_TXT "t1"

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
#define THROT_BAR "j1"
#define POW_LIM_BAR "j0"

//Race specific Values
#define TV_FL "t2"
#define TV_FR "t3"
#define TV_LR "t5"
#define TV_RR "t4"
#define BATT_TEMP "t8"
#define BATT_VOLT "t10"
#define BATT_CURR "t13"
#define MOT_TEMP "t20"
#define GEAR_TEMP "t21"
#define CAR_STAT "t22"
#define SPEED "t0"


typedef enum {

  // Pages selectable with the rot encoder
  // Should corresspond with the page count in main.h
  PAGE_RACE,
  PAGE_SETTINGS,
  PAGE_DATA,
  PAGE_FAULTS,
  PAGE_TVSETTINGS,    

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
char *int_to_char(int16_t val, char *val_to_send);  // Convert integer value to character for the nextion interface
bool zeroEncoder(volatile int8_t* start_pos);       // Zero the encoder position for page selection
void sendTVParameters();                            // Periodically send updates to the TV configuration to TV board
void updateFaultPageIndicators();         

#endif
