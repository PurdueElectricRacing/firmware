/**
 * @file vcu.c
 * @brief VCU page implementation
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "vcu.h"
#include "common/utils/countof.h"
#include "can_library/generated/DASHBOARD.h"

#include "menu_system.h"

void send_vcu_driver_request(void);

typedef enum {
    VCU_MODE_INDEX     = 0,
    LATERAL_GAIN_INDEX = 1,
    LONG_GAIN_INDEX    = 2,
    EBB_INDEX          = 3,
    REGEN_INDEX        = 4,
    TV_INDEX           = 5,
    LEFT_WHEEL_INDEX   = 6,
    RIGHT_WHEEL_INDEX  = 7,
    NUM_VCU_ELEMENTS
} vcu_elements_t;

menu_element_t vcu_elements[NUM_VCU_ELEMENTS] = {
    [VCU_MODE_INDEX] = {
        .type        = ELEMENT_OPTION,
        .object_name = VCU_MODE_BUTTON,
        .current_value = VCU_MODE_ACCEL,
        .increment = 1,
        .min_value = VCU_MODE_ACCEL,
        .max_value = VCU_MODE_ENDURANCE,
        .on_change   = send_vcu_driver_request
    },
    [LATERAL_GAIN_INDEX] = {
        .type        = ELEMENT_VAL,
        .object_name = LATERAL_GAIN_BUTTON,
        .increment = 1,
        .min_value = 0,
        .max_value = 100,
        .on_change   = send_vcu_driver_request
    },
    [LONG_GAIN_INDEX] = {
        .type        = ELEMENT_VAL,
        .object_name = LONG_GAIN_BUTTON,
        .increment = 1,
        .min_value = 0,
        .max_value = 100,
        .on_change   = send_vcu_driver_request
    },
    [EBB_INDEX] = {
        .type        = ELEMENT_VAL,
        .object_name = EBB_BUTTON,
        .increment = 1,
        .min_value = 0,
        .max_value = 100,
        .on_change   = send_vcu_driver_request
    },
    [REGEN_INDEX] = {
        .type        = ELEMENT_BUTTON,
        .object_name = REGEN_BUTTON,
        .increment = 1,
        .min_value = 0,
        .max_value = 1,
        .on_change   = send_vcu_driver_request
    },
    [TV_INDEX] = {
        .type        = ELEMENT_BUTTON,
        .object_name = TV_BUTTON,
        .on_change   = send_vcu_driver_request
    },
    [LEFT_WHEEL_INDEX] = {
        .type        = ELEMENT_OPTION,
        .object_name = LEFT_WHEEL_BUTTON,
        .on_change   = send_vcu_driver_request
    },
    [RIGHT_WHEEL_INDEX] = {
        .type        = ELEMENT_OPTION,
        .object_name = RIGHT_WHEEL_BUTTON,
        .on_change   = send_vcu_driver_request
    }
};

menu_page_t vcu_page = {
    .elements            = vcu_elements,
    .num_elements        = countof(vcu_elements),
    .current_index       = 0,
    .is_element_selected = false
};

void vcu_update() {
    // force sync with tv's last reported state
    vcu_elements[VCU_MODE_INDEX].current_value = can_data.vcu_settings.vcu_mode;
    vcu_elements[LATERAL_GAIN_INDEX].current_value = can_data.vcu_settings.lateral_gain;
    vcu_elements[LONG_GAIN_INDEX].current_value = can_data.vcu_settings.longitudinal_gain;
    vcu_elements[EBB_INDEX].current_value = can_data.vcu_settings.electronic_brake_bias;
    vcu_elements[REGEN_INDEX].current_value = can_data.vcu_settings.is_regen_enabled;

    MS_refreshPage(&vcu_page);
}

void vcu_move_up() {
    MS_moveUp(&vcu_page);
}

void vcu_move_down() {
    MS_moveDown(&vcu_page);
}

void vcu_select() {
    MS_select(&vcu_page);
}

void send_vcu_driver_request(void) {
    CAN_SEND_vcu_driver_request(
        can_data.vcu_settings.vcu_mode,
        can_data.vcu_settings.lateral_gain,
        can_data.vcu_settings.longitudinal_gain,
        can_data.vcu_settings.electronic_brake_bias,
        can_data.vcu_settings.is_regen_enabled
    );
}