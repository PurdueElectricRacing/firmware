#include "bms.h"

/**
 * Code for Precharge to combine the BMS remote's data into a pack level model.  
 * 
 */

volatile uint16_t cell_volts[20 * 4];
void volts_cells_1_CALLBACK(CanParsedData_t* msg_data_a)
{
    uint16_t idx = msg_data_a->volts_cells_1.idx;
    cell_volts[idx * 3 + 0] = msg_data_a->volts_cells_1.v1;
    cell_volts[idx * 3 + 1] = msg_data_a->volts_cells_1.v2;
    cell_volts[idx * 3 + 2] = msg_data_a->volts_cells_1.v3;
}