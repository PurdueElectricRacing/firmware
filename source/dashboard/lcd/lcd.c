#include "lcd.h"
#include "common/psched/psched.h"
#include "pedals.h"
#include "common_defs.h"
#include <string.h>
#include <stdio.h>
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
static char* _float_to_char(float, char*, int);
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
bool precharge_complete = false;
int prev_time = 0;
int curr_lap_time = 0;
int delta = 0;
int delta_old = 0;
uint32_t orig_time = 0;
static uint8_t time_wait = 0;
uint8_t new_data[3];
/* Joystick Management */
//Not In use anymore - Joystick functionality removed
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

//Data struct for SPI button + joystick reception
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
//Wheel button SPI defs
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
    //Has the button actually been pressed? And is this the first time?
   if ((!wheel_new_btns.aux3 && wheel_old_btns.aux3) && (num_iterations == 0)) {

    b_selected = sched.os_ticks; //The button was selected at this time, and record that the button was pressed
       pressed_once = true;
   }
   // The button was pressed another time within the window to switch pages
   else if ((!wheel_new_btns.aux3 && wheel_old_btns.aux3) && (num_iterations != 0)) {
        //Change to the other page, and record that a page was changed
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
   //If the button was not pressed during this iteration of the function, record it.
   else if ((wheel_new_btns.aux3 && wheel_old_btns.aux3) && pressed_once) {
       num_iterations++;
   }

    //Has the function ran 4 times without another button press, or has the page changed?
   if (num_iterations >= 7 || page_changed) {
    //The page has not changed, so set delta and current lap timings accordingly
       if (!page_changed) {
           b_selected_time = b_selected;
           b_selected = 0;
           //Not the first lap
           if (prev_time != 0 && curr_lap_time != 0) {
               curr_lap_time = b_selected_time - orig_time;
               delta_old = delta;
               delta = curr_lap_time - prev_time;
               prev_time =  curr_lap_time;
           }
           //The first lap; No other info to compare it with
           else if (prev_time == 0 && curr_lap_time != 0) {
               prev_time = curr_lap_time;
               curr_lap_time = 0;
           }

       }
       //Reset to standard settings to complete loop again
       num_iterations = 0;
       pressed_once = false;
       page_changed = false;
   }
    //Update info, cycle SPI and get new data for next iteration
   wheel_old_btns = wheel_new_btns;
    bool b = PHAL_SPI_busy(&hspi1);
    bool c = PHAL_SPI_transfer(&hspi1, wheel_read_cmd, 3, new_data);
    while (PHAL_SPI_busy(&hspi1));
   wheel_new_btns.raw_data = new_data[2];

}


//Deprecated; Used to control Joystick, but not tested
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

//Previous Screen update function - Deprecated
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

 //Function to update lap timer on screen
void update_time() {
  static uint32_t wait_state;
  uint32_t time_to_send = 0;
  uint32_t curr_time = sched.os_ticks;

  //Has button been pressed; If pressed, set the time it was pressed at
  if (b_selected_time > 0) {
      orig_time = b_selected_time;
  }
  //Set the original time properly
  else {
      if (orig_time == 0) {
          orig_time = curr_time;
      }
  }
  //Calculate time since last button press to send to screen
  time_to_send = curr_time - orig_time;
   curr_lap_time = time_to_send;
   //Calculate milliseconds, seconds, and minutes of the time, from only milliseconds
  uint16_t milliseconds = time_to_send % 1000;
  time_to_send /= 1000;
  uint8_t minutes = time_to_send / 60;
  //Prevent overflow
  if (minutes > 99)
      minutes = 99;
  uint8_t seconds = time_to_send % 60;
  //Parse each component into a string
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
  //Delay timer to make it readable to user
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
  //Send to screen (Created in this manner to fine tune how fast I want to update it without adding stress to micro)
  else {
    if (time_wait++ == 4) {
      if (p_idx == P_RACE)
           set_text("t3\0", NXT_TEXT, parsed);
       else if (p_idx == P_EXTRA_INFO)
           set_text("t1\0", NXT_TEXT, parsed);
        time_wait = 0;
    }

  }

}
//Function to update progress bars on error pages
void updateBarsFast() {
    if (p_idx != P_RACE) {
        return;
    }
    set_value("j2\0", NXT_VALUE, (int)((can_data.torque_request_main.rear_right / 4095.0) * 100));
    set_value("j1\0", NXT_VALUE, (100 - (int)((can_data.torque_request_main.rear_left / 4095.0) * 100)));

}
//Check whether an error was found and to update error pages accordingly
void check_error() {
    if ((p_idx == P_INFO) || (p_idx == P_WARNING) || (p_idx == P_CRITICAL)) {
       return;
    }
    if (can_data.main_hb.car_state == CAR_STATE_ERROR && can_data.main_hb.precharge_state != 0) {
        err_msg = "Car State Error\0";
        //Keep all values updated for other functions
       p_idx_perr = p_idx;
       p_idx = P_CRITICAL;
       car_stat_color = RED_ERR;
       update_err_pages();
   }
   else if (can_data.main_hb.car_state == CAR_STATE_FATAL) {
        //Keep all values updated for other functions
       p_idx_perr = p_idx;
        err_msg = "Car State Fatal\0";
       p_idx = P_CRITICAL;
       car_stat_color = RED_ERR;
       update_err_pages();
   }
   else if (can_data.main_hb.car_state == CAR_STATE_RECOVER) {
        //Keep all values updated for other functions
        err_msg = "Car State Recover\0";
       p_idx_perr = p_idx;
       p_idx = P_WARNING;
       car_stat_color = YELLOW_ERR;
       update_err_pages();
   }
   else if (can_data.main_hb.car_state == CAR_STATE_RESET) {
        //Keep all values updated for other functions
        err_msg = "Car State Reset\0";
       p_idx_perr = p_idx;
       p_idx = P_INFO;
       car_stat_color = RED_ERR;
       update_err_pages();
   }
}

//Function to add text to error pages, keep them in sync
void update_err_pages() {
  static uint8_t value;
  if ((p_idx != P_WARNING) && (p_idx != P_INFO) && (p_idx != P_CRITICAL)) {
      return;
  }
  if (value == 100) {
      switch (p_idx) {
        //Set the status bar on the race page
          case P_WARNING:
              car_stat_color = YELLOW_ERR;
              break;
          case P_CRITICAL:
              car_stat_color = RED_ERR;
              break;
      }
      p_idx = p_idx_perr;
      switch (p_idx_perr) {
        //Switch to the page user was on before the error
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
    //Select the proper error page for the situation
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

//Function to update the race + extra info page with relavant information
void update_info_pages(void) {
    char two_int_char[3] = {"\0"};
    char three_int_char[4] = {"\0"};
  switch(p_idx) {
    //Switch statement to see which page to update
    case P_RACE:
        //Set the error bar to the previous message, if there was one
        if (strcmp(err_msg, prev_err_msg) != 0) {
            set_text("t11\0", NXT_TEXT, err_msg);
            prev_err_msg = err_msg;
        }
        // Convert various can messages from integers to strings, then send them to the screen; Make sure char array is empty before next use (memset)
        int_to_char(MAX(can_data.rear_motor_currents_temps.right_temp, can_data.rear_motor_currents_temps.left_temp), three_int_char, 3);
        set_text("t14\0", NXT_TEXT, three_int_char);

        int_to_char((can_data.orion_info.pack_soc / 2), two_int_char, 2);
        set_value("j0\0", NXT_VALUE, can_data.orion_info.pack_soc / 2);
        set_text("t6\0", NXT_TEXT, two_int_char);
        memset(two_int_char, 3, '\0');
        int_to_char((can_data.orion_currents_volts.pack_voltage / 10), three_int_char, 3);
        set_text("t10\0", NXT_TEXT, three_int_char);
        memset(three_int_char, 4, '\0');
        int_to_char(can_data.max_cell_temp.max_temp / 10, two_int_char, 2);
        set_text("t8\0", NXT_TEXT, two_int_char);
        memset(two_int_char, 3, '\0');

        int_to_char((int) (((can_data.rear_wheel_data.left_speed +
                                    can_data.rear_wheel_data.right_speed) / 200.0) * 0.6213711922), two_int_char, 2);
        set_text("t0\0", NXT_TEXT, two_int_char);
        memset(two_int_char, 3, '\0');
        int_to_char((can_data.rear_controller_temps.left_temp + can_data.rear_controller_temps.right_temp) / 2, two_int_char, 2);
        set_text("t19\0", NXT_TEXT, two_int_char);
        memset(two_int_char, 3, '\0');
        //Set the delta section on the screen, based on previous times
        if (delta == 0) {
            set_text("t1\0", NXT_TEXT, "0.00\0");
        }
        else if ((delta != 0)) {
            char parsed_data[6] = {'\0'};
                float delta_interpreted = delta/1000.0;
                _float_to_char(delta_interpreted, parsed_data, 5);
                set_text("t1\0", NXT_TEXT, parsed_data);
                (delta_interpreted > 0) ? set_value("t1\0", NXT_FONT_COLOR, RED) : set_value("t1\0", NXT_FONT_COLOR, GREEN);

        }
        break;
    case P_EXTRA_INFO:
        //Set delta, according to previous information
        if (delta == 0) {
            set_text("t2\0", NXT_TEXT, "0.00\0");
        }
        else if ((delta != 0)) {
            char parsed_data[6] = {'\0'};
            float delta_interpreted = delta/1000.0;
            _float_to_char(delta_interpreted, parsed_data, 5);
            set_text("t2\0", NXT_TEXT, parsed_data);
            (delta_interpreted > 0) ? set_value("t2\0", NXT_FONT_COLOR, RED) : set_value("t2\0", NXT_FONT_COLOR, GREEN);

        }
        // Convert various can messages from integers to strings, then send them to the screen; Make sure char array is empty before next use (memset)
        int_to_char(can_data.rear_motor_currents_temps.left_temp, three_int_char, 3);
        set_text("t5\0", NXT_TEXT, three_int_char);
        memset(three_int_char, 4, '\0');
        int_to_char(can_data.rear_motor_currents_temps.right_temp, three_int_char, 3);
        set_text("t6\0", NXT_TEXT, three_int_char);
        memset(three_int_char, 4, '\0');

        int_to_char((int) (((can_data.rear_wheel_data.left_speed +
                                    can_data.rear_wheel_data.right_speed) / 200.0) * 0.6213711922), two_int_char, 2);
        set_text("t3\0", NXT_TEXT, two_int_char);
        memset(two_int_char, 3, '\0');
        int_to_char(can_data.rear_controller_temps.left_temp, two_int_char, 2);
        set_text("t15\0", NXT_TEXT, two_int_char);
        memset(two_int_char, 3, '\0');
        int_to_char(can_data.rear_controller_temps.right_temp, two_int_char, 2);
        set_text("t16\0", NXT_TEXT, two_int_char);
        memset(two_int_char, 3, '\0');
        int_to_char((int)can_data.orion_info.pack_dcl, two_int_char, 2);
        set_text("t17\0", NXT_TEXT, two_int_char);
        memset(two_int_char, 3, '\0');
        int_to_char((int)can_data.orion_info.pack_ccl, two_int_char, 2);
        set_text("t17\0", NXT_TEXT, two_int_char);
        memset(two_int_char, 3, '\0');
        int_to_char((int)can_data.orion_currents_volts.pack_current, two_int_char, 2);
        set_text("t13\0", NXT_TEXT, two_int_char);
        memset(two_int_char, 3, '\0');
        int_to_char((int)(can_data.orion_currents_volts.pack_voltage / 10), three_int_char, 3);
        set_text("t12\0", NXT_TEXT, three_int_char);
        set_value("j0\0", NXT_VALUE, can_data.orion_info.pack_soc / 2);
        memset(three_int_char, 4, '\0');
        int_to_char((int)(can_data.orion_currents_volts.pack_current / 10), three_int_char, 3);
        set_text("t18\0", NXT_TEXT, three_int_char);
        break;
  }
}
//Function to update the colors on the pages (not needed to happen as frequently)
void update_race_colors() {
    uint16_t temp = can_data.max_cell_temp.max_temp;
    switch(p_idx) {
        case P_RACE:
            //Depending on value, set color (Green = good, yellow = watch closely, red = car failing imminently)
            //Max temp between left + right motors
            if (can_data.rear_motor_currents_temps.left_temp < 40 && can_data.rear_motor_currents_temps.right_temp < 40) {
                set_value("t14\0", NXT_FONT_COLOR, GREEN);
            }
            else if ((can_data.rear_motor_currents_temps.left_temp > 39 && can_data.rear_motor_currents_temps.left_temp < 70) || (can_data.rear_motor_currents_temps.right_temp > 39 && can_data.rear_motor_currents_temps.right_temp < 70)) {
                set_value("t14\0", NXT_FONT_COLOR, YELLOW);
            }
            else {
                set_value("t14\0", NXT_FONT_COLOR, RED);
            }
            //Pack SOC
            if ((can_data.orion_info.pack_soc / 2) > 49) {
                set_value("j0\0", NXT_FONT_COLOR, GREEN);
                set_value("t6\0", NXT_FONT_COLOR, GREEN);
            }
            else if (((can_data.orion_info.pack_soc / 2) < 50) && ((can_data.orion_info.pack_soc / 2) > 30)) {
                set_value("j0\0", NXT_FONT_COLOR, YELLOW);
                set_value("t6\0", NXT_FONT_COLOR, YELLOW);

            }
            else {
                set_value("j0\0", NXT_FONT_COLOR, RED);
                set_value("t6\0", NXT_FONT_COLOR, RED);
            }
            //Max cell temp in battery
            if (temp < 36) {
                set_value("t8\0", NXT_FONT_COLOR, GREEN);
            }
            else if (temp > 35 && temp < 50) {
                set_value("t8\0", NXT_FONT_COLOR, YELLOW);
            }
            else {
                set_value("t8\0", NXT_FONT_COLOR, RED);
            }
            //Max temp between each controller
            if (can_data.rear_controller_temps.left_temp < 40 && can_data.rear_controller_temps.right_temp < 40) {
                set_value("t19\0", NXT_FONT_COLOR, GREEN);
            }
            else if ((can_data.rear_controller_temps.left_temp > 39 && can_data.rear_controller_temps.left_temp < 70) || (can_data.rear_controller_temps.right_temp > 39 && can_data.rear_controller_temps.right_temp < 70)) {
                set_value("t19\0", NXT_FONT_COLOR, YELLOW);
            }
            else {
                set_value("t19\0", NXT_FONT_COLOR, RED);
            }
            set_value("j1\0", NXT_BACKGROUND_COLOR, GREEN);
            break;
        case P_EXTRA_INFO:
            //Depending on value, set color (Green = good, yellow = watch closely, red = car failing imminently)
            //Left motor temps
            if (can_data.rear_motor_currents_temps.left_temp < 60){
                set_value("t5\0", NXT_FONT_COLOR, GREEN);
            }
            else if (can_data.rear_motor_currents_temps.left_temp > 39 && can_data.rear_motor_currents_temps.left_temp < 70)
                set_value("t5\0", NXT_FONT_COLOR, YELLOW);
            else
                set_value("t5\0", NXT_FONT_COLOR, RED);
            if (can_data.rear_motor_currents_temps.right_temp < 60) {
                set_value("t6\0", NXT_FONT_COLOR, GREEN);
            }
            //Right motor temps
            else if (can_data.rear_motor_currents_temps.right_temp > 39 && can_data.rear_motor_currents_temps.right_temp < 70)
                set_value("t6\0", NXT_FONT_COLOR, YELLOW);
            else
                set_value("t6\0", NXT_FONT_COLOR, RED);
            //Pack SOC
            if ((can_data.orion_info.pack_soc / 2)  > 49) {
                set_value("j0\0", NXT_FONT_COLOR, GREEN);
            }
            else if (((can_data.orion_info.pack_soc / 2)  < 50) && ((can_data.orion_info.pack_soc / 2)  > 30)) {
                set_value("j0\0", NXT_FONT_COLOR, YELLOW);
            }
            else {
                set_value("j0\0", NXT_FONT_COLOR, RED);
            }
            //Left controller temps
            if (can_data.rear_controller_temps.left_temp < 60){
                set_value("t5\0", NXT_FONT_COLOR, GREEN);
            }
            else if (can_data.rear_controller_temps.left_temp > 39 && can_data.rear_controller_temps.left_temp < 70)
                set_value("t5\0", NXT_FONT_COLOR, YELLOW);
            else
                set_value("t5\0", NXT_FONT_COLOR, RED);
            //Right controller temps
            if (can_data.rear_controller_temps.right_temp < 60) {
                set_value("t6\0", NXT_FONT_COLOR, GREEN);
            }
            else if (can_data.rear_controller_temps.right_temp > 39 && can_data.rear_controller_temps.right_temp < 70)
                set_value("t6\0", NXT_FONT_COLOR, YELLOW);
            else
                set_value("t6\0", NXT_FONT_COLOR, RED);

            break;

    }
}
//Controls the startup page
void check_precharge() {
    //Make sure we are on the correct page, and go back to startup page if precharge is incomplete
   if (p_idx != P_STARTUP) {
       if ((can_data.main_hb.precharge_state == 0) && (precharge_complete)) {
           precharge_complete = false;
           set_page("startup\0");
           p_idx = P_STARTUP;
       }
       return;
   }
   //Set character limit to proper length
   set_value("g0\0", "txt_maxl\0", 1000);
    //Update page if precharge is complete
   if (can_data.main_hb.precharge_state) {
       precharge_complete = true;
       set_text("g0\0", NXT_TEXT, "Precharge Complete! Press the start button to continue: \0");
       set_value("power\0", "=\0", 1);

   }
   //Went from precharged back to HV off state
   else if ((can_data.main_hb.precharge_state == 0) && (precharge_complete)) {
       precharge_complete = false;
       set_text("g0\0", NXT_TEXT, "HV OFF");
   }
   else {
       set_value("power\0", "=\0", 0);

   }
    //If car is succesfully active, go to race page
   if (can_data.main_hb.car_state == CAR_STATE_READY2DRIVE) {
       p_idx = P_RACE;
       set_page("race\0");
   }
}

//Function to change page easily
void changePage(uint8_t new_page)
{
  if (new_page == p_idx) return;
  set_page(pages[new_page].name);
  p_idx = new_page;
  b_idx = 0;
}

//Function to do (basic) 2 decimal float to char conversion, as that is all I need - gets unnecesarily complicated if I do it the actual way
static char * _float_to_char(float x, char *p, int buff_size) {
    const int CHAR_BUFF_SIZE = buff_size;
    char *s = p + CHAR_BUFF_SIZE; // go to end of buffer
    uint16_t decimals;  // variable to store the decimals
    int units;  // variable to store the units (part to left of decimal place)
    if (x < 0) { // take care of negative numbers
        decimals = (int)(x * -100) % 100; // make 1000 for 3 decimals etc.
        units = (int)(-1 * x);
    } else { // positive numbers
        decimals = (int)(x * 100) % 100;
        units = (int)x;
    }

    *--s = (decimals % 10) + '0';
    decimals /= 10; // repeat for as many decimal places as you need
    *--s = (decimals % 10) + '0';
    *--s = '.';


    *--s = (units % 10) + '0';
    if (x < 0) *--s = '-';
    else *--s = '+'; // unary minus sign for negative numbers
    return s;
}

//Convert an integer to a char array to be sent to screen
static char* int_to_char(int x, char *str, int buffer) {
    //If the integer is 0 or negative
    if (x < 1) {
        //Go to end of array
        char *s = str + buffer - 1;
        //If x is just 0
        if(x == 0)
        {
            *str = '0';
            return str;
        }
        //Iterate over the integer, add it to array, then append a '-'
        while (x > 0) {
            *s-- = (x % 10) + '0';
            x /=10;
        }
        *s='-';
    return s;
    }
    //Iterate over teh integer, adding it to the array
    char *s = str + buffer - 1;
    if(x == 0)
    {
        *str = '0';
        return str;
    }
    while (x > 0) {
        *s-- = (x % 10) + '0';
        x /=10;
    }
    return s;
}