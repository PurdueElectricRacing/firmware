#include "orion.h"

#include <stdbool.h>

#include "common/can_library/generated/A_BOX.h"
#include "common/can_library/faults_common.h"
#include "common_defs.h"

uint8_t charge_request_user          = false; // Enable charge algo
uint16_t user_charge_current_request = 0;
uint16_t user_charge_voltage_request = 0;
uint8_t orion_error                  = 0;

void orionInit() {
    user_charge_voltage_request = DEFAULT_CHARGE_VOLTAGE_REQUEST;
    user_charge_current_request = DEFAULT_CHARGE_CURRENT_REQUEST;
    charge_request_user         = false;
}

bool orionErrors() {
    static uint8_t counter;
    bool bms_err = false;

    update_fault(FAULT_INDEX_A_BOX_DISCHARGE_LIMIT_ENFORCE, can_data.orion_errors.discharge_limit_enforce | can_data.orion_errors_charger.discharge_limit_enforce);
    update_fault(FAULT_INDEX_A_BOX_CHARGER_SAFETY_RELAY, can_data.orion_errors.charger_safety_relay | can_data.orion_errors_charger.charger_safety_relay);
    update_fault(FAULT_INDEX_A_BOX_INTERNAL_HARDWARE, can_data.orion_errors.internal_hardware | can_data.orion_errors_charger.internal_hardware);
    update_fault(FAULT_INDEX_A_BOX_HEATSINK_THERMISTOR, can_data.orion_errors.heatsink_thermistor | can_data.orion_errors_charger.heatsink_thermistor);
    update_fault(FAULT_INDEX_A_BOX_SOFTWARE, can_data.orion_errors.software | can_data.orion_errors_charger.software);
    update_fault(FAULT_INDEX_A_BOX_MAX_CELLV_HIGH, can_data.orion_errors.max_cellv_high | can_data.orion_errors_charger.max_cellv_high);
    update_fault(FAULT_INDEX_A_BOX_MIN_CELLV_LOW, can_data.orion_errors.min_cellv_low | can_data.orion_errors_charger.min_cellv_low);
    update_fault(FAULT_INDEX_A_BOX_PACK_OVERHEAT_ORION, can_data.orion_errors.pack_overheat | can_data.orion_errors_charger.pack_overheat);
    update_fault(FAULT_INDEX_A_BOX_INTERNAL_COMMS, can_data.orion_errors.internal_comms | can_data.orion_errors_charger.internal_comms);
    update_fault(FAULT_INDEX_A_BOX_CELL_BALANCING_FOFF, can_data.orion_errors.cell_balancing_foff | can_data.orion_errors_charger.cell_balancing_foff);
    update_fault(FAULT_INDEX_A_BOX_WEAK_CELL, can_data.orion_errors.weak_cell | can_data.orion_errors_charger.weak_cell);
    update_fault(FAULT_INDEX_A_BOX_LOW_CELLV, can_data.orion_errors.low_cellv | can_data.orion_errors_charger.low_cellv);
    update_fault(FAULT_INDEX_A_BOX_OPEN_WIRE, can_data.orion_errors.open_wire | can_data.orion_errors_charger.open_wire);
    update_fault(FAULT_INDEX_A_BOX_CURRENT_SENSOR, can_data.orion_errors.current_sensor | can_data.orion_errors_charger.current_sensor);
    update_fault(FAULT_INDEX_A_BOX_MAX_CELLV_O5V, can_data.orion_errors.max_cellv_o5v | can_data.orion_errors_charger.max_cellv_o5v);
    update_fault(FAULT_INDEX_A_BOX_CELL_ASIC, can_data.orion_errors.cell_asic | can_data.orion_errors_charger.cell_asic);
    update_fault(FAULT_INDEX_A_BOX_WEAK_PACK, can_data.orion_errors.weak_pack | can_data.orion_errors_charger.weak_pack);
    update_fault(FAULT_INDEX_A_BOX_FAN_MONITOR, can_data.orion_errors.fan_monitor | can_data.orion_errors_charger.fan_monitor);
    update_fault(FAULT_INDEX_A_BOX_THERMISTOR, can_data.orion_errors.thermistor | can_data.orion_errors_charger.thermistor);
    update_fault(FAULT_INDEX_A_BOX_EXTERNAL_COMMS, can_data.orion_errors.external_comms | can_data.orion_errors_charger.external_comms);
    update_fault(FAULT_INDEX_A_BOX_REDUNDANT_PSU, can_data.orion_errors.redundant_psu | can_data.orion_errors_charger.redundant_psu);
    update_fault(FAULT_INDEX_A_BOX_HV_ISOLATION, can_data.orion_errors.hv_isolation | can_data.orion_errors_charger.hv_isolation);
    update_fault(FAULT_INDEX_A_BOX_INPUT_PSU, can_data.orion_errors.input_psu | can_data.orion_errors_charger.input_psu);
    update_fault(FAULT_INDEX_A_BOX_CHARGE_LIMIT_ENFORCE, can_data.orion_errors.charge_limit_enforce | can_data.orion_errors_charger.charge_limit_enforce);

    // Require dtc to be held high 4x before signaling error
    counter = (can_data.orion_info.dtc_status | can_data.orion_info_charger.dtc_status) ? counter + 1 : 0;
    if (counter == 4) {
        counter--;
        bms_err = true;
    }

    orion_error = bms_err;

    return orion_error; // TODO:  || can_data.orion_info_charger.stale);
}

/*
    Key items that are excluded from the revised function
    1. findGlobalImbalance - Everything cell balancing related is controlled by Orion - we don't get any control?
    *This funciton is also covered by DTC flags, so no longer needed*
    2. Cell balancing stuff from bms - Orion handles the balancing
*/
void orionChargePeriodic() {
    bool orion_charger_status;
    bool elcon_charge_enable    = false; // Allow power from elcon
    uint16_t charge_voltage_req = 0; // Voltage limit request to send to charger
    uint16_t charge_current_req = 0; // Current limit request to send to charger
    uint16_t charge_current; // Current charge current from charger
    uint16_t charge_voltage; // Current pack voltage from charger
    float power;

    orion_charger_status = can_data.orion_info_charger.is_charging && !can_data.orion_info_charger.stale;

    charge_request_user &= !can_data.elcon_charger_status.stale;
    if (charge_request_user && orion_charger_status && !orionErrors() && !warning_latched() && !fatal_latched()) {
        elcon_charge_enable = true;

        //user_charge_current_request = 10;
        charge_current_req = MIN(can_data.orion_info_charger.pack_ccl, user_charge_current_request);

        //user_charge_voltage_request = 314;
        charge_voltage_req = MIN(user_charge_voltage_request, MAX_VOLT); // Hard limit, don't overcharge
        charge_voltage_req *= 10;
        charge_current_req *= 10;

        // Swap endianess
        charge_voltage_req = ((charge_voltage_req & 0x00FF) << 8) | (charge_voltage_req >> 8);
        charge_current_req = ((charge_current_req & 0x00FF) << 8) | (charge_current_req >> 8);
    }
    if (!elcon_charge_enable)
        __asm__("nop"); // for bkpt

    CAN_SEND_elcon_charger_command(charge_voltage_req, charge_current_req, !elcon_charge_enable);

    // Parse current values from elcon charger status
    charge_current = can_data.elcon_charger_status.charge_current;
    charge_voltage = can_data.elcon_charger_status.charge_voltage;
    // Swap endianess
    charge_current = ((charge_current & 0x00FF) << 8) | (charge_current >> 8);
    charge_voltage = ((charge_voltage & 0x00FF) << 8) | (charge_voltage >> 8);
    power          = (charge_current / 10.0f) * (charge_voltage / 10.0f);
    CAN_SEND_pack_charge_status((uint16_t)(power), elcon_charge_enable, charge_voltage, charge_current);
}
