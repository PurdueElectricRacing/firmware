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
    uint8_t inverter_state,
    bool has_error,
    uint32_t diagnostic_number,
    bool inverter_on
) {
    NXT_setFontColor(error_id, WHITE);
    NXT_setFontColor(diagnostic_id, WHITE);
    NXT_setFontColor(on_id, WHITE);

    if (is_stale) {
        NXT_setFontColor(status_id, RED);
        NXT_setText(status_id, "STALE");
        NXT_setText(error_id, "--");
        NXT_setText(diagnostic_id, "--");
        NXT_setText(on_id, "--");
        return;
    }

    NXT_setFontColor(status_id, has_error ? RED : GREEN);
    NXT_setTextFormatted(status_id, "%u", (unsigned int) inverter_state);
    NXT_setText(error_id, has_error ? "ERROR" : "OK");
    NXT_setTextFormatted(diagnostic_id, "%lu", (unsigned int) diagnostic_number);
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