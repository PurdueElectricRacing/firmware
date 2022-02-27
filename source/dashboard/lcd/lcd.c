#include "lcd.h"

typedef enum {
    J_UP,
    J_RIGHT,
    J_DOWN,
    J_LEFT,
    J_CENTER
} joystick_dir_t;

// button press only registered when joystick is centered
joystick_dir_t j_dir = J_CENTER;
uint8_t btn_state = 0; // activate only on falling edge

button_t main_buttons[] = {
    {.name="tcButton", .norm_id=BTN_NORM_ID, .high_id=BTN_HIGH_ID,
     .dirs={B_SETTINGS, B_DIAG_BUTTON, B_SETTINGS, B_START_BUTTON}},
    {.name="diagButton", .norm_id=BTN_NORM_ID, .high_id=BTN_HIGH_ID,
     .dirs={B_SETTINGS, B_LAPS_BUTTON, B_SETTINGS, B_TC_BUTTON}},
    {.name="lapsButton", .norm_id=BTN_NORM_ID, .high_id=BTN_HIGH_ID,
     .dirs={B_SETTINGS, B_LAPS_BUTTON, B_SETTINGS, B_TC_BUTTON}},
    {.name="startButton", .norm_id=BTN_NORM_ID, .high_id=BTN_HIGH_ID,
    .dirs={B_SETTINGS, B_TC_BUTTON, B_SETTINGS, B_LAPS_BUTTON}}, 
    {.name="settings", .norm_id=BTN_NORM_ID, .high_id=BTN_HIGH_ID,
     .dirs={B_START_BUTTON, B_SETTINGS, B_TC_BUTTON, B_SETTINGS}}
};

attribute_t main_attributes[] =   {
        {.name="speed", .type=A_VALUE,
                        .val_addr=&can_data.main_status.car_state, 
                        .val_size=sizeof(can_data.main_status.car_state)},
        {.name="voltage", .type=A_VALUE,
                          .val_addr=&can_data.main_status.car_state, 
                          .val_size=sizeof(can_data.main_status.car_state)},
        {.name="statusLabel", .type=A_LABEL},
};

page_t pages[P_TOTAL] = {
    {.name="splash", .attributes=0, .num_attributes=0},
    {.name="main", .attributes=main_attributes, .num_attributes=A_MAIN_TOTAL,
                   .buttons=main_buttons, .num_buttons=B_MAIN_TOTAL},
};

uint8_t p_idx = 0;
uint8_t b_idx = 0; 

void joystick_update()
{
    // Based on direction, updated the selected element and highlight / un-highlight accordingly
    // posibly have timeout for the highlight
    // may have to read from queue if interrupt based
    // joystick would be on interrupt

    if (j_dir == J_CENTER || !pages[p_idx].buttons) return;

    // determine new selection
    uint8_t new_selection = pages[p_idx].buttons[b_idx].dirs[j_dir];
    // debounce
    j_dir = J_CENTER;

    // update selection if changed
    if (new_selection != b_idx) 
    {
        set_pic(pages[p_idx].buttons[b_idx].name, 
                pages[p_idx].buttons[b_idx].norm_id);
        set_pic(pages[p_idx].buttons[new_selection].name, 
                pages[p_idx].buttons[new_selection].high_id);
        b_idx = new_selection;
        // dont unintentionally select something
        btn_state = 0;
    }

}

void action_update()
{
    // Based on current selection, run the action associated to the element

    if (!btn_state) return;
    switch (p_idx)
    {
        case P_MAIN: // main
            switch(s_idx)
            {
                case A_DIAG_BUTTON:
                    change_page(P_INFO);
                    break;
                case A_SETTINGS:
                    change_page(P_SETTINGS);
                    break;
            }
            break;
    }

    // debounce
    btn_state = 0;
}


void value_update()
{
    // Ran periodically at a refresh rate
    // Based on the current page, update all values
    // many will be from the can_data struct
    // possible to check if values are stale
    for (uint8_t i = 0; i < pages[p_idx].num_attributes; i++)
    {
        attribute_t a = pages[p_idx].attributes[i];
        uint16_t curr;
        switch (a.type)
        {
            case A_VALUE:
                if (a.val_size == 1) curr = *((uint8_t *) a.val_addr);
                else curr = *((uint16_t *) a.val_addr);
                if (curr != a.last_val) 
                {
                    set_value(a.name, curr);
                    a.last_val = curr;
                }
                break;
        }
    }
}

void change_page(uint8_t new_page)
{
    if (new_page == p_idx) return;
    set_page(pages[new_page].name);
    p_idx = new_page;
    b_idx = 0;
}