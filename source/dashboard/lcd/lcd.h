#ifndef __LCD_H__
#define __LCD_H__
#include "common/phal_L4/spi/spi.h"
#include "nextion.h"
#include "pedals.h"
#include "can_parse.h"
#include "main.h"

//Page Strings / info
#define RACE_STRING "race"
#define SETTINGS_STRING "settings"
#define DATA_STRING "data"
#define TV_STRING "tv"
#define ERR_STRING "error"
#define WARN_STRING "warning"
#define FATAL_STRING "critical"
#define DEADBAND_STRING "deadband"
#define INTENSITY_STRING "intensity"
#define GEAR_RATIO ((49.0F * 111.0F / 27.0F / 26.0F) + 1U)

//Error/Knob page values
#define TIME_BAR "j0"
#define ERR_TXT "t1"
#define KNB_TXT "t3"

//TV Page Values
#define P_BAR "j0"
#define I_BAR "j1"
#define P_TXT "t3"
#define I_TXT "t5"
#define TV_DB_TXT "t7"
#define TV_IN_TXT "t9"

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
#define TV_P_FG 64512
#define TV_I_FG 31
#define TV_HOVER_FG_P 65106
#define TV_HOVER_FG_I 33823
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
  PAGE_PREFLIGHT,
  PAGE_RACE,
  PAGE_SETTINGS,
  PAGE_DATA,
  PAGE_TV,
  PAGE_WARNING,
  PAGE_ERROR,
  PAGE_FATAL,
  PAGE_KNOBS
} page_t;

typedef enum {
  NONE_SELECTED,
  P_SELECTED,
  I_SELECTED
} tv_select_t;

typedef struct {
  uint8_t yaw_p_val;
  uint8_t yaw_i_val;
  bool p_hover;
  tv_select_t p_selected;
  uint8_t intensity;
  uint8_t deadband;
  char *deadband_msg;
} tv_options_t;

typedef enum {
  DT_FAN_HOVER,
  DT_PUMP_HOVER,
  FAN1_HOVER,
  FAN2_HOVER,
  PUMP_HOVER,
  DT_FAN_SELECT,
  FAN1_SELECT
} hover_state_t;

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

void initLCD();
void updatePage();
void moveLeft();
void moveRight();
void moveUp();
void moveDown();
void selectItem();
void updateFaultDisplay();
void knobDisplay();
void send_p_val();
void send_i_val();
void update_data_pages();
char *get_deadband();
char *int_to_char(int16_t val, char *val_to_send);

#endif
