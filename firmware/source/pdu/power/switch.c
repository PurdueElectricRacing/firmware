/**
 * @file switch.c
 * @brief switch mappings and control code
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "switch.h"
#include "main.h"
#include "common/rtos/rtos.h"

switch_t switches[NUM_SWITCHES] = {
    [SWITCH_SDC]       = {.ctrl_port = CTRL1_PORT, .ctrl_pin = CTRL1_PIN, .mux_id = ANALOG_MUX1, .mux_addr = 0, .current_ma = 0},
    [SWITCH_SDC_LOOP]  = {.ctrl_port = CTRL2_PORT, .ctrl_pin = CTRL2_PIN, .mux_id = ANALOG_MUX1, .mux_addr = 1, .current_ma = 0},
    [SWITCH_DISCHARGE] = {.ctrl_port = CTRL3_PORT, .ctrl_pin = CTRL3_PIN, .mux_id = ANALOG_MUX1, .mux_addr = 2, .current_ma = 0},
    [SWITCH_TV]        = {.ctrl_port = CTRL4_PORT, .ctrl_pin = CTRL4_PIN, .mux_id = ANALOG_MUX1, .mux_addr = 3, .current_ma = 0},
    [SWITCH_EMETER]    = {.ctrl_port = CTRL5_PORT, .ctrl_pin = CTRL5_PIN, .mux_id = ANALOG_MUX1, .mux_addr = 4, .current_ma = 0},
    [SWITCH_ABOX]      = {.ctrl_port = CTRL6_PORT, .ctrl_pin = CTRL6_PIN, .mux_id = ANALOG_MUX2, .mux_addr = 0, .current_ma = 0},
    [SWITCH_DASH]      = {.ctrl_port = CTRL7_PORT, .ctrl_pin = CTRL7_PIN, .mux_id = ANALOG_MUX2, .mux_addr = 1, .current_ma = 0},
    [SWITCH_DL_REAR]   = {.ctrl_port = CTRL8_PORT, .ctrl_pin = CTRL8_PIN, .mux_id = ANALOG_MUX2, .mux_addr = 2, .current_ma = 0},
    [SWITCH_DL_FRONT]  = {.ctrl_port = CTRL9_PORT, .ctrl_pin = CTRL9_PIN, .mux_id = ANALOG_MUX2, .mux_addr = 3, .current_ma = 0},
    [SWITCH_MAIN]      = {.ctrl_port = CTRL10_PORT, .ctrl_pin = CTRL10_PIN, .mux_id = ANALOG_MUX2, .mux_addr = 4, .current_ma = 0},
    [SWITCH_DAQ]       = {.ctrl_port = CTRL11_PORT, .ctrl_pin = CTRL11_PIN, .mux_id = ANALOG_MUX2, .mux_addr = 5, .current_ma = 0},
    [SWITCH_HXFAN]     = {.ctrl_port = CTRL12_PORT, .ctrl_pin = CTRL12_PIN, .mux_id = ANALOG_MUX2, .mux_addr = 6, .current_ma = 0},
    [SWITCH_FAN5]      = {.ctrl_port = CTRL13_PORT, .ctrl_pin = CTRL13_PIN, .mux_id = ANALOG_MUX2, .mux_addr = 7, .current_ma = 0},
    [SWITCH_FAN4]      = {.ctrl_port = CTRL14_PORT, .ctrl_pin = CTRL14_PIN, .mux_id = ANALOG_MUX2, .mux_addr = 8, .current_ma = 0},
    [SWITCH_FAN3]      = {.ctrl_port = CTRL15_PORT, .ctrl_pin = CTRL15_PIN, .mux_id = ANALOG_MUX2, .mux_addr = 9, .current_ma = 0},
    [SWITCH_FAN2]      = {.ctrl_port = CTRL16_PORT, .ctrl_pin = CTRL16_PIN, .mux_id = ANALOG_MUX2, .mux_addr = 10, .current_ma = 0},
    [SWITCH_FAN1]      = {.ctrl_port = CTRL17_PORT, .ctrl_pin = CTRL17_PIN, .mux_id = ANALOG_MUX2, .mux_addr = 11, .current_ma = 0},
    [SWITCH_AMK1]      = {.ctrl_port = CTRL18_PORT, .ctrl_pin = CTRL18_PIN, .mux_id = ANALOG_MUX2, .mux_addr = 12, .current_ma = 0},
    [SWITCH_AMK2]      = {.ctrl_port = CTRL19_PORT, .ctrl_pin = CTRL19_PIN, .mux_id = ANALOG_MUX2, .mux_addr = 13, .current_ma = 0},
    [SWITCH_PUMP1]     = {.ctrl_port = CTRL20_PORT, .ctrl_pin = CTRL20_PIN, .mux_id = ANALOG_MUX2, .mux_addr = 14, .current_ma = 0},
    [SWITCH_PUMP2]     = {.ctrl_port = CTRL21_PORT, .ctrl_pin = CTRL21_PIN, .mux_id = ANALOG_MUX2, .mux_addr = 15, .current_ma = 0},
};

void mux_select(mux_id_t mux_id, uint8_t mux_addr) {
    switch(mux_id) {
        case ANALOG_MUX1:
            PHAL_GPIO_write(MUX1_CTRL_A_PORT, MUX1_CTRL_A_PIN, (mux_addr >> 0) & 0x1);
            PHAL_GPIO_write(MUX1_CTRL_B_PORT, MUX1_CTRL_B_PIN, (mux_addr >> 1) & 0x1);
            PHAL_GPIO_write(MUX1_CTRL_C_PORT, MUX1_CTRL_C_PIN, (mux_addr >> 2) & 0x1);
            break;
        case ANALOG_MUX2:
            PHAL_GPIO_write(MUX2_CTRL_A_PORT, MUX2_CTRL_A_PIN, (mux_addr >> 0) & 0x1);
            PHAL_GPIO_write(MUX2_CTRL_B_PORT, MUX2_CTRL_B_PIN, (mux_addr >> 1) & 0x1);
            PHAL_GPIO_write(MUX2_CTRL_C_PORT, MUX2_CTRL_C_PIN, (mux_addr >> 2) & 0x1);
            PHAL_GPIO_write(MUX2_CTRL_D_PORT, MUX2_CTRL_D_PIN, (mux_addr >> 3) & 0x1);
            break;
    }
}

void current_sense_periodic() {
    for (switch_id_t id = 0; id < NUM_SWITCHES; id++) {
        switch_t *sw = &switches[id];

        mux_select(sw->mux_id, sw->mux_addr);
        RTOS_delay_ms(1); // wait for mux to settle

        // todo resistor divider
        sw->current_ma = 0; // todo
    }
}

