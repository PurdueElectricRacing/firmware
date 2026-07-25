/**
 * @file powertrain.c
 * @brief Powertrain implementation
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "powertrain.h"

#include "can_library/generated/MAIN_MODULE.h"
#include "vehicle_fsm.h"

static powertrain_t g_powertrain;
static INVA_SET_data_t amk_set_front_right;
static INVA_SET_data_t amk_set_front_left;
static INVA_SET_data_t amk_set_rear_left;
static INVA_SET_data_t amk_set_rear_right;

// ! important: assert that the layout hashes of all AMK messages match
static_assert(INVA_SET_LAYOUT_HASH == INVB_SET_LAYOUT_HASH, "AMK INVA/B Struct Mismatch");
static_assert(INVA_SET_LAYOUT_HASH == INVC_SET_LAYOUT_HASH, "AMK INVA/C Struct Mismatch");
static_assert(INVA_SET_LAYOUT_HASH == INVD_SET_LAYOUT_HASH, "AMK INVA/D Struct Mismatch");
static_assert(INVA_CRIT_LAYOUT_HASH == INVB_CRIT_LAYOUT_HASH, "AMK INVA/B Crit Struct Mismatch");
static_assert(INVA_CRIT_LAYOUT_HASH == INVC_CRIT_LAYOUT_HASH, "AMK INVA/C Crit Struct Mismatch");
static_assert(INVA_CRIT_LAYOUT_HASH == INVD_CRIT_LAYOUT_HASH, "AMK INVA/D Crit Struct Mismatch");
static_assert(INVA_INFO_LAYOUT_HASH == INVB_INFO_LAYOUT_HASH, "AMK INVA/B Info Struct Mismatch");
static_assert(INVA_INFO_LAYOUT_HASH == INVC_INFO_LAYOUT_HASH, "AMK INVA/C Info Struct Mismatch");
static_assert(INVA_INFO_LAYOUT_HASH == INVD_INFO_LAYOUT_HASH, "AMK INVA/D Info Struct Mismatch");
static_assert(INVA_ERR_1_LAYOUT_HASH == INVB_ERR_1_LAYOUT_HASH, "AMK INVA/B Err1 Struct Mismatch");
static_assert(INVA_ERR_1_LAYOUT_HASH == INVC_ERR_1_LAYOUT_HASH, "AMK INVA/C Err1 Struct Mismatch");
static_assert(INVA_ERR_1_LAYOUT_HASH == INVD_ERR_1_LAYOUT_HASH, "AMK INVA/D Err1 Struct Mismatch");
static_assert(INVA_ERR_2_LAYOUT_HASH == INVB_ERR_2_LAYOUT_HASH, "AMK INVA/B Err2 Struct Mismatch");
static_assert(INVA_ERR_2_LAYOUT_HASH == INVC_ERR_2_LAYOUT_HASH, "AMK INVA/C Err2 Struct Mismatch");
static_assert(INVA_ERR_2_LAYOUT_HASH == INVD_ERR_2_LAYOUT_HASH, "AMK INVA/D Err2 Struct Mismatch");
static_assert(INVA_TEMPS_LAYOUT_HASH == INVB_TEMPS_LAYOUT_HASH, "AMK INVA/B Temps Struct Mismatch");
static_assert(INVA_TEMPS_LAYOUT_HASH == INVC_TEMPS_LAYOUT_HASH, "AMK INVA/C Temps Struct Mismatch");
static_assert(INVA_TEMPS_LAYOUT_HASH == INVD_TEMPS_LAYOUT_HASH, "AMK INVA/D Temps Struct Mismatch");
static_assert(INVA_PHASE_I_LAYOUT_HASH == INVB_PHASE_I_LAYOUT_HASH, "AMK INVA/B Phase I Struct Mismatch");
static_assert(INVA_PHASE_I_LAYOUT_HASH == INVC_PHASE_I_LAYOUT_HASH, "AMK INVA/C Phase I Struct Mismatch");
static_assert(INVA_PHASE_I_LAYOUT_HASH == INVD_PHASE_I_LAYOUT_HASH, "AMK INVA/D Phase I Struct Mismatch");

void flush_inva(void) {
    CAN_SEND_INVA_SET(
        g_car.front_right.set->AMK_Control_bReserve,
        g_car.front_right.set->AMK_Control_bInverterOn,
        g_car.front_right.set->AMK_Control_bDcOn,
        g_car.front_right.set->AMK_Control_bEnable,
        g_car.front_right.set->AMK_Control_bErrorReset,
        g_car.front_right.set->AMK_Control_bReserve2,
        g_car.front_right.set->AMK_TorqueSetpoint,
        g_car.front_right.set->AMK_PositiveTorqueLimit,
        g_car.front_right.set->AMK_NegativeTorqueLimit
    );
}

void flush_invb(void) {
    CAN_SEND_INVB_SET(
        g_car.front_left.set->AMK_Control_bReserve,
        g_car.front_left.set->AMK_Control_bInverterOn,
        g_car.front_left.set->AMK_Control_bDcOn,
        g_car.front_left.set->AMK_Control_bEnable,
        g_car.front_left.set->AMK_Control_bErrorReset,
        g_car.front_left.set->AMK_Control_bReserve2,
        g_car.front_left.set->AMK_TorqueSetpoint,
        g_car.front_left.set->AMK_PositiveTorqueLimit,
        g_car.front_left.set->AMK_NegativeTorqueLimit
    );
}

void flush_invc(void) {
    CAN_SEND_INVC_SET(
        g_car.rear_left.set->AMK_Control_bReserve,
        g_car.rear_left.set->AMK_Control_bInverterOn,
        g_car.rear_left.set->AMK_Control_bDcOn,
        g_car.rear_left.set->AMK_Control_bEnable,
        g_car.rear_left.set->AMK_Control_bErrorReset,
        g_car.rear_left.set->AMK_Control_bReserve2,
        g_car.rear_left.set->AMK_TorqueSetpoint,
        g_car.rear_left.set->AMK_PositiveTorqueLimit,
        g_car.rear_left.set->AMK_NegativeTorqueLimit
    );
}

void flush_invd(void) {
    CAN_SEND_INVD_SET(
        g_car.rear_right.set->AMK_Control_bReserve,
        g_car.rear_right.set->AMK_Control_bInverterOn,
        g_car.rear_right.set->AMK_Control_bDcOn,
        g_car.rear_right.set->AMK_Control_bEnable,
        g_car.rear_right.set->AMK_Control_bErrorReset,
        g_car.rear_right.set->AMK_Control_bReserve2,
        g_car.rear_right.set->AMK_TorqueSetpoint,
        g_car.rear_right.set->AMK_PositiveTorqueLimit,
        g_car.rear_right.set->AMK_NegativeTorqueLimit
    );
}

void powertrain_init(void) {
    // Inverter A
    AMK_init(
        &g_car.front_right,
        flush_inva,
        (INVA_SET_data_t *) &amk_set_front_right,
        (INVA_CRIT_data_t *) &can_data.INVA_CRIT,
        (INVA_INFO_data_t *) &can_data.INVA_INFO,
        (INVA_TEMPS_data_t *) &can_data.INVA_TEMPS,
        (INVA_ERR_1_data_t *) &can_data.INVA_ERR_1,
        (INVA_ERR_2_data_t *) &can_data.INVA_ERR_2,
        &g_car.is_precharge_complete
    );

    // Inverter B
    AMK_init(
        &g_car.front_left,
        flush_invb,
        (INVA_SET_data_t *) &amk_set_front_left,
        (INVA_CRIT_data_t *) &can_data.INVB_CRIT,
        (INVA_INFO_data_t *) &can_data.INVB_INFO,
        (INVA_TEMPS_data_t *) &can_data.INVB_TEMPS,
        (INVA_ERR_1_data_t *) &can_data.INVB_ERR_1,
        (INVA_ERR_2_data_t *) &can_data.INVB_ERR_2,
        &g_car.is_precharge_complete
    );

    // Inverter C
    AMK_init(
        &g_car.rear_left,
        flush_invc,
        (INVA_SET_data_t *) &amk_set_rear_left,
        (INVA_CRIT_data_t *) &can_data.INVC_CRIT,
        (INVA_INFO_data_t *) &can_data.INVC_INFO,
        (INVA_TEMPS_data_t *) &can_data.INVC_TEMPS,
        (INVA_ERR_1_data_t *) &can_data.INVC_ERR_1,
        (INVA_ERR_2_data_t *) &can_data.INVC_ERR_2,
        &g_car.is_precharge_complete
    );

    // Inverter D
    AMK_init(
        &g_car.rear_right,
        flush_invd,
        (INVA_SET_data_t *)&amk_set_rear_right,
        (INVA_CRIT_data_t *) &can_data.INVD_CRIT,
        (INVA_INFO_data_t *) &can_data.INVD_INFO,
        (INVA_TEMPS_data_t *) &can_data.INVD_TEMPS,
        (INVA_ERR_1_data_t *) &can_data.INVD_ERR_1,
        (INVA_ERR_2_data_t *) &can_data.INVD_ERR_2,
        &g_car.is_precharge_complete
    );
}

static torque_request_t zero_torque_request() {
    torque_request_t torque_request = {
        .front_left  = 0,
        .front_right = 0,
        .rear_left   = 0,
        .rear_right  = 0
    };

    return torque_request;
}

static torque_request_t direct_mapped_regen() {
    // Map brake [0, 100] to torque [0, -100]
    int16_t regen_torque = can_data.pedals.brake * -1.0f;

    torque_request_t torque_request = {
        .front_left  = regen_torque,
        .front_right = regen_torque,
        .rear_left   = regen_torque,
        .rear_right  = regen_torque
    };

    return torque_request;
}

static torque_request_t direct_mapped_throttle() {
    // Map throttle [0, 100] to torque [0, 210]
    int16_t rear_torque = can_data.pedals.throttle * 2.1f;
    
    // Bias to feel like a 40% - 60% torque split
    int16_t front_torque = (40.0f / 60.0f) * rear_torque;

    torque_request_t torque_request = {
        .front_left  = front_torque,
        .front_right = front_torque,
        .rear_left   = rear_torque,
        .rear_right  = rear_torque
    };

    return torque_request;
}


void powertrain_periodic(void) {
    // iterate the AMK fsms
    AMK_periodic(&g_car.front_right);
    AMK_periodic(&g_car.front_left);
    AMK_periodic(&g_car.rear_left);
    AMK_periodic(&g_car.rear_right);
}


void powertrain_flush(void) {
    AMK_set_torque(&g_car.front_right, g_torque_request.front_right);
    AMK_set_torque(&g_car.front_left,  g_torque_request.front_left);
    AMK_set_torque(&g_car.rear_left,   g_torque_request.rear_left);
    AMK_set_torque(&g_car.rear_right,  g_torque_request.rear_right);
}