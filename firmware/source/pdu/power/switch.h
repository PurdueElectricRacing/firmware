/**
 * @file switch.h
 * @brief switch mappings and control code
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#ifndef SWITCH_H
#define SWITCH_H

#include "common/phal_G4/gpio/gpio.h"

typedef enum {
    SWITCH_SDC = 0,
    SWITCH_SDC_LOOP,
    SWITCH_DISCHARGE,
    SWITCH_TV,
    SWITCH_EMETER,
    SWITCH_ABOX,
    SWITCH_DASH,
    SWITCH_DL_REAR,
    SWITCH_DL_FRONT,
    SWITCH_MAIN,
    SWITCH_DAQ,
    SWITCH_HXFAN,
    SWITCH_FAN5,
    SWITCH_FAN4,
    SWITCH_FAN3,
    SWITCH_FAN2,
    SWITCH_FAN1,
    SWITCH_AMK1,
    SWITCH_AMK2,
    SWITCH_PUMP1,
    SWITCH_PUMP2,
    NUM_SWITCHES
} switch_id_t;

typedef enum {
    ANALOG_MUX1 = 0,
    ANALOG_MUX2,
} mux_id_t;

typedef struct {
    GPIO_TypeDef* ctrl_port;
    uint8_t ctrl_pin;
    mux_id_t mux_id;
    uint8_t mux_addr;
    uint32_t current_ma;
} switch_t;

extern switch_t switches[NUM_SWITCHES];

#endif // SWITCH_H