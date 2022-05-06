#include "bms.h"
#include "can_parse.h"
#include "daq.h"
#include "common_defs.h"

/**
 * Code for Precharge to combine the BMS remote's data into a pack level model.  
 * 
 */

uint16_t cell_volts[NUM_CELLS] = {0};
extern q_handle_t q_tx_can;

bool charge_mode_enable = false; // Enable charge algo
uint16_t charge_voltage_limit = 0;
uint16_t charge_current_limit = 0;

void BMS_init()
{
    linkReada(DAQ_ID_CHARGE_MODE_ENABLE, &charge_mode_enable);
    linkWritea(DAQ_ID_CHARGE_MODE_ENABLE, &charge_mode_enable);
    linkReada(DAQ_ID_CHARGE_VOLTAGE_LIMIT, &charge_voltage_limit);
    linkWritea(DAQ_ID_CHARGE_VOLTAGE_LIMIT, &charge_voltage_limit);
    linkReada(DAQ_ID_CHARGE_CURRENT_LIMIT, &charge_current_limit);
    linkWritea(DAQ_ID_CHARGE_CURRENT_LIMIT, &charge_current_limit);
}


uint16_t BMS_updateErrorFlags()
{
    static uint16_t bms_error_flags = 0;
    bms_error_flags |= 
                 can_data.pack_info_1.error | can_data.pack_info_2.error 
               | can_data.pack_info_3.error | can_data.pack_info_4.error
               | can_data.pack_info_5.error | can_data.pack_info_6.error
               | can_data.pack_info_7.error | can_data.pack_info_8.error;

    return bms_error_flags;
}


void BMS_txBatteryStatus()
{
    static uint8_t state = 0;

    uint16_t pack_voltage = 0;
    uint8_t idx;
    uint16_t v1 = 0, v2 = 0, v3 = 0;

    switch (state)
    {
    case 0:
        for(int i = 0; i < NUM_CELLS; i++)
            pack_voltage += cell_volts[i];

        SEND_BATTERY_INFO(q_tx_can, pack_voltage, BMS_updateErrorFlags());
        state ++;
        break;
    
    default:
        idx = (state - 1) * 3;

        v1 = cell_volts[idx + 0];
        if (idx + 1 < NUM_CELLS)
            v2 = cell_volts[idx + 1];
        
        if (idx + 2 < NUM_CELLS)
            v3 = cell_volts[idx + 2];

        SEND_CELL_INFO(q_tx_can, idx, v1, v2, v3);
        state ++;
        if (idx + 2 >= NUM_CELLS)
            state = 0;
        break;
    }
}

static void findGlobalImbalance(uint16_t* lowest, uint16_t* delta)
{
    *lowest = cell_volts[0];
    *delta = 0;
    for (uint16_t* cell = cell_volts; cell <= cell_volts+NUM_CELLS; cell++)
    {
        if (*lowest > *cell)
            *lowest = *cell;
        // Lowest should be lower than whatever cell is, safe subrtaction
        if (*cell - *lowest > *delta)
            *delta = *cell - *lowest;
    }
}

/**
 * @brief 
 * 
 * 1. Global balance to lowest cell
 * 2. Wait for global balance
 */

void BMS_chargePeriodic()
{
    bool charge_power_enable = false; // Allow power from elcon
    static bool cells_balanced_for_charge = false;
    uint16_t charge_voltage_req = charge_voltage_limit;
    uint16_t charge_current_req = charge_current_limit;

    // Ensure that we only charge if we do not have a latched error
    charge_mode_enable &= (BMS_updateErrorFlags() == 0);
    
    if(charge_mode_enable)
    {
        // Check that all of the cells are globally balanced
        uint16_t lowest, delta;
        findGlobalImbalance(&lowest, &delta);

        // Request balance to battery modules if delta is large enough        
        if (cells_balanced_for_charge)
        {
            // Hysteresis for if delta gets too large during charge
            if (delta > CHARGE_DELTA_MAXIMUM_V * 15000) 
                cells_balanced_for_charge = false;
        }
        else if (delta < CHARGE_DELTA_MAXIMUM_V * 10000)
        {
            // Start pushing power, cells are close enough to charge
            cells_balanced_for_charge = true;
        }

        if (can_data.elcon_charger_status.charge_current < 5) // 500mA global
        {
            // Basiacally done charging, try balancing to the mV level
            if (delta > BALANCE_DELTA_MINIMUM_V * 10000)
                SEND_BALANCE_REQUEST(q_tx_can, lowest);
            else
            {
                // Done charging!
                charge_mode_enable = false;
            }
        }

        // Only allow power if global delta is less than the configured maximum
        charge_power_enable = cells_balanced_for_charge;
    }
    else
    {
        cells_balanced_for_charge = false;
    }

    // Variable sanity check
    charge_voltage_req      = MIN(charge_voltage_req, 42 * 80); // Hard limit, dont overcharge!
    SEND_ELCON_CHARGER_COMMAND(q_tx_can, charge_voltage_req, charge_current_req, !charge_power_enable);
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