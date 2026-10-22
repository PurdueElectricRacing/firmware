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
    SWITCH_FAN4 = 0,
    SWITCH_SDC,
    SWITCH_DASH,
    SWITCH_FAN5,
    SWITCH_ABOX,
    SWITCH_PUMP2,
    SWITCH_DL_FRONT,
    SWITCH_TV,
    SWITCH_SDC_LOOP,
    SWITCH_HXFAN,
    SWITCH_PUMP1,
    SWITCH_FAN1,
    SWITCH_DAQ,
    SWITCH_MAIN,
    SWITCH_EMETER,
    SWITCH_DL_REAR,
    SWITCH_FAN3,
    SWITCH_FAN2,
    SWITCH_AMK2,
    SWITCH_AMK1,
    SWITCH_HV_BOARD,
    NUM_SWITCHES
} switch_id_t;

typedef enum {
    ANALOG_MUX1 = 0,
    ANALOG_MUX2,
    NO_MUX
} mux_id_t;

typedef struct {
    float current_ma;
    GPIO_TypeDef* ctrl_port;
    uint8_t ctrl_pin;
    mux_id_t mux_id;
    uint8_t mux_addr;
} switch_t;

extern switch_t g_switches[NUM_SWITCHES];

#endif // SWITCH_H