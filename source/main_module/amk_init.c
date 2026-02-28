#include "main.h"
#include "common/can_library/generated/MAIN_MODULE.h"

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

void flush_inva() {
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

void flush_invb() {
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

void flush_invc() {
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

void flush_invd() {
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

void init_amks() {
    // Inverter A
    AMK_init(
        &g_car.front_right,
        flush_inva,
        (INVA_SET_data_t *) &can_data.INVA_SET,
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
        (INVA_SET_data_t *) &can_data.INVB_SET,
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
        (INVA_SET_data_t *) &can_data.INVC_SET,
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
        (INVA_SET_data_t *) &can_data.INVD_SET,
        (INVA_CRIT_data_t *) &can_data.INVD_CRIT,
        (INVA_INFO_data_t *) &can_data.INVD_INFO,
        (INVA_TEMPS_data_t *) &can_data.INVD_TEMPS,
        (INVA_ERR_1_data_t *) &can_data.INVD_ERR_1,
        (INVA_ERR_2_data_t *) &can_data.INVD_ERR_2,
        &g_car.is_precharge_complete
    );
}

void car_init() {
    // enter INIT at n_reset
    g_car.current_state = CARSTATE_INIT;
    g_car.next_state    = CARSTATE_INIT;
    init_amks();
}