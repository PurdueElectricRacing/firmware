#include "bms.h"
#include "can_parse.h"

/**
 * Code for Precharge to combine the BMS remote's data into a pack level model.  
 * 
 */

volatile uint16_t cell_volts[NUM_CELLS] = {0};
static uint16_t bms_error_flags = 0;
extern q_handle_t q_tx_can;

uint16_t updateBMSErrorFlags()
{
    bms_error_flags |= 
                 can_data.pack_info_1.error | can_data.pack_info_2.error 
               | can_data.pack_info_3.error | can_data.pack_info_4.error
               | can_data.pack_info_5.error | can_data.pack_info_6.error
               | can_data.pack_info_7.error | can_data.pack_info_8.error;

    return bms_error_flags;
}


void txBatteryStatus()
{
    static uint8_t state = 0;

    uint16_t pack_voltage = 0;
    bool done_with_cells = false;
    uint8_t idx;
    uint16_t v1 = 0, v2 = 0, v3 = 0;

    switch (state)
    {
    case 0:
        for(int i = 0; i < NUM_CELLS; i++)
            pack_voltage += cell_volts[i];

        SEND_BATTERY_INFO(q_tx_can, pack_voltage, updateBMSErrorFlags());
        state ++;
        break;
    
    default:
        idx = (state - 1) * 3;

        v1 = cell_volts[idx + 0];
        if (idx + 1 < NUM_CELLS)
            v2 = cell_volts[idx + 1];
        else
            done_with_cells = true;
        
        if (idx + 1 < NUM_CELLS)
            v3 = cell_volts[idx + 2];
        else
            done_with_cells = true;

        SEND_CELL_INFO(q_tx_can, idx, v1, v2, v3);
        state ++;
        if (done_with_cells)
            state = 0;
        break;
    }
}


#define MAKE_VOLTS_CELLS_CALLBACK(fname, msg_name, cell_offset) \
void fname (CanParsedData_t* msg_data_a) { \
    uint16_t idx = msg_data_a->msg_name.idx; \
    cell_volts[cell_offset + (idx * 3 + 0)] = msg_data_a->msg_name.v1; \
    if (idx < 2) {\
        cell_volts[cell_offset + (idx * 3 + 1)] = msg_data_a->msg_name.v2; \
        cell_volts[cell_offset + (idx * 3 + 2)] = msg_data_a->msg_name.v3; \
    }\
}\

MAKE_VOLTS_CELLS_CALLBACK(volts_cells_1_CALLBACK, volts_cells_1, 0)
MAKE_VOLTS_CELLS_CALLBACK(volts_cells_2_CALLBACK, volts_cells_2, 10)
MAKE_VOLTS_CELLS_CALLBACK(volts_cells_3_CALLBACK, volts_cells_3, 20)
MAKE_VOLTS_CELLS_CALLBACK(volts_cells_4_CALLBACK, volts_cells_4, 30)
MAKE_VOLTS_CELLS_CALLBACK(volts_cells_5_CALLBACK, volts_cells_5, 40)
MAKE_VOLTS_CELLS_CALLBACK(volts_cells_6_CALLBACK, volts_cells_6, 50)
MAKE_VOLTS_CELLS_CALLBACK(volts_cells_7_CALLBACK, volts_cells_7, 60)
MAKE_VOLTS_CELLS_CALLBACK(volts_cells_8_CALLBACK, volts_cells_8, 70)