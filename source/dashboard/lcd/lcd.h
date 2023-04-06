#ifndef __LCD_H__
#define __LCD_H__
#include "common/phal_L4/spi/spi.h"
#include "nextion.h"
#include "pedals.h"
#include "can_parse.h"
#include "main.h"

//Page Strings
#define RACE_STRING "race"
#define SETTINGS_STRING "settings"
#define DATA_STRING "data"
#define TV_STRING "tv"
#define ERR_STRING "error"
#define WARN_STRING "warning"
#define FATAL_STRING "critical"
#define DEADBAND_STRING "deadband"
#define INTENSITY_STRING "intensity"

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
#define DT_FAN_OP "c0"
#define DT_PUMP_OP "c1"
#define B_FAN1_OP "c2"
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
#define SETTINGS_BG 1214
#define SETTINGS_FG BLACK

typedef enum {
  PAGE_PREFLIGHT,
  PAGE_RACE,
  PAGE_SETTINGS,
  PAGE_DATA,
  PAGE_TV,
  PAGE_ERROR,
  PAGE_WARNING,
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
  PUMP_HOVER
} hover_state_t;

typedef struct {
  bool d_fan_selected;
  bool d_pump_selected;
  bool b_fan1_selected;
  bool b_fan2_selected;
  bool b_pump_selected;
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
char *get_deadband();

#endif
