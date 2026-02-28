#include "main.h"
#include "pindefs.h"

#include "common/phal/gpio.h"
#include "common/can_library/faults_common.h"
#include "common/freertos/freertos.h"

typedef struct {
    fault_index_t fault_index;
    uint8_t mux_addr;
} sdc_node_t;

// mux address to indicate the node is inacessible
static constexpr uint8_t SDC_UNREADABLE = 0xFF; 

// index by the SDC node number (1-17)
static const sdc_node_t SDC_NODE_LUT[NUM_SDC_NODES] = {
    {FAULT_INDEX_MAIN_MODULE_SDC1_OPEN,  11},
    {FAULT_INDEX_MAIN_MODULE_SDC2_OPEN,  10},
    {FAULT_INDEX_MAIN_MODULE_SDC3_OPEN,  9},
    {FAULT_INDEX_MAIN_MODULE_SDC4_OPEN,  8},
    {FAULT_INDEX_MAIN_MODULE_SDC5_OPEN,  7},
    {FAULT_INDEX_MAIN_MODULE_SDC6_OPEN,  6},
    {FAULT_INDEX_MAIN_MODULE_SDC7_OPEN,  5},
    {NULL,  SDC_UNREADABLE}, // SDC8 status cant be checked
    {FAULT_INDEX_MAIN_MODULE_SDC9_OPEN,  4},
    {FAULT_INDEX_MAIN_MODULE_SDC10_OPEN, 12},
    {FAULT_INDEX_MAIN_MODULE_SDC11_OPEN, 3},
    {FAULT_INDEX_MAIN_MODULE_SDC12_OPEN, 2},
    {FAULT_INDEX_MAIN_MODULE_SDC13_OPEN, 13},
    {NULL, SDC_UNREADABLE}, // SDC14 status cant be checked
    {FAULT_INDEX_MAIN_MODULE_SDC15_OPEN, 1}, // ! schematic inconsitency, SDC15 is marked as SDC14
    {FAULT_INDEX_MAIN_MODULE_SDC16_OPEN, 14},
    {FAULT_INDEX_MAIN_MODULE_SDC17_OPEN, 0} // precharge, checked separately
};

// todo: is SDC17 the same as precharge?

void update_SDC() {
    // ! reading the input pin as 1 = closed, 0 = open

    // SDC17 (precharge) is checked on it's own pin
    bool is_precharge_open = !PHAL_readGPIO(PRECHARGE_COMPLETE_PORT, PRECHARGE_COMPLETE_PIN);
    g_SDC_open_nodes[PRECHARGE_SDC_INDEX] = is_precharge_open;
    update_fault(FAULT_INDEX_MAIN_MODULE_SDC17_OPEN, is_precharge_open);

    // check SDC state by cycling through the mux and checking the input
    static uint8_t sdc_poll_index = 0;
    const sdc_node_t *current_node = &SDC_NODE_LUT[sdc_poll_index];
    uint8_t mux_addr = current_node->mux_addr;
    bool is_node_open = false; // default to closed if the mux address is invalidsdc_node_t
    
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
    update_fault(current_node->fault_index, is_node_open);

    // update the poll index for the next cycle (mux 0-15 = sdc 1-16)
    sdc_poll_index = (sdc_poll_index + 1) & 0xF;
}