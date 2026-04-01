#include "adbms.h"
#include "common/can_library/faults_common.h"
#include "common/can_library/generated/can_types.h"
#include "common/can_library/generated/A_BOX.h"

static constexpr uint16_t PACK_CHARGING_DECIVOLTS = 540 * 10;
static constexpr uint16_t PACK_CHARGING_DECIAMPS = 10 * 10;

charging_state_t current_state = CHARGING_STATE_IDLE;
charging_state_t next_state = CHARGING_STATE_IDLE;

extern adbms_bms_t g_bms;

static inline bool is_daq_requesting_charge() {
    return !can_data.charge_request.stale && can_data.charge_request.charge_enable;
}

static inline bool is_elcon_ready() {
    return !can_data.elcon_status.stale && !can_data.elcon_status.startup_fail;
}

static inline void report_charging_telemetry() {
    // todo
}

void charging_fsm_periodic() {
    // set default states
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
            // do nothing

            if (is_charging_permitted) {
                next_state = CHARGING_STATE_CHARGING;
            }
            break;
        }
        case CHARGING_STATE_CHARGING: {
            g_bms.is_balancing_enabled = true;

            // todo daq requests the charging parameters?
            CAN_SEND_elcon_command(PACK_CHARGING_DECIVOLTS, PACK_CHARGING_DECIAMPS, false);
            
            report_charging_telemetry();
            
            if (!is_charging_permitted) {
                next_state = CHARGING_STATE_IDLE;
            }
            break;
        }
    }
}