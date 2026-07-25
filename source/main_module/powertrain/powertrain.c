/**
 * @file powertrain.c
 * @brief Powertrain implementation
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "powertrain.h"

#include "can_library/faults_common.h"
#include "can_library/generated/MAIN_MODULE.h"
#include "common/utils/min.h"

powertrain_t g_powertrain;
static INVA_SET_data_t amk_set_front_right;
static INVA_SET_data_t amk_set_front_left;
static INVA_SET_data_t amk_set_rear_left;
static INVA_SET_data_t amk_set_rear_right;

// For speed calculations
static constexpr float WHEEL_RADIUS_IN = 8.0f;
static constexpr float GEAR_RATIO = 12.51f;
static constexpr float WHEEL_CIRCUMFERENCE_IN = 2.0f * 3.14159f * WHEEL_RADIUS_IN;
static constexpr float OUTPUT_REV_PER_MOTOR_REV = 1.0f / GEAR_RATIO;
static constexpr float INCHES_PER_MOTOR_REV = WHEEL_CIRCUMFERENCE_IN * OUTPUT_REV_PER_MOTOR_REV;
static constexpr float MINUTES_PER_HOUR = 60.0f;
static constexpr float INCHES_PER_MILE = 63360.0f;
static constexpr float RPM_TO_MPH = INCHES_PER_MOTOR_REV * MINUTES_PER_HOUR / INCHES_PER_MILE;

// The vehicle FSM calls the powertrain often enough to flush every inverter setpoint.
static_assert(POWERTRAIN_PERIOD_MS == INVA_SET_PERIOD_MS);
static_assert(POWERTRAIN_PERIOD_MS == INVB_SET_PERIOD_MS);
static_assert(POWERTRAIN_PERIOD_MS == INVC_SET_PERIOD_MS);
static_assert(POWERTRAIN_PERIOD_MS == INVD_SET_PERIOD_MS);

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

static void flush_inva(void) {
    CAN_SEND_INVA_SET(
        g_powertrain.front_right.set->AMK_Control_bReserve,
        g_powertrain.front_right.set->AMK_Control_bInverterOn,
        g_powertrain.front_right.set->AMK_Control_bDcOn,
        g_powertrain.front_right.set->AMK_Control_bEnable,
        g_powertrain.front_right.set->AMK_Control_bErrorReset,
        g_powertrain.front_right.set->AMK_Control_bReserve2,
        g_powertrain.front_right.set->AMK_TorqueSetpoint,
        g_powertrain.front_right.set->AMK_PositiveTorqueLimit,
        g_powertrain.front_right.set->AMK_NegativeTorqueLimit
    );
}

static void flush_invb(void) {
    CAN_SEND_INVB_SET(
        g_powertrain.front_left.set->AMK_Control_bReserve,
        g_powertrain.front_left.set->AMK_Control_bInverterOn,
        g_powertrain.front_left.set->AMK_Control_bDcOn,
        g_powertrain.front_left.set->AMK_Control_bEnable,
        g_powertrain.front_left.set->AMK_Control_bErrorReset,
        g_powertrain.front_left.set->AMK_Control_bReserve2,
        g_powertrain.front_left.set->AMK_TorqueSetpoint,
        g_powertrain.front_left.set->AMK_PositiveTorqueLimit,
        g_powertrain.front_left.set->AMK_NegativeTorqueLimit
    );
}

static void flush_invc(void) {
    CAN_SEND_INVC_SET(
        g_powertrain.rear_left.set->AMK_Control_bReserve,
        g_powertrain.rear_left.set->AMK_Control_bInverterOn,
        g_powertrain.rear_left.set->AMK_Control_bDcOn,
        g_powertrain.rear_left.set->AMK_Control_bEnable,
        g_powertrain.rear_left.set->AMK_Control_bErrorReset,
        g_powertrain.rear_left.set->AMK_Control_bReserve2,
        g_powertrain.rear_left.set->AMK_TorqueSetpoint,
        g_powertrain.rear_left.set->AMK_PositiveTorqueLimit,
        g_powertrain.rear_left.set->AMK_NegativeTorqueLimit
    );
}

static void flush_invd(void) {
    CAN_SEND_INVD_SET(
        g_powertrain.rear_right.set->AMK_Control_bReserve,
        g_powertrain.rear_right.set->AMK_Control_bInverterOn,
        g_powertrain.rear_right.set->AMK_Control_bDcOn,
        g_powertrain.rear_right.set->AMK_Control_bEnable,
        g_powertrain.rear_right.set->AMK_Control_bErrorReset,
        g_powertrain.rear_right.set->AMK_Control_bReserve2,
        g_powertrain.rear_right.set->AMK_TorqueSetpoint,
        g_powertrain.rear_right.set->AMK_PositiveTorqueLimit,
        g_powertrain.rear_right.set->AMK_NegativeTorqueLimit
    );
}

void powertrain_init(void) {
    // Inverter A
    const AMK_config_t front_right_config = {
        .flush_function = flush_inva,
        .set            = (INVA_SET_data_t *)&amk_set_front_right,
        .crit           = (INVA_CRIT_data_t *)&can_data.INVA_CRIT,
        .info           = (INVA_INFO_data_t *)&can_data.INVA_INFO,
        .temps          = (INVA_TEMPS_data_t *)&can_data.INVA_TEMPS,
        .err1           = (INVA_ERR_1_data_t *)&can_data.INVA_ERR_1,
        .err2           = (INVA_ERR_2_data_t *)&can_data.INVA_ERR_2,
    };
    AMK_init(&g_powertrain.front_right, &front_right_config);

    // Inverter B
    const AMK_config_t front_left_config = {
        .flush_function = flush_invb,
        .set            = (INVA_SET_data_t *)&amk_set_front_left,
        .crit           = (INVA_CRIT_data_t *)&can_data.INVB_CRIT,
        .info           = (INVA_INFO_data_t *)&can_data.INVB_INFO,
        .temps          = (INVA_TEMPS_data_t *)&can_data.INVB_TEMPS,
        .err1           = (INVA_ERR_1_data_t *)&can_data.INVB_ERR_1,
        .err2           = (INVA_ERR_2_data_t *)&can_data.INVB_ERR_2,
    };
    AMK_init(&g_powertrain.front_left, &front_left_config);

    // Inverter C
    const AMK_config_t rear_left_config = {
        .flush_function = flush_invc,
        .set            = (INVA_SET_data_t *)&amk_set_rear_left,
        .crit           = (INVA_CRIT_data_t *)&can_data.INVC_CRIT,
        .info           = (INVA_INFO_data_t *)&can_data.INVC_INFO,
        .temps          = (INVA_TEMPS_data_t *)&can_data.INVC_TEMPS,
        .err1           = (INVA_ERR_1_data_t *)&can_data.INVC_ERR_1,
        .err2           = (INVA_ERR_2_data_t *)&can_data.INVC_ERR_2,
    };
    AMK_init(&g_powertrain.rear_left, &rear_left_config);

    // Inverter D
    const AMK_config_t rear_right_config = {
        .flush_function = flush_invd,
        .set            = (INVA_SET_data_t *)&amk_set_rear_right,
        .crit           = (INVA_CRIT_data_t *)&can_data.INVD_CRIT,
        .info           = (INVA_INFO_data_t *)&can_data.INVD_INFO,
        .temps          = (INVA_TEMPS_data_t *)&can_data.INVD_TEMPS,
        .err1           = (INVA_ERR_1_data_t *)&can_data.INVD_ERR_1,
        .err2           = (INVA_ERR_2_data_t *)&can_data.INVD_ERR_2,
    };
    AMK_init(&g_powertrain.rear_right, &rear_right_config);
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

static torque_request_t torque_vectoring_request() {
    torque_request_t torque_request = {
        .front_left  = can_data.vcu_torque_request.front_left,
        .front_right = can_data.vcu_torque_request.front_right,
        .rear_left   = can_data.vcu_torque_request.rear_left,
        .rear_right  = can_data.vcu_torque_request.rear_right
    };

    return torque_request;
}

void powertrain_update_torque_request(void) {
    if (can_data.pedals.is_stale()) {
        g_powertrain.torque_request = zero_torque_request();
        return;
    }

    bool is_tv_stale = can_data.vcu_settings.is_stale() || can_data.vcu_torque_request.is_stale();
    if (!is_tv_stale && can_data.vcu_settings.is_tv_enabled) {
        g_powertrain.torque_request = torque_vectoring_request();
        return;
    }

    bool is_braking = can_data.pedals.brake > 5;
    int16_t min_wheelspeed = MINOF(
        g_powertrain.front_right.crit->AMK_ActualSpeed,
        g_powertrain.front_left.crit->AMK_ActualSpeed,
        g_powertrain.rear_left.crit->AMK_ActualSpeed,
        g_powertrain.rear_right.crit->AMK_ActualSpeed
    );
    bool is_vehicle_speed_high = min_wheelspeed * RPM_TO_MPH > 5;
    bool is_pack_low_enough = can_data.pack_stats.pack_voltage < 470;
    bool is_regen_allowed = is_braking && is_vehicle_speed_high && is_pack_low_enough;

    if (can_data.pedals.throttle > 0) {
        g_powertrain.torque_request = direct_mapped_throttle();
    } else if (is_regen_allowed) {
        g_powertrain.torque_request = direct_mapped_regen();
    } else {
        g_powertrain.torque_request = zero_torque_request();
    }
}

void powertrain_periodic(void) {
    // stage torque requests to the AMKs
    AMK_set_torque(&g_powertrain.front_right, g_powertrain.torque_request.front_right);
    AMK_set_torque(&g_powertrain.front_left,  g_powertrain.torque_request.front_left);
    AMK_set_torque(&g_powertrain.rear_left,   g_powertrain.torque_request.rear_left);
    AMK_set_torque(&g_powertrain.rear_right,  g_powertrain.torque_request.rear_right);

    // iterate the AMK fsms and flush the setpoints to the CAN bus
    bool is_precharge_complete = is_clear(FAULT_ID_PRECHARGE_INCOMPLETE);
    AMK_periodic(&g_powertrain.front_right, is_precharge_complete);
    AMK_periodic(&g_powertrain.front_left, is_precharge_complete);
    AMK_periodic(&g_powertrain.rear_left, is_precharge_complete);
    AMK_periodic(&g_powertrain.rear_right, is_precharge_complete);
}

void powertrain_zero_torque_request(void) {
    g_powertrain.torque_request = zero_torque_request();
}

bool is_powertrain_ready(void) {
    return g_powertrain.front_right.state == AMK_STATE_RUNNING
        && g_powertrain.front_left.state  == AMK_STATE_RUNNING
        && g_powertrain.rear_left.state   == AMK_STATE_RUNNING
        && g_powertrain.rear_right.state  == AMK_STATE_RUNNING;
}
