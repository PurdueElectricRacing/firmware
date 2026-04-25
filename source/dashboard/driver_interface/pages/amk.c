/**
 * @file amk.c
 * @brief AMK page implementation
 *
 * @author Amruth Nadimpally (nadimpaa@purdue.edu)
 * @author Aditya Saini (sain91@purdue.edu)
 */

#include "amk.h"

#include "can_library/generated/DASHBOARD.h"
#include "common/nextion/nextion.h"
#include "colors.h"

static void update_inverter_telemetry(
    char *status_id,
    char *error_id,
    char *diagnostic_id,
    char *on_id,
    bool is_stale,
    AMK_state_t inverter_state,
    bool has_error,
    uint32_t diagnostic_number,
    bool inverter_on
) {
    if (is_stale) {
        NXT_setFontColor(status_id, WHITE);
        NXT_setText(status_id, "STALE");
        NXT_setText(error_id, "--");
        NXT_setText(diagnostic_id, "--");
        NXT_setText(on_id, "--");
        return;
    }

    switch(inverter_state) {
        case AMK_STATE_OFF:
            NXT_setText(status_id, "OFF");
            break;
        case AMK_STATE_STARTING:
            NXT_setText(status_id, "STARTING");
            break;
        case AMK_STATE_RUNNING:
            NXT_setText(status_id, "RUNNING");
            break;
        case AMK_STATE_RECOVERING:
            NXT_setText(status_id, "RECOVERING");
            break;
        case AMK_STATE_FATAL:
            NXT_setText(status_id, "FATAL");
            break;
        default:
            NXT_setText(status_id, "UNKNOWN");
            break;
    }
    NXT_setFontColor(error_id, has_error ? RED : GREEN);
    NXT_setText(error_id, has_error ? "ERROR" : "OK");
    NXT_setTextFormatted(diagnostic_id, "%d", diagnostic_number);
    NXT_setText(on_id, inverter_on ? "ON" : "OFF");
}

void amk_telemetry_update() {
    update_inverter_telemetry(
        INVA_STATUS,
        INVA_ERROR,
        INVA_DIAGNOSTIC,
        INVA_ON,
        can_data.inva_diagnostics.is_stale(),
        can_data.inva_diagnostics.inverter_state,
        can_data.inva_diagnostics.bError,
        can_data.inva_diagnostics.diagnostic_number,
        can_data.inva_diagnostics.InverterOn
    );
    update_inverter_telemetry(
        INVB_STATUS,
        INVB_ERROR,
        INVB_DIAGNOSTIC,
        INVB_ON,
        can_data.invb_diagnostics.is_stale(),
        can_data.invb_diagnostics.inverter_state,
        can_data.invb_diagnostics.bError,
        can_data.invb_diagnostics.diagnostic_number,
        can_data.invb_diagnostics.InverterOn
    );
    update_inverter_telemetry(
        INVC_STATUS,
        INVC_ERROR,
        INVC_DIAGNOSTIC,
        INVC_ON,
        can_data.invc_diagnostics.is_stale(),
        can_data.invc_diagnostics.inverter_state,
        can_data.invc_diagnostics.bError,
        can_data.invc_diagnostics.diagnostic_number,
        can_data.invc_diagnostics.InverterOn
    );
    update_inverter_telemetry(
        INVD_STATUS,
        INVD_ERROR,
        INVD_DIAGNOSTIC,
        INVD_ON,
        can_data.invd_diagnostics.is_stale(),
        can_data.invd_diagnostics.inverter_state,
        can_data.invd_diagnostics.bError,
        can_data.invd_diagnostics.diagnostic_number,
        can_data.invd_diagnostics.InverterOn
    );
}