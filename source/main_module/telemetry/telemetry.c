/**
 * @file telemetry.c
 * @brief Telemetry task implementations
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "telemetry.h"
#include "can_library/generated/MAIN_MODULE.h"
#include "vehicle_fsm.h"

void report_telemetry_50hz(void) {
    CAN_SEND_wheel_speeds(
        g_car.front_right.crit->AMK_ActualSpeed,
        g_car.front_left.crit->AMK_ActualSpeed,
        g_car.rear_left.crit->AMK_ActualSpeed,
        g_car.rear_right.crit->AMK_ActualSpeed
    );
}

void report_telemetry_1hz(void) {
    CAN_SEND_inva_diagnostics(
        g_car.front_right.state,
        g_car.front_right.info->AMK_Status_bError,
        g_car.front_right.err1->AMK_DiagnosticNumber,
        g_car.front_right.info->AMK_Status_bInverterOn
    );

    CAN_SEND_invb_diagnostics(
        g_car.front_left.state,
        g_car.front_left.info->AMK_Status_bError,
        g_car.front_left.err1->AMK_DiagnosticNumber,
        g_car.front_left.info->AMK_Status_bInverterOn
    );

    CAN_SEND_invc_diagnostics(
        g_car.rear_left.state,
        g_car.rear_left.info->AMK_Status_bError,
        g_car.rear_left.err1->AMK_DiagnosticNumber,
        g_car.rear_left.info->AMK_Status_bInverterOn
    );

    CAN_SEND_invd_diagnostics(
        g_car.rear_right.state,
        g_car.rear_right.info->AMK_Status_bError,
        g_car.rear_right.err1->AMK_DiagnosticNumber,
        g_car.rear_right.info->AMK_Status_bInverterOn
    );
}

void report_telemetry_02hz(void) {
    CAN_SEND_motor_temps(
        g_car.front_right.temps->AMK_MotorTemp,
        g_car.front_left.temps->AMK_MotorTemp,
        g_car.rear_left.temps->AMK_MotorTemp,
        g_car.rear_right.temps->AMK_MotorTemp
    );

    CAN_SEND_igbt_temps(
        g_car.front_right.temps->AMK_IGBTTemp,
        g_car.front_left.temps->AMK_IGBTTemp,
        g_car.rear_left.temps->AMK_IGBTTemp,
        g_car.rear_right.temps->AMK_IGBTTemp
    );

    CAN_SEND_main_version(GIT_HASH);
}