#include "adbms.h"
#include "common/can_library/faults_common.h"
#include "common/can_library/generated/can_types.h"
#include "common/can_library/generated/A_BOX.h"

charging_state_t current_state = CHARGING_STATE_IDLE;
charging_state_t next_state = CHARGING_STATE_IDLE;

extern adbms_bms_t g_bms;

static inline bool is_daq_requesting_charge() {
    return !can_data.charge_request.stale && can_data.charge_request.charge_enable;
}

static inline bool is_elcon_ready() {
    return !can_data.elcon_charger_status.stale && !can_data.elcon_charger_status.startup_fail;
}

void charging_fsm_periodic() {
    current_state = next_state;
    next_state = current_state;
    g_bms.is_balancing_enabled = false;

    update_fault(FAULT_ID_PACK_FULL, g_bms.sum_voltage);

    bool is_charging_permitted = 
        is_clear(FAULT_ID_PACK_FULL) &&
        is_clear(FAULT_ID_BMS_DISCONNECTED) &&
        is_latched(FAULT_ID_SDC16_TSMS) && // car should not energize while charging
        is_daq_requesting_charge() &&
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

            // send elcon charging message
            // send pack stats on CCAN

            if (!is_charging_permitted) {
                next_state = CHARGING_STATE_IDLE;
            }
            break;
        }
    }
}