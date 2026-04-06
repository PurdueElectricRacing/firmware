#include "adbms.h"
#include "common/can_library/faults_common.h"
#include "common/can_library/generated/can_types.h"
#include "common/can_library/generated/A_BOX.h"
#include "telem.h"

static constexpr uint16_t MAX_PACK_CHARGING_DECIVOLTS = 540 * 10;
static constexpr uint16_t MAX_PACK_CHARGING_DECIAMPS = 20 * 10;

charging_state_t current_state = CHARGING_STATE_IDLE;
charging_state_t next_state = CHARGING_STATE_IDLE;

uint16_t charge_request_decivolts = 0;
uint16_t charge_request_deciamps = 0;
bool charge_enable = false;

extern adbms_bms_t g_bms;

static inline bool is_daqapp_requesting_charge() {
    return !can_data.charge_request.is_stale() && can_data.charge_request.charge_enable;
}

static inline bool is_elcon_ready() {
    return !can_data.elcon_status.is_stale() && !can_data.elcon_status.startup_fail;
}

static inline void report_charging_telemetry() {
    uint16_t pack_voltage = (uint16_t)(g_bms.sum_voltage * PACK_COEFF_CHARGING_TELEMETRY_PACK_VOLTAGE);
    uint16_t min_cell_voltage = (uint16_t)(g_bms.min_voltage * PACK_COEFF_CHARGING_TELEMETRY_MIN_CELL_VOLTAGE);
    uint16_t max_cell_voltage = (uint16_t)(g_bms.max_voltage * PACK_COEFF_CHARGING_TELEMETRY_MAX_CELL_VOLTAGE);

    CAN_SEND_charging_telemetry(
        pack_voltage,
        min_cell_voltage,
        max_cell_voltage,
        current_state
    );
}

static inline void update_charge_request() {
    if (can_data.charge_request.is_stale()) {
        charge_request_decivolts = 0;
        charge_request_deciamps = 0;
        charge_enable = false;
        return;
    }

    // cap the charge request to max values
    if (can_data.charge_request.charge_volts > MAX_PACK_CHARGING_DECIVOLTS) {
        charge_request_decivolts = MAX_PACK_CHARGING_DECIVOLTS;
    } else {
        charge_request_decivolts = can_data.charge_request.charge_volts;
    }

    // cap the charge request to max values
    if (can_data.charge_request.charge_current > MAX_PACK_CHARGING_DECIAMPS) {
        charge_request_deciamps = MAX_PACK_CHARGING_DECIAMPS;
    } else {
        charge_request_deciamps = can_data.charge_request.charge_current;
    }

    charge_enable = true;
}

void charging_fsm_periodic() {
    // set default states
    current_state = next_state;
    next_state = current_state;

    g_bms.is_balancing_enabled = false;
    charge_request_decivolts = 0;
    charge_request_deciamps = 0;
    charge_enable = false;

    update_fault(FAULT_ID_PACK_FULL, g_bms.sum_voltage);

    bool is_charging_permitted = 
        is_clear(FAULT_ID_PACK_FULL) &&
        is_clear(FAULT_ID_BMS_DISCONNECTED) &&
        is_latched(FAULT_ID_SDC16_TSMS) && // car should not energize while charging
        is_daqapp_requesting_charge() &&
        is_elcon_ready();

    switch (current_state) {
        case CHARGING_STATE_IDLE: {

            if (is_charging_permitted) {
                next_state = CHARGING_STATE_CHARGING;
            }
            break;
        }
        case CHARGING_STATE_CHARGING: {
            g_bms.is_balancing_enabled = true;
            update_charge_request();
            
            if (!is_charging_permitted) {
                next_state = CHARGING_STATE_IDLE;
            }
            break;
        }
    }

    CAN_SEND_elcon_command(
        charge_request_decivolts,
        charge_request_deciamps,
        !charge_enable
    );

    report_charging_telemetry();
}