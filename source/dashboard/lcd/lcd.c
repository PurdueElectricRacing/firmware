#include "lcd.h"
#include "common/psched/psched.h"
#include "pedals.h"
#include <string.h>
#include <stdbool.h>
extern SPI_InitConfig_t hspi1;
button_t lap_time[] = {
  {.name="laptime", .norm_id=B_M_BUTTON_NORM_PIC, .high_id=B_M_BUTTON_HIGH_PIC,
   .dirs={B_M_BUTTON, B_M_BUTTON, B_M_BUTTON, B_M_BUTTON}},
};
button_t info_buttons[] = {
  {.name="bButton", .norm_id=B_BUTTON_NORM_PIC, .high_id=B_BUTTON_HIGH_PIC,
   .dirs={B_B_BUTTON, B_B_BUTTON, B_B_BUTTON, B_B_BUTTON}},
};
page_t pages[P_TOTAL] = {
  {.name="startup"},
  {.name="race", .buttons=lap_time, .num_buttons=B_MAIN_TOTAL},
  {.name="extra_info", .buttons=lap_time, .num_buttons=B_MAIN_TOTAL},
  {.name="info"},
  {.name="warning"},
  {.name="critical"}
};
uint8_t p_idx_prev = P_RACE;
uint8_t p_idx = P_RACE;
uint8_t p_idx_perr = P_RACE;
uint32_t car_stat_color = GREEN;
char *err_msg = "CAR_OK\0";
char *prev_err_msg = "CAR_OK\0";
uint8_t b_idx = 0;
uint32_t b_change_time = 0;
uint32_t b_selected_time = 0;
uint32_t avg_soc = 0;
uint16_t gas_to_send = 0;
bool precharge_complete = false;
int prev_time = 0;
int curr_lap_time = 0;
int delta = 0;
int delta_old = 0;
uint32_t orig_time = 0;
/* Joystick Management */
typedef enum {
  J_UP,
  J_RIGHT,
  J_DOWN,
  J_LEFT,
  J_CENTER
} joystick_dir_t;

typedef enum {
   B_HIGH,
   B_LOW
} button_state_t;
joystick_dir_t j_dir = J_CENTER;
uint8_t btn_state = 0; // activate only on falling edge

button_state_t b_state = B_LOW;
uint8_t wheel_read_cmd[] = {WHEEL_SPI_ADDR | WHEEL_SPI_READ, WHEEL_GPIO_REG, 0x00};
typedef union __attribute__((packed))
{
   struct {
       uint8_t aux1  :1;
       uint8_t up    :1;
       uint8_t down  :1;
       uint8_t left  :1;
       uint8_t right :1;
       uint8_t aux2  :1;
       uint8_t aux3  :1;
   };
   uint8_t raw_data;
} WheelBtns_t;
WheelBtns_t wheel_new_btns = {0};
WheelBtns_t wheel_old_btns = {0};
/* Function Prototypes */
static void actionUpdatePeriodic();
void joystickUpdatePeriodic()
{
  // Manage button selection with joystick
  // Update the joystick position and button state
  // Joystick direction only changes on rising edge
  j_dir = J_CENTER;
  if      (wheel_new_btns.up    && !wheel_old_btns.up)    j_dir = J_UP;
  else if (wheel_new_btns.down  && !wheel_old_btns.down)  j_dir = J_DOWN;
  else if (wheel_new_btns.right && !wheel_old_btns.right) j_dir = J_RIGHT;
  else if (wheel_new_btns.left  && !wheel_old_btns.left)  j_dir = J_LEFT;
  // Button only active on falling edge, set to 0 once processed
  if (!wheel_new_btns.aux1 && wheel_old_btns.aux1) btn_state = 1;
  wheel_old_btns = wheel_new_btns;
  // TODO: revert, just using adc pins to test joystick
  uint16_t th = 4000 / 2;
//    PHAL_SPI_transfer(&hspi1, wheel_read_cmd, sizeof(wheel_read_cmd), &wheel_new_btns);
  wheel_new_btns.up   = raw_pedals.t1 > th;
  wheel_new_btns.down = raw_pedals.t2 > th;
  wheel_new_btns.left = raw_pedals.b1 > th;
  wheel_new_btns.right = raw_pedals.b2 > th;
  wheel_new_btns.aux1 = raw_pedals.b3 > th;
  // skip if center or no buttons on page
  if (j_dir != J_CENTER && pages[p_idx].buttons)
  {
      // determine new selection
      uint8_t new_selection = pages[p_idx].buttons[b_idx].dirs[j_dir];
      // debounce
      j_dir = J_CENTER;
      if (new_selection != b_idx)
      {
          // un-highlight the old button
          set_value(pages[p_idx].buttons[b_idx].name,
                  NXT_PICTURE,
                  pages[p_idx].buttons[b_idx].norm_id);
      }
      // highlight the new button
      set_value(pages[p_idx].buttons[new_selection].name,
              NXT_PICTURE,
              pages[p_idx].buttons[new_selection].high_id);
      b_idx = new_selection;
      // dont unintentionally select something
      btn_state = 0;
      // update change time
      b_change_time = sched.os_ticks;
  }
  else
  {
      if (b_change_time != 0 && sched.os_ticks - b_change_time > BTN_SELECT_TIMEOUT_MS)
      {
          // unselect button
          set_value(pages[p_idx].buttons[b_idx].name,
                    NXT_PICTURE,
                    pages[p_idx].buttons[b_idx].norm_id);
          b_change_time = 0;
      }
  }
  actionUpdatePeriodic();
}
void check_buttons() {
    static bool pressed_once;
    bool page_changed = false;
    static uint32_t b_selected;
    static uint32_t num_iterations;
   // if (!wheel_new_btns.aux3 && !wheel_old_btns.aux3) {
   //     switch(p_idx) {
   //         case P_RACE:
   //             set_page("extra_info\0");
   //             p_idx = P_EXTRA_INFO;
   //             p_idx_perr = P_EXTRA_INFO;
   //             page_changed = true;
   //             break;
   //         case P_EXTRA_INFO:
   //             set_page("race\0");
   //             p_idx = P_RACE;
   //             p_idx_perr = P_RACE;
   //             page_changed = true;
   //             break;
   //     }

   // }
   if ((!wheel_new_btns.aux3 && wheel_old_btns.aux3) && (num_iterations == 0)) {
       b_selected = sched.os_ticks;
       pressed_once = true;
   }
   else if ((!wheel_new_btns.aux3 && wheel_old_btns.aux3) && (num_iterations != 0)) {
           switch(p_idx) {
               case P_RACE:
                   set_page("extra_info\0");
                   p_idx = P_EXTRA_INFO;
                   p_idx_perr = P_EXTRA_INFO;
                   page_changed = true;
                   break;
               case P_EXTRA_INFO:
                   set_page("race\0");
                   p_idx = P_RACE;
                   p_idx = P_RACE;
                   page_changed = true;
                   break;
       }
   }
   else if ((wheel_new_btns.aux3 && wheel_new_btns.aux3) && pressed_once) {
       num_iterations++;
   }


   if (num_iterations == 4 || page_changed) {
       if (!page_changed) {
           b_selected_time = b_selected;
           b_selected = 0;
           if (prev_time != 0 && curr_lap_time != 0) {
               curr_lap_time = b_selected_time - orig_time;
               delta_old = delta;
               delta = curr_lap_time - prev_time;
               prev_time =  curr_lap_time;
           }
           else if (prev_time == 0 && curr_lap_time != 0) {
               prev_time = curr_lap_time;
               curr_lap_time = 0;
           }

       }
       num_iterations = 0;
       pressed_once = false;
       page_changed = false;
   }

   wheel_old_btns = wheel_new_btns;
   uint8_t new_data[3];
   bool b = PHAL_SPI_busy();
   bool c = PHAL_SPI_transfer(&hspi1, wheel_read_cmd, 3, new_data);
   while (PHAL_SPI_busy());

   wheel_new_btns.raw_data = new_data[2];

}
void actionUpdatePeriodic()
{
  // Based on current selection, run the action associated to the element
//     if (!btn_state) return;
//     switch (p_idx)
//     {
//         case P_MAIN: // main
//             switch(b_idx)
//             {
//                 case B_M_BUTTON:
//                     changePage(P_INFO);
//                     break;
//             }
//             break;
//         case P_INFO:
//             switch(b_idx)
//             {
//                 case B_B_BUTTON:
//                     changePage(P_MAIN);
//                     break;
//             }
//             break;
//     }
//     // debounce
//     btn_state = 0;
}


void valueUpdatePeriodic()
{
  // Ran periodically at a refresh rate
  // Based on the current page, update all values
  // many will be from the can_data struct
  // possible to check if values are stale
  // switch (p_idx)
  // {
  //     case P_MAIN:
  //         /* SPEED */
  //         float speed = can_data.front_wheel_data.left_speed +
  //                       ((float) (can_data.front_wheel_data.right_speed -
  //                       can_data.front_wheel_data.left_speed)) / 2;
  //         // TODO: convert speed to MPH
  //         set_float(A_SPEED, NXT_TEXT, speed, 1);
  //         /* VOLTAGE */
  //         // TODO: get voltage
  //         set_float(A_VOLTAGE, NXT_TEXT, 0.00, 2);
  //         /* BATTERY */
  //         // TODO: get soc
  //         set_value(A_BATTERY, NXT_VALUE, 0);
  //         /* POWER */
  //         // TODO: get main status
  //         set_value(A_POWER, NXT_PICTURE, POWER_OFF_PIC);
  //         /* TV STATUS */
  //         // TODO: get tv stat
  //         set_value(A_TV_STATUS, NXT_PICTURE, TV_STAT_OFF_PIC);
  //         break;
  //     case P_INFO:
  //         break;
  // }
 }
void update_time() {
  static uint32_t wait_state;
  uint32_t time_to_send = 0;
  uint32_t curr_time = sched.os_ticks;
  if (b_selected_time > 0) {
      orig_time = b_selected_time;
  }
  else {
      if (orig_time == 0) {
          orig_time = curr_time;
      }
  }
  time_to_send = curr_time - orig_time;
   curr_lap_time = time_to_send;
  uint16_t milliseconds = time_to_send % 1000;
  time_to_send /= 1000;
  uint8_t minutes = time_to_send / 60;
  if (minutes > 99)
      minutes = 99;
  uint8_t seconds = time_to_send % 60;
  char parsed[8] = "";
  for (int i = 7; i > -1; i--) {
      if ((i == 5) || (i == 2)) {
          parsed[i] = ':';
          continue;
      }
      if (i > 5) {
          parsed[i] = (milliseconds % 10) + '0';
          milliseconds /= 10;
      }
      else if (i > 2) {
          parsed[i] = (seconds % 10) + '0';
          seconds /= 10;
      }
      else {
          parsed[i] = (minutes % 10) + '0';
          minutes /= 10;
      }
  }
  // TODO: Add functionality with other pages, and persistance through error pages
  if (b_selected_time > 0) {
      switch(wait_state) {
          case 0:
               wait_state++;
          case 10:
               b_selected_time = 0;
               wait_state = 0;
               return;
           default:
               wait_state++;
               return;
      }
  }
  else {
      if (p_idx == P_RACE)
           set_text("t3\0", NXT_TEXT, parsed);
       else if (p_idx == P_EXTRA_INFO)
           set_text("t1\0", NXT_TEXT, parsed);
  }

}
void check_error() {
    if ((p_idx == P_INFO) || (p_idx == P_WARNING) || (p_idx == P_CRITICAL)) {
       return;
    }
    if (can_data.main_hb.car_state == CAR_STATE_ERROR && can_data.main_hb.precharge_state != 0) {
        err_msg = "Car State Error\0";
       p_idx_perr = p_idx;
       p_idx = P_CRITICAL;
       car_stat_color = RED_ERR;
       update_err_pages();
   }
   else if (can_data.main_hb.car_state == CAR_STATE_FATAL) {
       p_idx_perr = p_idx;
        err_msg = "Car State Fatal\0";
       p_idx = P_CRITICAL;
       car_stat_color = RED_ERR;
       update_err_pages();
   }
   else if (can_data.main_hb.car_state == CAR_STATE_RECOVER) {
        err_msg = "Car State Recover\0";
       p_idx_perr = p_idx;
       p_idx = P_WARNING;
       car_stat_color = YELLOW_ERR;
       update_err_pages();
   }
   else if (can_data.main_hb.car_state == CAR_STATE_RESET) {
        err_msg = "Car State Reset\0";
       p_idx_perr = p_idx;
       p_idx = P_INFO;
       car_stat_color = RED_ERR;
       update_err_pages();
   }
}
void update_page() {
  if (p_idx == p_idx_prev)
      return;
  switch (p_idx) {
      case P_STARTUP:
          p_idx_prev = P_STARTUP;
          set_page("startup\0");
          return;
      case P_RACE:
          p_idx_prev = P_RACE;
          set_page("race\0");
          return;
      case P_EXTRA_INFO:
          p_idx_prev = P_EXTRA_INFO;
          set_page("extra_info\0");
          return;
      case P_INFO:
          p_idx_prev = P_INFO;
          set_page("info\0");
          return;
      case P_WARNING:
          p_idx_prev = P_WARNING;
          set_page("warning\0");
          return;
      case P_CRITICAL:
          p_idx_prev = P_CRITICAL;
          set_page("critical\0");
          return;
  }
}
void update_err_pages() {
  static uint8_t value;
  if ((p_idx != P_WARNING) && (p_idx != P_INFO) && (p_idx != P_CRITICAL)) {
      return;
  }
  if (value == 100) {
      switch (p_idx) {
          case P_WARNING:
              car_stat_color = YELLOW_ERR;
              break;
          case P_CRITICAL:
              car_stat_color = RED_ERR;
              break;
      }
      p_idx = p_idx_perr;
      switch (p_idx_perr) {
          case P_STARTUP:
               set_page("startup\0");
               break;
          case P_RACE:
               set_page("race\0");
               return;
           case P_EXTRA_INFO:
               set_page("extra_info\0");
               return;
      }
      return;
  }
  if (value == 0) {
      switch(p_idx) {
          case P_INFO:
               set_page("info\0");
               break;
           case P_WARNING:
               set_page("warning\0");
               break;
           case P_CRITICAL:
               set_page("critical\0");
               break;
      }
  }
  value += 5;
  set_value("j0\0", NXT_VALUE, value);
  set_text("t1\0", NXT_TEXT, err_msg);
}
void update_race_page(void) {
  if (p_idx != P_RACE) {
      return;
  }
  //This is an enum found in can_parse.h
  // if (strcmp(can_data.front_driveline_hb.front_left_motor, "disconnected") == 0) {
  //     err_msg = "Front Driveline Disconnected\0";
  //     p_idx = P_CRITICAL;
  // }
  // else if (strcmp(can_data.front_driveline_hb.front_left_motor, "error") == 0) {
  //     err_msg = "Front Driveline Error\0";
  //     p_idx = P_CRITICAL;
  // }
  // if (strcmp(can_data.rear_driveline_hb.back_left_motor, "disconnected") == 0) {
  //     err_msg = "Rear Driveline Disconnected\0";
  //     p_idx = P_CRITICAL;
  // }
  // else if (strcmp(can_data.rear_driveline_hb.back_left_motor, "error") == 0) {
  //     err_msg = "Rear Driveline Error\0";
  //     p_idx = P_CRITICAL;
  // }
  if (strcmp(err_msg, prev_err_msg) != 0) {
      set_text("t11\0", NXT_TEXT, err_msg);
      prev_err_msg = err_msg;
  }
//   uint8_t front_left_temp = can_data.front_motor_currents_temps.left_temp;
//   uint8_t front_right_temp = can_data.front_motor_currents_temps.right_temp;
//   uint8_t rear_left_temp = can_data.rear_motor_currents_temps.left_temp;
//   uint8_t rear_right_temp = can_data.rear_motor_currents_temps.right_temp;
//   char average = ((front_left_temp + front_right_temp + rear_left_temp + rear_right_temp) / 4) + '0';
//   set_text("t14\0", NXT_TEXT, &average);
//   float front_left_gas = can_data.torque_request_main.front_left;
//   float front_right_gas = can_data.torque_request_main.front_right;
//   float rear_left_gas = can_data.torque_request_main.rear_left;
//   float rear_right_gas = can_data.torque_request_main.rear_right;
//   int raw_gas = pedals.throttle;
//   int raw_brake = pedals.brake * -1;
//   float avg_gas = raw_gas + raw_brake;
//    gas_to_send = ((front_left_gas + front_right_gas + rear_left_gas + rear_right_gas) / 4) / 4095 * 100;
//    gas_to_send = rear_left_gas /4095.0 * 400;

//    // gas_to_send = ((can_data.torque_request_main.front_left
//    //                 + can_data.torque_request_main.front_right
//    //                 + can_data.torque_request_main.rear_left
//    //                 + can_data.torque_request_main.rear_right) / 4.0) / 4095.0 * 100;
//   if (gas_to_send < 0) {
//       set_value("j1\0", NXT_VALUE, (100 + gas_to_send));
//       set_value("j2\0", NXT_VALUE, 0);
//   }
//   else {
//       set_value("j1\0", NXT_VALUE, 100);
//       set_value("j2\0", NXT_VALUE, gas_to_send);
//   }
//    avg_soc = (can_data.soc_cells.soc1 + can_data.soc_cells.soc2 + can_data.soc_cells.soc3) / 3;
//   char soc[4];
//   sprintf(soc, "%d", avg_soc);
//   set_text("t6\0", NXT_TEXT, soc);
//   char temp[4];
//   sprintf(temp, "%d", ((can_data.temps_cells.t1 + can_data.temps_cells.t2 + can_data.temps_cells.t3) /3));
//   set_text("t8\0", NXT_TEXT, soc);
//   char volt[4];
//   sprintf(volt, "%d", ((can_data.volts_cells.v1 + can_data.volts_cells.v2 + can_data.volts_cells.v3) /3));
//   set_text("t10\0", NXT_TEXT, volt);
//    set_value("j0\0", NXT_VALUE, avg_soc);

//   char speed[3];
//   sprintf(speed, "%d", ((can_data.front_wheel_data.right_speed + can_data.front_wheel_data.left_speed + can_data.rear_wheel_data.right_speed + can_data.rear_wheel_data.left_speed) / 4));
//   set_text("t0\0", NXT_TEXT, speed);


//     char parsed_data[5];
//     if (completed_time == 0 || prev_time == 0) {
//         parsed_data = {'0', '.', '0', '0', '\0'};
//     }
//    set_text("t1\0", NXT_TEXT, parsed_data);

   if (delta == 0) {
       set_text("t1\0", NXT_TEXT, "0.00\0");
   }
   else if ((delta != 0)) {
       char parsed_data[10];
       if (delta >= 10000 || delta <= -10000 ) {
           float delta_interpreted = delta/1000.0;
           sprintf(parsed_data, "%2.1f", delta_interpreted);
           set_text("t1\0", NXT_TEXT, parsed_data);
           //if (delta_interpreted == 3.3)
           //    __asm__("bkpt");
           }
       else {
           float delta_interpreted = delta/1000.0;
           sprintf(parsed_data, "%1.3f", delta_interpreted);
           set_text("t1\0", NXT_TEXT, parsed_data);
           //if (delta_interpreted == 3.3)
           //    __asm__("bkpt");

       }
   }
}

void update_race_colors() {
   if (p_idx != P_RACE) {
       return;
   }
   if (avg_soc > 69) {
       set_value("j0\0", NXT_FONT_COLOR, 1376);
   }
   else if (avg_soc > 39) {
       set_value("j0\0", NXT_FONT_COLOR, 59011);
   }
   else if (avg_soc > 14) {
       set_value("j0\0", NXT_FONT_COLOR, 60323);
   }
   else {
       set_value("j0\0", NXT_FONT_COLOR, 47104);
   }

   (gas_to_send < 0) ? set_value("j1\0", NXT_BACKGROUND_COLOR, 65535) : set_value("j1\0", NXT_BACKGROUND_COLOR, 47104);

   if (strcmp(err_msg, prev_err_msg) != 0) {
      set_value("t11\0", NXT_BACKGROUND_COLOR, car_stat_color);
  }
}

void check_precharge() {
   if (p_idx != P_STARTUP) {
       if ((can_data.main_hb.precharge_state == 0) && (precharge_complete)) {
           precharge_complete = false;
           set_page("startup\0");
           p_idx = P_STARTUP;
       }
       return;
   }
   // set_value("g0\0", "txt_maxl\0", 1000);

   if (can_data.main_hb.precharge_state) {
       precharge_complete = true;
       // set_text("g0\0", NXT_TEXT, "Precharge Complete! Press the start button to continue: \0");
       set_value("power\0", "=\0", 1);

   }
   else if ((can_data.main_hb.precharge_state == 0) && (precharge_complete)) {
       precharge_complete = false;
       set_text("g0\0", NXT_TEXT, "HV OFF");
   }
   else {
       set_value("power\0", "=\0", 0);

   }

   if (can_data.main_hb.car_state == CAR_STATE_READY2DRIVE) {
       p_idx = P_RACE;
       set_page("race\0");
   }
}
void changePage(uint8_t new_page)
{
  if (new_page == p_idx) return;
  set_page(pages[new_page].name);
  p_idx = new_page;
  b_idx = 0;
}
