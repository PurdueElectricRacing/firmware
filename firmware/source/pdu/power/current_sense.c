/**
 * @file current_sense.c
 * @brief current sensing
 *
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "switch.h"
#include "main.h"
#include "common/rtos/rtos.h"

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