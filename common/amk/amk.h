/**
 * @file amk.h
 * @author Cole Roberts (rober638@purdue.edu)
 * @brief  Vroom
 * @version 0.1
 * @date 2024-10-11
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef _AMK_H_
#define _AMK_H_

#include <stdbool.h>
#include <stdint.h>

/* Inverter -> CAN */
/* In AMK_Actual_Values_1 */
typedef union
{
    struct {
        uint16_t AMK_bReserve        : 8;
        uint16_t AMK_bSystemReady    : 1;
        uint16_t AMK_bError          : 1;
        uint16_t AMK_bWarn           : 1;
        uint16_t AMK_bQuitDcOn       : 1;
        uint16_t AMK_bDcOn           : 1; /* Same as QUE ?? */
        uint16_t AMK_bQuitInverterOn : 1;
        uint16_t AMK_bInverterOn     : 1;
        uint16_t AMK_bDerating       : 1;
    } fields;
    uint16_t bits;
} AMK_Status_t;

/* CAN -> Inverter */
/* In AMK_Setpoints */
/* THIS NEEDS TO BE SENT EVERY 50ms */
typedef union
{
    struct {
        uint16_t AMK_bReserve1   : 8;
        uint16_t AMK_bInverterOn : 1;
        uint16_t AMK_bDcOn       : 1;
        uint16_t AMK_bEnable     : 1;
        uint16_t AMK_bErrorReset : 1;
        uint16_t AMK_bReserve2   : 1;
    } fields;
    uint16_t bits;
} AMK_Control_t;

typedef struct {
    // uint8_t state;
    uint8_t stage;
    uint8_t running_stage;
    uint8_t init_stage;
    uint8_t deinit_stage;
    uint8_t reset_state;
} amk_motor_states_t;

typedef struct {
    amk_motor_states_t states;
    AMK_Status_t status;
    AMK_Control_t control;

    /* Scaling: 0.1% */
    int16_t torque_setpoint;
    int16_t torque_limit_positive;
    int16_t torque_limit_negative;

    /* Scaling: 0.1% */
    int16_t actual_torque;
    int16_t actual_speed;

    uint32_t serial_num; // for sanity checking

    /* Scaling: 0.1% */
    int16_t motor_temp;
    int16_t inverter_temp;
    int16_t igbt_temp;

    uint16_t dc_bus_voltage;
    uint16_t system_reset;

    uint16_t diagnostic_num;

    bool* pchg_complete;
} amk_motor_t;

void motorInit(amk_motor_t* motor, bool* pchg_complete);
void motorPeriodic(amk_motor_t* motor);
void motorSetTorque(amk_motor_t* motor, int16_t torque_setpoint);

typedef enum {
    MOTOR_INIT_POWER_ON = 0,
    MOTOR_INIT_PRECHARGE,
    MOTOR_INIT_DC_ON,
    MOTOR_INIT_DC_ON_CHECK,
    MOTOR_INIT_TORQUE_INIT,
    MOTOR_INIT_ENABLE,
    MOTOR_INIT_INVERTER_ON,
    MOTOR_INIT_INVERTER_ON_CHECK,
    MOTOR_INIT_DONE,
} amk_motor_init_state_t;

typedef enum {
    MOTOR_DEINIT_SETPOINTS_DEINIT = 0,
    MOTOR_DEINIT_INVERTER_OFF,
    MOTOR_DEINIT_DISABLE,
    MOTOR_DEINIT_QUIT_INVERTER_CHECK,
    MOTOR_DEINIT_DC_OFF,
    MOTOR_DEINIT_DC_OFF_CHECK,
    MOTOR_DEINIT_PRECHARGE,
    MOTOR_DEINIT_POWER_OFF,
    MOTOR_DEINIT_DONE,
} amk_motor_deinit_state_t;

typedef enum {
    MOTOR_RUNNING_GOOD = 0,
    MOTOR_RUNNING_ERROR,
} amk_motor_running_state_t;

typedef enum {
    MOTOR_RESET_INVERTER_OFF = 0,
    MOTOR_RESET_ERROR_RESET_ON,
    MOTOR_RESET_ERROR_RESET_OFF,
    MOTOR_RESET_CHECK_SYSTEM_READY,
} amk_motor_reset_state_t;

typedef enum {
    MOTOR_STAGE_OFF = 0,
    MOTOR_STAGE_INIT,
    MOTOR_STAGE_RUNNING,
    MOTOR_STAGE_DEINIT
} amk_motor_stage_t;

#define DEFAULT_TORQUE_SETPOINT 0
#define DEFAULT_POSITIVE_TORQUE_LIMIT 0
#define DEFAULT_NEGATIVE_TORQUE_LIMIT 0

#define MAX_POSITIVE_TORQUE_SETPOINT 1000
#define MAX_NEGATIVE_TORQUE_SETPOINT -1000
#define MAX_POSITIVE_TORQUE_LIMIT 1000
#define MAX_NEGATIVE_TORQUE_LIMIT -1000

#endif /* _AMK_H_ */
