#include "orion.h"

#include <stdbool.h>
#include "can_parse.h"
#include "common_defs.h"
#include "daq.h"


extern q_handle_t q_tx_can;

bool charge_request_user = false; // Enable charge algo
uint16_t orion_charge_current_limit = 0;
uint16_t orion_charge_voltage_limit = 0;
uint8_t  orion_bms_temp_err = 0;
uint16_t orion_uv_limit = 25000;
uint16_t orion_ov_limit = 42000;

void orionInit()
{
    linkReada(DAQ_ID_CHARGE_MODE_ENABLE, &charge_request_user);
    linkWritea(DAQ_ID_CHARGE_MODE_ENABLE, &charge_request_user);
    linkReada(DAQ_ID_CHARGE_VOLTAGE_LIMIT, &orion_charge_voltage_limit);
    linkWritea(DAQ_ID_CHARGE_VOLTAGE_LIMIT, &orion_charge_voltage_limit);
    linkReada(DAQ_ID_CHARGE_CURRENT_LIMIT, &orion_charge_current_limit);
    linkWritea(DAQ_ID_CHARGE_CURRENT_LIMIT, &orion_charge_current_limit);
    linkReada(DAQ_ID_UV_LIMIT, &orion_uv_limit);
    linkWritea(DAQ_ID_UV_LIMIT, &orion_uv_limit);
    linkReada(DAQ_ID_OV_LIMIT, &orion_ov_limit);
    linkWritea(DAQ_ID_OV_LIMIT, &orion_ov_limit);
}

//Now also accounts for temp errors
bool orionErrors() {
    return (((can_data.orion_info.dtc_status == 1) /*|| bms_temp_err*/) ? 0 : 1);
}

/*
    Key items that are excluded from the revised function
    1. findGlobalImbalance - Everything cell balancing related is controlled by Orion - we don't get any control?
    *This funciton is also covered by DTC flags, so no longer needed*
    2. Cell balancing stuff from bms - Orion handles the balancing
    3.


*/

void orion_chargePeriodic() {
    bool charge_power_enable = can_data.orion_info.charger_safety;
    bool elcon_charge_enable = false;                 // Allow power from elcon
    bool balance_req         = false;                   // Sending balance request to the modules
    static bool cells_balanced_for_charge = false;      // Cells are ready to charge
    uint16_t charge_voltage_req = 0; // Voltage limit request to send to charger
    uint16_t charge_current_req = 0; // Current limit request
    orion_charge_current_limit = can_data.orion_info.pack_ccl;
    uint16_t charge_current = 0;
    uint16_t charge_voltage = 0;
    float power = 0;


    //According to the Orion Wiring manual, this signal apparently pulls down to ground when on.
    //As such, do I invert the signal, or should it be as is? For now, I am leaving as is.
    charge_request_user &= !can_data.elcon_charger_status.stale;
    if (charge_request_user && charge_power_enable) {
            if (charge_current_req > orion_charge_current_limit) {
                charge_current_req = orion_charge_current_limit;
                elcon_charge_enable = true;
            }
            elcon_charge_enable &= !orionErrors();

            charge_voltage_req = orion_charge_voltage_limit;
            charge_current_req = orion_charge_current_limit;
            charge_voltage_req      = MIN(charge_voltage_req, 42 * 80); // Taken from BMS Code, controls voltage and current limits
            charge_voltage_req = ((charge_voltage_req & 0x00FF) << 8) | (charge_voltage_req >> 8);
            charge_current_req = ((charge_current_req & 0x00FF) << 8) | (charge_current_req >> 8);

            charge_current = can_data.elcon_charger_status.charge_current;
            charge_voltage = can_data.elcon_charger_status.charge_voltage;
            charge_current = ((charge_current & 0x00FF) << 8) | (charge_current >> 8);
            charge_voltage = ((charge_voltage & 0x00FF) << 8) | (charge_voltage >> 8);


            power = (charge_current / 10.0f) * (charge_voltage / 10.0f);
    }
    SEND_ELCON_CHARGER_COMMAND(q_tx_can, charge_voltage_req, charge_current_req, !elcon_charge_enable);
    SEND_PACK_CHARGE_STATUS(q_tx_can, (uint16_t) (power), charge_power_enable, charge_voltage, charge_current);
}

/*
    This function is exactly the same as the one in bms.c. I have moved it to orion.c to make my life easier
    and to keep things in one place. I believe this is ok bc we use the same arduino setup from before.
*/

uint16_t* orion_temp_pointer[16] = {&can_data.module_temp_0.mod_temp_0, &can_data.module_temp_1.mod_temp_0,
                              &can_data.module_temp_2.mod_temp_0, &can_data.module_temp_3.mod_temp_0,
                              &can_data.module_temp_4.mod_temp_0, &can_data.module_temp_5.mod_temp_0,
                              &can_data.module_temp_6.mod_temp_0, &can_data.module_temp_7.mod_temp_0,
                              &can_data.module_temp_8.mod_temp_0, &can_data.module_temp_9.mod_temp_0,
                              &can_data.module_temp_10.mod_temp_0, &can_data.module_temp_11.mod_temp_0,
                              &can_data.module_temp_12.mod_temp_0, &can_data.module_temp_13.mod_temp_0,
                              &can_data.module_temp_14.mod_temp_0, &can_data.module_temp_15.mod_temp_0};

void orionCheckTempsPeriodic (){
    uint16_t max_temp = 0;
    uint8_t  i, j;
    uint16_t* curr_address = &can_data.module_temp_0.mod_temp_0;
    float    avg_temp[4] = {0};

    for (i = 0; i < 16; i++) {
        for (j = 0; j < 4; j++) {
            avg_temp[j] += ((float) *(orion_temp_pointer[i] + j)) / 10;
            if (*(orion_temp_pointer[i] + j) > max_temp) {
                max_temp = *(orion_temp_pointer[i] + j);
            }
        }
    }

    SEND_MAX_CELL_TEMP(q_tx_can, max_temp);
    SEND_MOD_CELL_TEMP_AVG(q_tx_can, (uint16_t) (avg_temp[0] * 10 / 16), (uint16_t) (avg_temp[1] * 10 / 16), (uint16_t) (avg_temp[2] * 10 / 16), (uint16_t) (avg_temp[3] * 10 / 16));

    if (max_temp > MAX_TEMP) {
        orion_bms_temp_err = 1;
    } else {
        orion_bms_temp_err = 0;
    }
}
