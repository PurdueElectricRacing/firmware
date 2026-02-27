#include "main.h"
#include "pindefs.h"

#include "common/phal/gpio.h"
#include "common/can_library/faults_common.h"
#include "common/freertos/freertos.h"

SDC_states_t g_SDC_states;

static const fault_index_t SDC_FAULT_LUT[NUM_SDC_NODES] = {
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
    FAULT_INDEX_MAIN_MODULE_SDC16_OPEN,
    FAULT_INDEX_MAIN_MODULE_SDC17_OPEN
};

void update_SDC() {
    // SDC17 (precharge) is checked sperately
    bool is_precharge_open = PHAL_readGPIO(PRECHARGE_COMPLETE_PORT, PRECHARGE_COMPLETE_PIN);
    g_SDC_states[PRECHARGE_SDC_INDEX] = is_precharge_open;
    update_fault(FAULT_INDEX_MAIN_MODULE_SDC17_OPEN, is_precharge_open);

    // check SDC state by cycling through the mux and checking the input
    static uint8_t sdc_poll_index = 0;

    // Set mux control and delay to allow mux signals to stabilize
    PHAL_writeGPIO(SDC_MUX_S0_PORT, SDC_MUX_S0_PIN, (sdc_poll_index >> 0) & 0x1);
    PHAL_writeGPIO(SDC_MUX_S1_PORT, SDC_MUX_S1_PIN, (sdc_poll_index >> 1) & 0x1);
    PHAL_writeGPIO(SDC_MUX_S2_PORT, SDC_MUX_S2_PIN, (sdc_poll_index >> 2) & 0x1);
    PHAL_writeGPIO(SDC_MUX_S3_PORT, SDC_MUX_S3_PIN, (sdc_poll_index >> 3) & 0x1);
    osDelay(1);
    
    // Read the signal and update the relevant fault state
    // todo is 1 open or closed?
    bool is_node_open = PHAL_readGPIO(SDC_MUX_PORT, SDC_MUX_PIN);
    g_SDC_states[sdc_poll_index] = is_node_open;
    update_fault(SDC_FAULT_LUT[sdc_poll_index], is_node_open);

    // update the poll index for the next cycle (0-15)
    sdc_poll_index = (sdc_poll_index + 1) & 0xF;
}