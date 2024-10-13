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

typedef struct {
    uint8_t stage;
    uint8_t running_state;
    uint8_t init_state;
    uint8_t deinit_state;
} amk_motor_states_t;

typedef struct {
    amk_motor_states_t states;
} amk_motor_t;

typedef enum {
    MOTOR_INIT_POWER_ON,
    MOTOR_INIT_PRECHARGE,
    
} amk_motor_init_state_t;

typedef enum {
    MOTOR_STAGE_INIT,
    MOTOR_STAGE_RUNNING,
    MOTOR_STAGE_DEINIT
} amk_motor_stage_t;

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

#define DEFAULT_TARGET_VELOCITY 0
#define DEFAULT_POSITIVE_TORQUE_LIMIT 0
#define DEFAULT_NEGATIVE_TORQUE_LIMIT 0

#endif /* _AMK_H_ */
