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

void amk_telemetry_update() {
    NXT_setFontColor(INVA_BERROR, WHITE);
    NXT_setFontColor(INVB_BERROR, WHITE);
    NXT_setFontColor(INVC_BERROR, WHITE);
    NXT_setFontColor(INVD_BERROR, WHITE);
    NXT_setFontColor(INVA_DIAGNOSTIC, WHITE);
    NXT_setFontColor(INVB_DIAGNOSTIC, WHITE);
    NXT_setFontColor(INVC_DIAGNOSTIC, WHITE);
    NXT_setFontColor(INVD_DIAGNOSTIC, WHITE);
    NXT_setFontColor(INVA_ON, WHITE);
    NXT_setFontColor(INVB_ON, WHITE);
    NXT_setFontColor(INVC_ON, WHITE);
    NXT_setFontColor(INVD_ON, WHITE);

    if (can_data.inva_set_vcan.is_stale()) {
        NXT_setFontColor(INVA_STATUS, RED);
        NXT_setText(INVA_STATUS, "STALE");
        NXT_setText(INVA_BERROR, "--");
        NXT_setText(INVA_DIAGNOSTIC, "--");
        NXT_setText(INVA_ON, "--");
    } else {
        NXT_setFontColor(INVA_STATUS, can_data.inva_set_vcan.AMK_Control_bEnable ? GREEN : RED);
        NXT_setText(INVA_STATUS, can_data.inva_set_vcan.AMK_Control_bEnable ? "ENABLE" : "DISABLE");
        NXT_setText(INVA_ON, can_data.inva_set_vcan.AMK_Control_bInverterOn ? "ON" : "OFF");
        NXT_setText(INVA_BERROR, can_data.inva_set_vcan.AMK_Control_bErrorReset ? "RESET" : "OK");
        int16_t inva_torque = (int16_t)(
            can_data.inva_set_vcan.AMK_TorqueSetpoint * UNPACK_COEFF_INVC_SET_VCAN_AMK_TORQUESETPOINT
        );
        NXT_setTextFormatted(INVA_DIAGNOSTIC, "%d", inva_torque);
    }

    if (can_data.invb_set_vcan.is_stale()) {
        NXT_setFontColor(INVB_STATUS, RED);
        NXT_setText(INVB_STATUS, "STALE");
        NXT_setText(INVB_BERROR, "--");
        NXT_setText(INVB_DIAGNOSTIC, "--");
        NXT_setText(INVB_ON, "--");
    } else {
        NXT_setFontColor(INVB_STATUS, can_data.invb_set_vcan.AMK_Control_bEnable ? GREEN : RED);
        NXT_setText(INVB_STATUS, can_data.invb_set_vcan.AMK_Control_bEnable ? "ENABLE" : "DISABLE");
        NXT_setText(INVB_ON, can_data.invb_set_vcan.AMK_Control_bInverterOn ? "ON" : "OFF");
        NXT_setText(INVB_BERROR, can_data.invb_set_vcan.AMK_Control_bErrorReset ? "RESET" : "OK");
        int16_t invb_torque = (int16_t)(
            can_data.invb_set_vcan.AMK_TorqueSetpoint * UNPACK_COEFF_INVC_SET_VCAN_AMK_TORQUESETPOINT
        );
        NXT_setTextFormatted(INVB_DIAGNOSTIC, "%d", invb_torque);
    }

    if (can_data.invc_set_vcan.is_stale()) {
        NXT_setFontColor(INVC_STATUS, RED);
        NXT_setText(INVC_STATUS, "STALE");
        NXT_setText(INVC_BERROR, "--");
        NXT_setText(INVC_DIAGNOSTIC, "--");
        NXT_setText(INVC_ON, "--");
    } else {
        NXT_setFontColor(INVC_STATUS, can_data.invc_set_vcan.AMK_Control_bEnable ? GREEN : RED);
        NXT_setText(INVC_STATUS, can_data.invc_set_vcan.AMK_Control_bEnable ? "ENABLE" : "DISABLE");
        NXT_setText(INVC_ON, can_data.invc_set_vcan.AMK_Control_bInverterOn ? "ON" : "OFF");
        NXT_setText(INVC_BERROR, can_data.invc_set_vcan.AMK_Control_bErrorReset ? "RESET" : "OK");
        int16_t invc_torque = (int16_t)(
            can_data.invc_set_vcan.AMK_TorqueSetpoint * UNPACK_COEFF_INVC_SET_VCAN_AMK_TORQUESETPOINT
        );
        NXT_setTextFormatted(INVC_DIAGNOSTIC, "%d", invc_torque);
    }

    if (can_data.invd_set_vcan.is_stale()) {
        NXT_setFontColor(INVD_STATUS, RED);
        NXT_setText(INVD_STATUS, "STALE");
        NXT_setText(INVD_BERROR, "--");
        NXT_setText(INVD_DIAGNOSTIC, "--");
        NXT_setText(INVD_ON, "--");
    } else {
        NXT_setFontColor(INVD_STATUS, can_data.invd_set_vcan.AMK_Control_bEnable ? GREEN : RED);
        NXT_setText(INVD_STATUS, can_data.invd_set_vcan.AMK_Control_bEnable ? "ENABLE" : "DISABLE");
        NXT_setText(INVD_ON, can_data.invd_set_vcan.AMK_Control_bInverterOn ? "ON" : "OFF");
        NXT_setText(INVD_BERROR, can_data.invd_set_vcan.AMK_Control_bErrorReset ? "RESET" : "OK");
        int16_t invd_torque = (int16_t)(
            can_data.invd_set_vcan.AMK_TorqueSetpoint * UNPACK_COEFF_INVC_SET_VCAN_AMK_TORQUESETPOINT
        );
        NXT_setTextFormatted(INVD_DIAGNOSTIC, "%d", invd_torque);
    }
}