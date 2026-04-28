#include "cooling_bangbang.h"

#if COOLING_ENABLE_BANGBANG

#include "common/bangbang/bangbang.h"
#include "can_library/generated/PDU.h"

static constexpr float MOTOR_PUMP_ON_TEMP_C = 60.0f;
static constexpr float MOTOR_PUMP_OFF_TEMP_C = 55.0f;
static constexpr float MOTOR_FAN_ON_TEMP_C = 100.0f;
static constexpr float MOTOR_FAN_OFF_TEMP_C = 95.0f;
static constexpr float MOTOR_FAN_STOP_ON_TEMP_C = 60.0f;
static constexpr uint32_t BANGBANG_MIN_SWITCH_MS = 1000;

static pdu_cooling_command_t *g_active_command;

static void set_pump2_on(void) {
    g_active_command->pump2_enabled = true;
}

static void set_pump2_off(void) {
    g_active_command->pump2_enabled = false;
}

static void set_fan2_on(void) {
    g_active_command->fan2_enabled = true;
    g_active_command->fan2_percent = 100;
}

static void set_fan2_off(void) {
    g_active_command->fan2_enabled = false;
    g_active_command->fan2_percent = 0;
}

INIT_BANG_BANG(
    motor_pump_controller,
    MOTOR_PUMP_ON_TEMP_C,
    MOTOR_PUMP_OFF_TEMP_C,
    set_pump2_on,
    set_pump2_off,
    BANGBANG_MIN_SWITCH_MS
)

INIT_BANG_BANG(
    motor_fan_controller,
    MOTOR_FAN_ON_TEMP_C,
    MOTOR_FAN_OFF_TEMP_C,
    set_fan2_on,
    set_fan2_off,
    BANGBANG_MIN_SWITCH_MS
)

static float cooling_hottest_motor_temp_c(void) {
    float hottest = can_data.motor_temps.front_left;

    if (can_data.motor_temps.front_right > hottest) {
        hottest = can_data.motor_temps.front_right;
    }

    if (can_data.motor_temps.rear_left > hottest) {
        hottest = can_data.motor_temps.rear_left;
    }

    if (can_data.motor_temps.rear_right > hottest) {
        hottest = can_data.motor_temps.rear_right;
    }

    return hottest;
}

static bool cooling_not_moving(void) {
    if (can_data.main_hb.is_stale()) {
        return true;
    }

    return can_data.main_hb.car_state != CAR_STATE_READY2DRIVE;
}

bool cooling_bangbang_is_enabled(void) {
    return true;
}

void cooling_bangbang_init(void) {
    motor_pump_controller.is_on = false;
    motor_fan_controller.is_on = false;
    motor_pump_controller.last_switch_ms = 0;
    motor_fan_controller.last_switch_ms = 0;
}

void cooling_bangbang_update(pdu_cooling_command_t *cooling_command) {
    if (cooling_command == nullptr) {
        return;
    }

    g_active_command = cooling_command;

    if (can_data.motor_temps.is_stale()) {
        return;
    }

    float motor_temp_c = cooling_hottest_motor_temp_c();
    uint32_t now_ms = OS_TICKS;

    bangbang_update(&motor_pump_controller, motor_temp_c, now_ms);

    bool not_moving = cooling_not_moving();
    if (not_moving) {
        bangbang_update(&motor_fan_controller, motor_temp_c, now_ms);
    } else if (motor_temp_c < MOTOR_FAN_STOP_ON_TEMP_C) {
        motor_fan_controller.last_switch_ms = now_ms;
        set_fan2_off();
    } else {
        bangbang_update(&motor_fan_controller, motor_temp_c, now_ms);
    }
}

#else

bool cooling_bangbang_is_enabled(void) {
    return false;
}

void cooling_bangbang_init(void) {
}

void cooling_bangbang_update(pdu_cooling_command_t *cooling_command) {
    (void)cooling_command;
}

#endif
