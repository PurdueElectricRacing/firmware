#include "lcd.h"

extern SPI_InitConfig_t hspi1;

button_t main_buttons[] = {
    {.name="mButton", .norm_id=B_M_BUTTON_NORM_PIC, .high_id=B_M_BUTTON_HIGH_PIC,
     .dirs={B_M_BUTTON, B_M_BUTTON, B_M_BUTTON, B_M_BUTTON}},
};

button_t info_buttons[] = {
    {.name="bButton", .norm_id=B_BUTTON_NORM_PIC, .high_id=B_BUTTON_HIGH_PIC,
     .dirs={B_B_BUTTON, B_B_BUTTON, B_B_BUTTON, B_B_BUTTON}},
};

page_t pages[P_TOTAL] = {
    {.name="splash"},
    {.name="main", .buttons=main_buttons, .num_buttons=B_MAIN_TOTAL},
    {.name="info", .buttons=info_buttons, .num_buttons=B_INFO_TOTAL}
};

uint8_t p_idx = P_MAIN;
uint8_t b_idx = 0; 
uint32_t b_change_time = 0;

/* Joystick Management */
typedef enum {
    J_UP,
    J_RIGHT,
    J_DOWN,
    J_LEFT,
    J_CENTER
} joystick_dir_t;

joystick_dir_t j_dir = J_CENTER;
uint8_t btn_state = 0; // activate only on falling edge

uint8_t wheel_read_cmd[] = {WHEEL_SPI_ADDR & WHEEL_SPI_READ, WHEEL_GPIO_REG};
typedef struct __attribute__((packed))
{
    uint8_t up    :1;
    uint8_t down  :1;
    uint8_t left  :1;
    uint8_t right :1;
    uint8_t aux1  :1;
    uint8_t aux2  :1;
    uint8_t aux3  :1;
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
    //PHAL_SPI_transfer(&hspi1, wheel_read_cmd, sizeof(wheel_read_cmd), &wheel_new_btns);
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

void actionUpdatePeriodic()
{
    // Based on current selection, run the action associated to the element

    if (!btn_state) return;
    switch (p_idx)
    {
        case P_MAIN: // main
            switch(b_idx)
            {
                case B_M_BUTTON:
                    changePage(P_INFO);
                    break;
            }
            break;
        case P_INFO:
            switch(b_idx)
            {
                case B_B_BUTTON:
                    changePage(P_MAIN);
                    break;
            }
            break;
    }
    // debounce
    btn_state = 0;
}


void valueUpdatePeriodic()
{
    // Ran periodically at a refresh rate
    // Based on the current page, update all values
    // many will be from the can_data struct
    // possible to check if values are stale
    switch (p_idx)
    {
        case P_MAIN:
            /* SPEED */
            float speed = can_data.front_wheel_data.left_speed + 
                          ((float) (can_data.front_wheel_data.right_speed - 
                          can_data.front_wheel_data.left_speed)) / 2;
            // TODO: convert speed to MPH
            set_float(A_SPEED, NXT_TEXT, speed, 1);

            /* VOLTAGE */
            // TODO: get voltage
            set_float(A_VOLTAGE, NXT_TEXT, 0.00, 2);

            /* BATTERY */
            // TODO: get soc
            set_value(A_BATTERY, NXT_VALUE, 0);

            /* POWER */
            // TODO: get main status
            set_value(A_POWER, NXT_PICTURE, POWER_OFF_PIC);

            /* TV STATUS */
            // TODO: get tv stat
            set_value(A_TV_STATUS, NXT_PICTURE, TV_STAT_OFF_PIC);

            break;
        case P_INFO:
            break;
    }
    
}

void changePage(uint8_t new_page)
{
    if (new_page == p_idx) return;
    set_page(pages[new_page].name);
    p_idx = new_page;
    b_idx = 0;
}