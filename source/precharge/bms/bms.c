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

static void findGlobalImbalance(uint16_t* lowest, uint16_t* delta, uint16_t* pack_voltage);

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
    uint16_t delta = 0;
    uint16_t lowest = 0;
    uint8_t idx;
    uint16_t v1 = 0, v2 = 0, v3 = 0;

    switch (state)
    {
    case 0:
        findGlobalImbalance(&lowest, &delta, &pack_voltage);
        SEND_BATTERY_INFO(q_tx_can, pack_voltage, delta, lowest, BMS_updateErrorFlags());
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

static void findGlobalImbalance(uint16_t* lowest, uint16_t* delta, uint16_t* pack_voltage)
{
    *lowest = cell_volts[0];
    *delta = 0;
    *pack_voltage = 0;
    for (uint16_t* cell = cell_volts; cell <= cell_volts+NUM_CELLS; cell++)
    {
        *pack_voltage += * cell;
        if (*lowest > *cell)
            *lowest = *cell;
        // Lowest should be lower than whatever cell is, safe subrtaction
        if (*cell - *lowest > *delta)
            *delta = *cell - *lowest;
    }
}

/**
 * @brief BMS_chargePeriodic
 * 
 * 1. Global balance to lowest cell to CHARGE_DELTA_MAXIMUM_V
 * 2. Wait for global balance
 * 3. Start charging
 * 4. If delta is greater than CHARGE_DELTA_MAXIMUM_V * 1.2 (hysteresis) stop charging and go to 1.
 * 5. If charge current is low and delta is less than BALANCE_DELTA_MINIMUM_V, finish charging
 */
void BMS_chargePeriodic()
{

    bool charge_power_enable = false;                   // Allow power from elcon
    bool balance_req         = false;                   // Sending balance request to the modules
    static bool cells_balanced_for_charge = false;      // Cells are ready to charge
    uint16_t charge_voltage_req = charge_voltage_limit; // Voltage limit request to send to charger
    uint16_t charge_current_req = charge_current_limit; // Current limit request

    // Ensure that we only charge if:
    // 1. we do not have a BMS error
    // 2. Elcon charger is connected.
    charge_mode_enable &= (BMS_updateErrorFlags() == 0) && !can_data.elcon_charger_status.stale;
    
    if(charge_mode_enable)
    {
        // Check that all of the cells are globally balanced
        uint16_t lowest, delta, pack_voltage;
        findGlobalImbalance(&lowest, &delta, &pack_voltage);

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
        else
        {
            cells_balanced_for_charge = false;
        }

        if (can_data.elcon_charger_status.charge_current < 5) // 500mA pack charge current, super low
        {
            // Basiacally done charging, try balancing to the mV level
            if (delta > BALANCE_DELTA_MINIMUM_V * 10000)
            {
                SEND_BALANCE_REQUEST(q_tx_can, lowest);
                balance_req = true;
            }
            else
            {
                // Done charging!
                charge_mode_enable = false;
            }
        }

        // Only allow power if global delta is less than the configured maximum
        charge_power_enable = cells_balanced_for_charge;
    }

    charge_voltage_req      = MIN(charge_voltage_req, 42 * 80); // Hard limit, don't overcharge!
    SEND_ELCON_CHARGER_COMMAND(q_tx_can, charge_voltage_req, charge_current_req, !charge_power_enable);
    
    float power = (can_data.elcon_charger_status.charge_current / 10.0f) * (can_data.elcon_charger_status.charge_voltage / 10.0f);
    SEND_PACK_CHARGE_STATUS(q_tx_can, (uint16_t) (power), charge_power_enable, balance_req);
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