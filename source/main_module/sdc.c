#include "main.h"
#include "pindefs.h"

#include "common/phal/gpio.h"
#include "common/can_library/faults_common.h"
#include "common/freertos/freertos.h"

typedef struct {
    fault_id_t fault_id;
    uint8_t mux_addr;
} sdc_node_t;

// mux address to indicate the node is inaccessible
static constexpr uint8_t SDC_UNREADABLE = 0xFF; 
static constexpr int NUM_SDC_NODES = 17;

// id by the SDC node number (1-17)
static const sdc_node_t SDC_NODE_LUT[NUM_SDC_NODES] = {
    {FAULT_ID_MAIN_MODULE_SDC1_IMD,  11},
    {FAULT_ID_MAIN_MODULE_SDC2_BMS,  10},
    {FAULT_ID_MAIN_MODULE_SDC3_BSPD,  9},
    {FAULT_ID_MAIN_MODULE_SDC4_MAIN_OK,  8},
    {FAULT_ID_MAIN_MODULE_SDC5_BOTS,  7},
    {FAULT_ID_MAIN_MODULE_SDC6_INERTIA,  6},
    {FAULT_ID_MAIN_MODULE_SDC7_COCKPIT_ESTOP,  5},
    {0,  SDC_UNREADABLE}, // SDC8 status cant be checked
    {FAULT_ID_MAIN_MODULE_SDC9_FRONT_INTERLOCK,  4},
    {FAULT_ID_MAIN_MODULE_SDC10_RIGHT_ESTOP, 12},
    {FAULT_ID_MAIN_MODULE_SDC11_LEFT_ESTOP, 3},
    {FAULT_ID_MAIN_MODULE_SDC12_MSD, 2},
    {FAULT_ID_MAIN_MODULE_SDC13_E_METER, 13},
    {0, SDC_UNREADABLE}, // SDC14 status cant be checked
    {FAULT_ID_MAIN_MODULE_SDC15_REAR_INTERLOCK, 1}, // ! schematic inconsitency, SDC15 is marked as SDC14
    {FAULT_ID_MAIN_MODULE_SDC16_TSMS, 14},
    {FAULT_ID_MAIN_MODULE_SDC17_AIR_M, 0}
};

void update_SDC() {
    static uint8_t sdc_poll_index = 0;
    const sdc_node_t *current_node = &SDC_NODE_LUT[sdc_poll_index];
    uint8_t mux_addr = current_node->mux_addr;
    bool is_node_open = false; // default to closed if unreadable
    
    if (mux_addr != SDC_UNREADABLE) {
        // Set mux control
        PHAL_writeGPIO(SDC_MUX_S0_PORT, SDC_MUX_S0_PIN, (mux_addr >> 0) & 0x1);
        PHAL_writeGPIO(SDC_MUX_S1_PORT, SDC_MUX_S1_PIN, (mux_addr >> 1) & 0x1);
        PHAL_writeGPIO(SDC_MUX_S2_PORT, SDC_MUX_S2_PIN, (mux_addr >> 2) & 0x1);
        PHAL_writeGPIO(SDC_MUX_S3_PORT, SDC_MUX_S3_PIN, (mux_addr >> 3) & 0x1);
        
        // delay to allow mux signals to stabilize
        osDelay(1);

        // ! reading the input pin as 1 = closed, 0 = open
        is_node_open = !PHAL_readGPIO(SDC_MUX_PORT, SDC_MUX_PIN);
    }
    
    // Read the signal and update the relevant fault state
    update_fault(current_node->fault_id, is_node_open);

    // update the poll id for the next cycle (0-16 = sdc 1-17)
    sdc_poll_index++;
    if (sdc_poll_index >= NUM_SDC_NODES) sdc_poll_index = 0;
}