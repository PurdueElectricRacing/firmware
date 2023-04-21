#include "MC_PL_pp.h"
#include "MC_PL0.h"
#include "can_parse.h"
#include "common/modules/wheel_speeds/wheel_speeds.h"
#include "common_defs.h"

extern WheelSpeeds_t *wheel_speeds;

void MC_PL_pp(ExtU* rtU, motor_t* motor_left, micro_t* micro) {
    rtU->Tx[0] = CLAMP(0.0 * TORQUE_CALIBRATION, ABS_MIN_TORQUE, ABS_MAX_TORQUE);
    rtU->Tx[1] = CLAMP(0.0 * TORQUE_CALIBRATION, ABS_MIN_TORQUE, ABS_MAX_TORQUE);
    rtU->Tx[2] = CLAMP(micro->Tx_in[2] * TORQUE_CALIBRATION, ABS_MIN_TORQUE, ABS_MAX_TORQUE);
    rtU->Tx[3] = CLAMP(0.0 * TORQUE_CALIBRATION, ABS_MIN_TORQUE, ABS_MAX_TORQUE);

    rtU->Wx[0] = CLAMP(0.00001 * SPEED_CALIBRATION, MIN_OMEGA, MAX_OMEGA);
    rtU->Wx[1] = CLAMP(0.00001 * SPEED_CALIBRATION, MIN_OMEGA, MAX_OMEGA);
    rtU->Wx[2] = CLAMP(0, MIN_OMEGA, MAX_OMEGA);
    rtU->Wx[3] = CLAMP(0, MIN_OMEGA, MAX_OMEGA);

    rtU->motor_T[0] = CLAMP(40.0 * TEMPERATURE_CALIBRATION, MIN_MOTOR_TEMPERATURE, MAX_MOTOR_TEMPERATURE);
    rtU->motor_T[1] = CLAMP(40.0 * TEMPERATURE_CALIBRATION, MIN_MOTOR_TEMPERATURE, MAX_MOTOR_TEMPERATURE);
    rtU->motor_T[2] = CLAMP(motor_left->motor_temp * TEMPERATURE_CALIBRATION, MIN_MOTOR_TEMPERATURE, MAX_MOTOR_TEMPERATURE);
    rtU->motor_T[3] = CLAMP(40.0 * TEMPERATURE_CALIBRATION, MIN_MOTOR_TEMPERATURE, MAX_MOTOR_TEMPERATURE);

    rtU->Motor_V[0] = CLAMP(200.0 * VOLTAGE_CALIBRATION, MIN_VOLTAGE, MAX_VOLTAGE);
    rtU->Motor_V[1] = CLAMP(200.0 * VOLTAGE_CALIBRATION, MIN_VOLTAGE, MAX_VOLTAGE);
    rtU->Motor_V[2] = CLAMP(motor_left->voltage_x10 * VOLTAGE_CALIBRATION, MIN_VOLTAGE, MAX_VOLTAGE);
    rtU->Motor_V[3] = CLAMP(200.0 * VOLTAGE_CALIBRATION, MIN_VOLTAGE, MAX_VOLTAGE);

    rtU->Motor_I[0] = CLAMP(0.0 * CURRENT_CALIBRATION, MIN_MOTOR_CURRENT, MAX_MOTOR_CURRENT);
    rtU->Motor_I[1] = CLAMP(0.0 * CURRENT_CALIBRATION , MIN_MOTOR_CURRENT, MAX_MOTOR_CURRENT);
    rtU->Motor_I[2] = CLAMP(motor_left->current_x10 * CURRENT_CALIBRATION, MIN_MOTOR_CURRENT, MAX_MOTOR_CURRENT);
    rtU->Motor_I[3] = CLAMP(0.0 * CURRENT_CALIBRATION, MIN_MOTOR_CURRENT, MAX_MOTOR_CURRENT);

    rtU->power_limits[0] = CLAMP(100.0, 0.0, MAX_BATTERY_POWER);
    rtU->power_limits[1] = CLAMP(0.0, 0.0, MIN_BATTERY_POWER);

    //rtU->Motor_V[0] = CLAMP(can_data.orion_currents_volts.pack_voltage * VOLTAGE_CALIBRATION, MIN_VOLTAGE, MAX_VOLTAGE);
    //rtU->Motor_V[1] = CLAMP(can_data.orion_currents_volts.pack_voltage * VOLTAGE_CALIBRATION, MIN_VOLTAGE, MAX_VOLTAGE);
    //rtU->Motor_V[2] = CLAMP(can_data.orion_currents_volts.pack_voltage * VOLTAGE_CALIBRATION, MIN_VOLTAGE, MAX_VOLTAGE);
    //rtU->Motor_V[3] = CLAMP(can_data.orion_currents_volts.pack_voltage * VOLTAGE_CALIBRATION, MIN_VOLTAGE, MAX_VOLTAGE);

    //rtU->power_limits[0] = CLAMP(can_data.orion_currents_volts.pack_voltage * VOLTAGE_CALIBRATION * can_data.orion_info.pack_dcl, 0, MAX_BATTERY_POWER);
    //rtU->power_limits[1] = CLAMP(can_data.orion_currents_volts.pack_voltage * VOLTAGE_CALIBRATION * can_data.orion_info.pack_ccl, 0, MIN_BATTERY_POWER);

}