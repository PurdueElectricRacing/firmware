#ifndef VCU_H
#define VCU_H

#include <stdint.h>

/**
 * @file VCU.h
 * @brief VCU page implementation
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#define VCU_STRING "vcu"

// Object names
#define VCU_MODE_BUTTON     "mode"
#define LATERAL_GAIN_BUTTON "lat"
#define LONG_GAIN_BUTTON    "long"
#define EBB_BUTTON          "ebb"
#define REGEN_BUTTON        "regen"
#define TV_BUTTON           "tv"
#define LEFT_WHEEL_BUTTON   "left"
#define RIGHT_WHEEL_BUTTON  "right"

typedef enum : uint8_t {
    VCU_BINDING_MODE = 0,
    VCU_BINDING_LATERAL_GAIN = 1,
    VCU_BINDING_LONGITUDINAL_GAIN = 2,
    VCU_BINDING_EBB = 3,
} vcu_binding_t;

void vcu_update(void);
void vcu_move_up(void);
void vcu_move_down(void);
void vcu_select(void);
void vcu_wheel_adjust(bool is_right_wheel, int8_t delta);
void vcu_toggle_regen(void);
void send_vcu_driver_request(void);

#endif // VCU_H
