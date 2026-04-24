/**
 * @file telemetry.c
 * @brief Telemetry task implementations
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "telemetry.h"
#include "can_library/generated/MAIN_MODULE.h"
#include "vehicle_fsm.h"

/**
 * @brief Reports telemetry data at 50 Hz rate
 * Includes: AMK reported wheel_speeds
 */
static_assert(TELEMETRY_50HZ_PERIOD_MS == WHEEL_SPEEDS_PERIOD_MS);
void report_telemetry_50hz(void) {
    CAN_SEND_wheel_speeds(
        g_car.front_right.crit->AMK_ActualSpeed,
        g_car.front_left.crit->AMK_ActualSpeed,
        g_car.rear_left.crit->AMK_ActualSpeed,
        g_car.rear_right.crit->AMK_ActualSpeed
    );
}

/**
 * @brief Reports telemetry data at 1 Hz rate
 * Includes: AMK diagnostics
 */
static_assert(TELEMETRY_1HZ_PERIOD_MS == INVA_DIAGNOSTICS_PERIOD_MS);
static_assert(TELEMETRY_1HZ_PERIOD_MS == INVB_DIAGNOSTICS_PERIOD_MS);
static_assert(TELEMETRY_1HZ_PERIOD_MS == INVC_DIAGNOSTICS_PERIOD_MS);
static_assert(TELEMETRY_1HZ_PERIOD_MS == INVD_DIAGNOSTICS_PERIOD_MS);
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

/**
 * @brief Reports telemetry data at 0.2 Hz rate
 * Includes: AMK reported motor and IGBT temperatures, firmware version
 */
static_assert(TELEMETRY_02HZ_PERIOD_MS == MOTOR_TEMPS_PERIOD_MS);
static_assert(TELEMETRY_02HZ_PERIOD_MS == IGBT_TEMPS_PERIOD_MS);
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