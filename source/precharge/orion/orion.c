#include "orion.h"

#include <stdbool.h>
#include "can_parse.h"
#include "common_defs.h"
// #include "daq.h"


extern q_handle_t q_tx_can;

uint8_t charge_request_user = false; // Enable charge algo
uint16_t user_charge_current_request = 0;
uint16_t user_charge_voltage_request = 0;
uint8_t  orion_bms_temp_err = 0;


void orionInit()
{
    // linkReada(DAQ_ID_CHARGE_MODE_ENABLE, &charge_request_user);
    // linkWritea(DAQ_ID_CHARGE_MODE_ENABLE, &charge_request_user);
    // linkReada(DAQ_ID_CHARGE_VOLTAGE_LIMIT, &user_charge_voltage_request);
    // linkWritea(DAQ_ID_CHARGE_VOLTAGE_LIMIT, &user_charge_voltage_request);
    // linkReada(DAQ_ID_CHARGE_CURRENT_LIMIT, &user_charge_current_request);
    // linkWritea(DAQ_ID_CHARGE_CURRENT_LIMIT, &user_charge_current_request);
}


bool orionErrors() {
    static uint8_t counter;
    bool bms_err = false;

    // Require dtc to be held high 4x before signaling error
    counter = can_data.orion_info.dtc_status ? counter + 1 : 0;
    if (counter == 4)
    {
        counter--;
        bms_err = true;
    }

    return (bms_err ||
            /* TODO: can_data.orion_info.stale || */
            orion_bms_temp_err);
}

/*
    Key items that are excluded from the revised function
    1. findGlobalImbalance - Everything cell balancing related is controlled by Orion - we don't get any control?
    *This funciton is also covered by DTC flags, so no longer needed*
    2. Cell balancing stuff from bms - Orion handles the balancing
*/
void orionChargePeriodic() {
    bool orion_charger_status;
    bool elcon_charge_enable  = false; // Allow power from elcon
    uint16_t charge_voltage_req = 0;   // Voltage limit request to send to charger
    uint16_t charge_current_req = 0;   // Current limit request to send to charger
    uint16_t charge_current;           // Current charge current from charger
    uint16_t charge_voltage;           // Current pack voltage from charger
    float power;

    orion_charger_status = can_data.orion_info.is_charging /* TODO: &&
                           !can_data.orion_info.stale */;

    /* TODO: charge_request_user &= !can_data.elcon_charger_status.stale; */
    if (charge_request_user && orion_charger_status && !orionErrors()) {
            elcon_charge_enable = true;

            charge_current_req = MIN(can_data.orion_info.pack_ccl, user_charge_current_request);

            charge_voltage_req = MIN(user_charge_voltage_request, MAX_VOLT); // Hard limit, don't overcharge
            charge_voltage_req *=  10;
            charge_current_req *= 10;

            // Swap endianess
            charge_voltage_req = ((charge_voltage_req & 0x00FF) << 8) | (charge_voltage_req >> 8);
            charge_current_req = ((charge_current_req & 0x00FF) << 8) | (charge_current_req >> 8);
    }
    if (!elcon_charge_enable) asm("nop"); // for bkpt

    SEND_ELCON_CHARGER_COMMAND(q_tx_can, charge_voltage_req, charge_current_req, !elcon_charge_enable);

    // Parse current values from elcon charger status
    charge_current = can_data.elcon_charger_status.charge_current;
    charge_voltage = can_data.elcon_charger_status.charge_voltage;
    // Swap endianess
    charge_current = ((charge_current & 0x00FF) << 8) | (charge_current >> 8);
    charge_voltage = ((charge_voltage & 0x00FF) << 8) | (charge_voltage >> 8);
    power = (charge_current / 10.0f) * (charge_voltage / 10.0f);
    SEND_PACK_CHARGE_STATUS(q_tx_can, (uint16_t) (power), elcon_charge_enable, charge_voltage, charge_current);
}


uint16_t* orion_temp_pointer[16] = {&can_data.module_temp_0.mod_temp_0,  &can_data.module_temp_1.mod_temp_0,
                                    &can_data.module_temp_2.mod_temp_0,  &can_data.module_temp_3.mod_temp_0,
                                    &can_data.module_temp_4.mod_temp_0,  &can_data.module_temp_5.mod_temp_0,
                                    &can_data.module_temp_6.mod_temp_0,  &can_data.module_temp_7.mod_temp_0,
                                    &can_data.module_temp_8.mod_temp_0,  &can_data.module_temp_9.mod_temp_0,
                                    &can_data.module_temp_10.mod_temp_0, &can_data.module_temp_11.mod_temp_0,
                                    &can_data.module_temp_12.mod_temp_0, &can_data.module_temp_13.mod_temp_0,
                                    &can_data.module_temp_14.mod_temp_0, &can_data.module_temp_15.mod_temp_0};

void orionCheckTempsPeriodic (){
    uint16_t max_temp = 0;
    uint8_t  i, j;
    uint16_t *curr_address = &can_data.module_temp_0.mod_temp_0;
    float    avg_temp[4] = {0};

    // Calculate average temperature per module and overall max
    for (i = 0; i < 16; i++) {
        for (j = 0; j < 4; j++) {
            avg_temp[j] += ((float) *(orion_temp_pointer[i] + j)) / 10;
            if (*(orion_temp_pointer[i] + j) > max_temp) {
                max_temp = *(orion_temp_pointer[i] + j);
            }
        }
    }

    SEND_MAX_CELL_TEMP(q_tx_can, max_temp);
    SEND_MOD_CELL_TEMP_AVG(q_tx_can, (uint16_t) (avg_temp[0] * 10 / 16),
                                     (uint16_t) (avg_temp[1] * 10 / 16),
                                     (uint16_t) (avg_temp[2] * 10 / 16),
                                     (uint16_t) (avg_temp[3] * 10 / 16));

    // Send raw temperatures for heat map generation
    static uint8_t idx;
    SEND_RAW_CELL_TEMP(q_tx_can, idx,
                                 *(orion_temp_pointer[idx] + 0),
                                 *(orion_temp_pointer[idx] + 1),
                                 *(orion_temp_pointer[idx] + 2),
                                 *(orion_temp_pointer[idx] + 3));
    idx = (idx == 15) ? 0 : idx + 1;

    // Require over-temp 10 times before signaling error (filter out noise)
    static uint8_t counter;
    counter = (max_temp >= MAX_TEMP) ? counter + 1 : 0;

    if (counter == 10)
    {
        counter--;
        orion_bms_temp_err = 1;
    }
    else orion_bms_temp_err = 0;

}
