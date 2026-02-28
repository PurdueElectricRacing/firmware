#include "main.h"
#include "pindefs.h"

#include "common/phal/gpio.h"
#include "common/can_library/faults_common.h"
#include "common/freertos/freertos.h"

static const fault_index_t SDC_FAULT_LUT[NUM_SDC_NODES - 1] = {
    FAULT_INDEX_MAIN_MODULE_SDC1_OPEN,
    FAULT_INDEX_MAIN_MODULE_SDC2_OPEN,
    FAULT_INDEX_MAIN_MODULE_SDC3_OPEN,
    FAULT_INDEX_MAIN_MODULE_SDC4_OPEN,
    FAULT_INDEX_MAIN_MODULE_SDC5_OPEN,
    FAULT_INDEX_MAIN_MODULE_SDC6_OPEN,
    FAULT_INDEX_MAIN_MODULE_SDC7_OPEN,
    FAULT_INDEX_MAIN_MODULE_SDC8_OPEN,
    FAULT_INDEX_MAIN_MODULE_SDC9_OPEN,
    FAULT_INDEX_MAIN_MODULE_SDC10_OPEN,
    FAULT_INDEX_MAIN_MODULE_SDC11_OPEN,
    FAULT_INDEX_MAIN_MODULE_SDC12_OPEN,
    FAULT_INDEX_MAIN_MODULE_SDC13_OPEN,
    FAULT_INDEX_MAIN_MODULE_SDC14_OPEN,
    FAULT_INDEX_MAIN_MODULE_SDC15_OPEN,
    FAULT_INDEX_MAIN_MODULE_SDC16_OPEN
    // SDC17 is handled seperately
};

static constexpr uint8_t SDC_UNREADABLE = 0xFF;
static const uint8_t NODE_TO_MUX_LUT[NUM_SDC_NODES] = {
    11,             // SDC1
    10,             // SDC2
    9,              // SDC3
    5,              // SDC4
    7,              // SDC5
    6,              // SDC6
    8,              // SDC7
    4,              // SDC8
    12,             // SDC9
    3,              // SDC10
    2,              // SDC11
    13,             // SDC12
    SDC_UNREADABLE, // SDC13
    1,              // SDC14
    SDC_UNREADABLE, // SDC15
    14,             // SDC16
    0               // SDC17 (precharge)
};

void update_SDC() {
    // ! reading the input pin as 1 = closed, 0 = open

    // SDC17 (precharge) is checked on it's own pin
    bool is_precharge_open = !PHAL_readGPIO(PRECHARGE_COMPLETE_PORT, PRECHARGE_COMPLETE_PIN);
    g_SDC_open_nodes[PRECHARGE_SDC_INDEX] = is_precharge_open;
    update_fault(FAULT_INDEX_MAIN_MODULE_SDC17_OPEN, is_precharge_open);

    // check SDC state by cycling through the mux and checking the input
    static uint8_t sdc_poll_index = 0;
    uint8_t mux_addr = NODE_TO_MUX_LUT[sdc_poll_index];
    bool is_node_open = false; // default to closed if the mux address is invalid
    
    if (mux_addr != SDC_UNREADABLE) {
        // Set mux control
        PHAL_writeGPIO(SDC_MUX_S0_PORT, SDC_MUX_S0_PIN, (mux_addr >> 0) & 0x1);
        PHAL_writeGPIO(SDC_MUX_S1_PORT, SDC_MUX_S1_PIN, (mux_addr >> 1) & 0x1);
        PHAL_writeGPIO(SDC_MUX_S2_PORT, SDC_MUX_S2_PIN, (mux_addr >> 2) & 0x1);
        PHAL_writeGPIO(SDC_MUX_S3_PORT, SDC_MUX_S3_PIN, (mux_addr >> 3) & 0x1);
        
        // delay to allow mux signals to stabilize
        osDelay(1);

        is_node_open = !PHAL_readGPIO(SDC_MUX_PORT, SDC_MUX_PIN);
    }
    
    // Read the signal and update the relevant fault state
    g_SDC_open_nodes[sdc_poll_index] = is_node_open;
    update_fault(SDC_FAULT_LUT[sdc_poll_index], is_node_open);

    // update the poll index for the next cycle (mux 0-15 = sdc 1-16)
    sdc_poll_index = (sdc_poll_index + 1) & 0xF;
}