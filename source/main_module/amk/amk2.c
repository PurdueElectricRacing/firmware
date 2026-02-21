/**
 * @file amk2.c
 * @brief Modernized AMK driver
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "amk2.h"

// Diagnostic IDs
static constexpr uint32_t AMK_CAN_ERR_ID = 3587U;
static constexpr uint32_t AMK_DC_BUS_ID  = 1049U;

// Default limits in ppt (parts per thousand)
static constexpr int16_t AMK_DEFAULT_POS_LIMIT = 2140;
static constexpr int16_t AMK_DEFAULT_NEG_LIMIT = -1;

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
) {
    amk->next_state = AMK_STATE_OFF;
    amk->state = AMK_STATE_OFF;
    amk->set_function = set_func;
    amk->log_function = log_func;
    amk->set = set;
    amk->crit = crit;
    amk->info = info;
    amk->temps = temps;
    amk->err1 = err1;
    amk->err2 = err2;
    amk->precharge_ptr = precharge_ptr;
}

void AMK_reset(AMK_t* amk) {
    amk->set->AMK_Control_bErrorReset = true;
    amk->set->AMK_Control_bInverterOn = false;
    amk->set->AMK_TorqueSetpoint = 0;
}

void AMK_set_torque(AMK_t* amk, int16_t torque_percent) {
    if (torque_percent > 100) torque_percent = 100;
    if (torque_percent < 0) torque_percent = 0;

    // Scale to ppt (parts per thousand)
    amk->set->AMK_TorqueSetpoint = torque_percent * 10;
}

static void AMK_stop(AMK_t* amk) {
    amk->set->AMK_Control_bDcOn = false;
    amk->set->AMK_Control_bInverterOn = false;
    amk->set->AMK_Control_bEnable = false;
    amk->set->AMK_TorqueSetpoint = 0;
    amk->set->AMK_PositiveTorqueLimit = 0;
    amk->set->AMK_NegativeTorqueLimit = AMK_DEFAULT_NEG_LIMIT;
}


void AMK_periodic(AMK_t* amk) {
    amk->state = amk->next_state;
    amk->next_state = amk->state; // default: stay in current state

    bool is_ready = *(amk->precharge_ptr) && amk->info->AMK_Status_bSystemReady;
    bool is_bError = amk->info->AMK_Status_bError;
    bool is_simple_error = (amk->err1->AMK_DiagnosticNumber == AMK_CAN_ERR_ID || 
                            amk->err1->AMK_DiagnosticNumber == AMK_DC_BUS_ID);

    switch(amk->state) {
        case AMK_STATE_OFF:
            if (is_bError && is_simple_error) {
                AMK_reset(amk);
            } else if (!is_bError && is_ready) {
                amk->next_state = AMK_STATE_INIT;
            }
            break;

        case AMK_STATE_INIT:
            if (!is_ready) {
                AMK_stop(amk);
                amk->next_state = AMK_STATE_OFF;
                break;
            }

            amk->set->AMK_TorqueSetpoint = 0;
            amk->set->AMK_PositiveTorqueLimit = 0;
            amk->set->AMK_NegativeTorqueLimit = 0;
            amk->set->AMK_Control_bDcOn = true;
            amk->set->AMK_Control_bInverterOn = true;
            amk->set->AMK_Control_bEnable = true;
            amk->next_state = AMK_STATE_RUNNING;
            break;

        case AMK_STATE_RUNNING:
            if (!is_ready || is_bError) {
                AMK_stop(amk);
                amk->next_state = AMK_STATE_OFF;
                break;
            }

            amk->set->AMK_PositiveTorqueLimit = AMK_DEFAULT_POS_LIMIT;
            amk->set->AMK_NegativeTorqueLimit = AMK_DEFAULT_NEG_LIMIT;
            break;
    }

    // flush the internal state to the CAN library
    if (amk->set_function) amk->set_function();
    if (amk->log_function) amk->log_function();

    // clear error reset
    amk->set->AMK_Control_bErrorReset = false;
}
