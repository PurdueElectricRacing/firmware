/**
 * @file amk.c
 * @brief AMK page implementation
 *
 * @author Amruth Nadimpally (nadimpaa@purdue.edu)
 * @author Aditya Saini (sain91@purdue.edu)
 */

#include "amk.h"

#include "common/can_library/generated/DASHBOARD.h"
#include "nextion.h"
#include "colors.h"

void reset_inverter_text_colors(char *error_id, char *torque_id, char *on_id) {
    NXT_setFontColor(error_id, WHITE);
    NXT_setFontColor(torque_id, WHITE);
    NXT_setFontColor(on_id, WHITE);
}

void update_inverter_telemetry(
    char *status_id,
    char *error_id,
    char *torque_id,
    char *on_id,
    bool is_stale,
    bool is_enabled,
    bool command_inverter_on,
    bool command_error_reset,
    int16_t torque_setpoint,
    bool inverter_on,
    bool has_error
) {
    if (is_stale) {
        NXT_setFontColor(status_id, RED);
        NXT_setText(status_id, "STALE");
        NXT_setText(error_id, "--");
        NXT_setText(torque_id, "--");
        NXT_setText(on_id, "--");
        return;
    }
    
    NXT_setFontColor(status_id, is_enabled ? GREEN : RED);
    NXT_setText(status_id, is_enabled ? "ENABLE" : "DISABLE");
    NXT_setText(on_id, (inverter_on || command_inverter_on) ? "ON" : "OFF");
    NXT_setText(error_id, has_error ? "ERROR" : (command_error_reset ? "RESET" : "OK"));
    NXT_setTextFormatted(torque_id, "%d", (int) torque_setpoint);
}

void amk_telemetry_update() {
    reset_inverter_text_colors(INVA_ERROR, INVA_DIAGNOSTIC, INVA_ON);
    reset_inverter_text_colors(INVB_ERROR, INVB_DIAGNOSTIC, INVB_ON);
    reset_inverter_text_colors(INVC_ERROR, INVC_DIAGNOSTIC, INVC_ON);
    reset_inverter_text_colors(INVD_ERROR, INVD_DIAGNOSTIC, INVD_ON);

    update_inverter_telemetry(
        INVA_STATUS,
        INVA_ERROR,
        INVA_DIAGNOSTIC,
        INVA_ON,
        can_data.inva_state_diag_vcan.is_stale(),
        can_data.inva_state_diag_vcan.amk_control_enable,
        can_data.inva_state_diag_vcan.amk_control_inverter_on,
        can_data.inva_state_diag_vcan.amk_control_error_reset,
        can_data.inva_state_diag_vcan.amk_torque_setpoint,
        can_data.inva_state_diag_vcan.amk_state_inverter_on,
        can_data.inva_state_diag_vcan.amk_state_error
    );
    update_inverter_telemetry(
        INVB_STATUS,
        INVB_ERROR,
        INVB_DIAGNOSTIC,
        INVB_ON,
        can_data.invb_state_diag_vcan.is_stale(),
        can_data.invb_state_diag_vcan.amk_control_enable,
        can_data.invb_state_diag_vcan.amk_control_inverter_on,
        can_data.invb_state_diag_vcan.amk_control_error_reset,
        can_data.invb_state_diag_vcan.amk_torque_setpoint,
        can_data.invb_state_diag_vcan.amk_state_inverter_on,
        can_data.invb_state_diag_vcan.amk_state_error
    );
    update_inverter_telemetry(
        INVC_STATUS,
        INVC_ERROR,
        INVC_DIAGNOSTIC,
        INVC_ON,
        can_data.invc_state_diag_vcan.is_stale(),
        can_data.invc_state_diag_vcan.amk_control_enable,
        can_data.invc_state_diag_vcan.amk_control_inverter_on,
        can_data.invc_state_diag_vcan.amk_control_error_reset,
        can_data.invc_state_diag_vcan.amk_torque_setpoint,
        can_data.invc_state_diag_vcan.amk_state_inverter_on,
        can_data.invc_state_diag_vcan.amk_state_error
    );
    update_inverter_telemetry(
        INVD_STATUS,
        INVD_ERROR,
        INVD_DIAGNOSTIC,
        INVD_ON,
        can_data.invd_state_diag_vcan.is_stale(),
        can_data.invd_state_diag_vcan.amk_control_enable,
        can_data.invd_state_diag_vcan.amk_control_inverter_on,
        can_data.invd_state_diag_vcan.amk_control_error_reset,
        can_data.invd_state_diag_vcan.amk_torque_setpoint,
        can_data.invd_state_diag_vcan.amk_state_inverter_on,
        can_data.invd_state_diag_vcan.amk_state_error
    );
}