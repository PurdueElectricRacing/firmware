/**
 * @file amk2.h
 * @brief Modernized AMK driver
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 * @author Cole Roberts (rober638@purdue.edu)
 * @author Chris McGalliard (cpmcgalliard@gmail.com)
 */

#ifndef AMK2_H
#define AMK2_H

#include "common/can_library/generated/MCAN.h"
#include <stdint.h>

typedef enum : uint8_t {
    AMK_STATE_OFF     = 0,
    AMK_STATE_INIT    = 1,
    AMK_STATE_RUNNING = 2
} AMK_motor_state_t;

typedef struct {
    // Flush functions
    // ! must be a wrapper around CAN library
    void (*set_function)(void);
    void (*log_function)(void);

    // Direct pointers to CAN library data structures
    // ! cast all motor objects to INVA
    INVA_SET_data_t   *set;
    INVA_CRIT_data_t  *crit;
    INVA_INFO_data_t  *info;
    INVA_TEMPS_data_t *temps;
    INVA_ERR_1_data_t *err1;
    INVA_ERR_2_data_t *err2;

    // Internal state
    AMK_motor_state_t state;
    AMK_motor_state_t next_state;
    bool *precharge_ptr; // owned pointer to precharge status from car module
} AMK_t;

void AMK_init(
    AMK_t *amk,
    void (*set_func)(void),
    void (*log_func)(void),
    INVA_SET_data_t *set,
    INVA_CRIT_data_t *crit,
    INVA_INFO_data_t *info,
    INVA_TEMPS_data_t *temps,
    INVA_ERR_1_data_t *err1,
    INVA_ERR_2_data_t *err2,
    bool *precharge_ptr
);

void AMK_reset(AMK_t* amk);
void AMK_set_torque(AMK_t* amk, int16_t torque_percent);
void AMK_periodic(AMK_t* amk);

#endif // AMK2_H
