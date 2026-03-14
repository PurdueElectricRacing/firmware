#include "race.h"

#include "nextion.h"
#include "common/can_library/generated/DASHBOARD.h"
#include "pedals.h"
#include "common_defs.h"

// For speed calcs
constexpr float WHEEL_RADIUS_IN = 8.0f;
constexpr float GEAR_RATIO = 12.51f;

constexpr float WHEEL_CIRCUMFERENCE_IN = 2.0f * PI * WHEEL_RADIUS_IN;
constexpr float OUTPUT_REV_PER_MOTOR_REV = 1.0f / GEAR_RATIO;
constexpr float INCHES_PER_MOTOR_REV = WHEEL_CIRCUMFERENCE_IN * OUTPUT_REV_PER_MOTOR_REV;

constexpr float MINUTES_PER_HOUR = 60.0f;
constexpr float INCHES_PER_MILE = 63360.0f;

static constexpr float RPM_TO_MPH = INCHES_PER_MOTOR_REV * MINUTES_PER_HOUR / INCHES_PER_MILE;


static inline void update_car_state_telemetry() {
    if (can_data.main_hb.stale) {
        NXT_setText(CAR_STATE, "STALE");
        NXT_setFontColor(CAR_STATE, WHITE);
        return;
    }

    switch (can_data.main_hb.car_state) {
        case CAR_STATE_INIT:
            NXT_setFontColor(CAR_STATE, WHITE);
            NXT_setText(CAR_STATE, "INIT");
            NXT_setBorderColor(CAR_STATE, WHITE);
            break;
        case CAR_STATE_IDLE:
            NXT_setFontColor(CAR_STATE, WHITE);
            NXT_setText(CAR_STATE, "IDLE");
            NXT_setBorderColor(CAR_STATE, WHITE);
            break;
        case CAR_STATE_PRECHARGING:
            NXT_setFontColor(CAR_STATE, YELLOW);
            NXT_setText(CAR_STATE, "PRECHRG");
            NXT_setBorderColor(CAR_STATE, YELLOW);
            break;
        case CAR_STATE_ENERGIZED:
            NXT_setFontColor(CAR_STATE, GREEN);
            NXT_setText(CAR_STATE, "ENERGZD");
            NXT_setBorderColor(CAR_STATE, GREEN);
            break;
        case CAR_STATE_BUZZING:
            NXT_setFontColor(CAR_STATE, YELLOW);
            NXT_setText(CAR_STATE, "BUZZING");
            NXT_setBorderColor(CAR_STATE, YELLOW);
            break;
        case CAR_STATE_READY2DRIVE:
            NXT_setFontColor(CAR_STATE, GREEN);
            NXT_setText(CAR_STATE, "R2D");
            NXT_setBorderColor(CAR_STATE, GREEN);
            break;
        case CAR_STATE_FATAL:
            NXT_setFontColor(CAR_STATE, RED);
            NXT_setText(CAR_STATE, "FATAL");
            NXT_setBorderColor(CAR_STATE, RED);
            break;
        default:
            NXT_setFontColor(CAR_STATE, WHITE);
            NXT_setText(CAR_STATE, "UNKNOWN");
            NXT_setBorderColor(CAR_STATE, WHITE);
            break;
    }
}

static inline void update_motor_telemetry() {
    if (can_data.motor_temps.stale) {
        NXT_setText(MOTOR_TEMP, "S");
    } else {
        int16_t max_motor_temp = MAX4(
            can_data.motor_temps.front_right,
            can_data.motor_temps.front_left,
            can_data.motor_temps.rear_left,
            can_data.motor_temps.rear_right
        );

        int16_t scaled_motor_temp = max_motor_temp * UNPACK_COEFF_MOTOR_TEMPS_FRONT_RIGHT;
        NXT_setTextFormatted(MOTOR_TEMP, "%dC", scaled_motor_temp);
    }
}

static inline void update_igbt_telemetry() {
    if (can_data.igbt_temps.stale) {
        NXT_setText(IGBT_TEMP, "S");
    } else {
        int16_t max_igbt_temp = MAX4(
            can_data.igbt_temps.front_right,
            can_data.igbt_temps.front_left,
            can_data.igbt_temps.rear_left,
            can_data.igbt_temps.rear_right
        );

        int16_t scaled_igbt_temp = max_igbt_temp * UNPACK_COEFF_IGBT_TEMPS_FRONT_RIGHT;
        NXT_setTextFormatted(IGBT_TEMP, "%dC", scaled_igbt_temp);
    }
}

static inline void update_pack_telemetry() {
    if (can_data.pack_stats.stale) {
        NXT_setText(BATT_VOLT, "S");
        NXT_setText(BATT_CURR, "S");
        NXT_setText(BATT_TEMP, "S");
    } else {
        uint16_t scaled_voltage = can_data.pack_stats.pack_voltage * UNPACK_COEFF_PACK_STATS_PACK_VOLTAGE;
        int16_t scaled_current = can_data.pack_stats.pack_current * UNPACK_COEFF_PACK_STATS_PACK_CURRENT;
        NXT_setTextFormatted(BATT_VOLT, "%dV", scaled_voltage);
        NXT_setTextFormatted(BATT_CURR, "%dA", scaled_current);
        NXT_setTextFormatted(BATT_TEMP, "%dC", can_data.pack_stats.avg_temp);
    }
}

// todo better speed calc lol
static inline void update_speed_telemetry() {
    if (can_data.wheel_speeds.stale) {
        NXT_setText(SPEED, "S");
    } else {
        if (can_data.wheel_speeds.rear_left < 0) {
            NXT_setText(SPEED, "NEG");
        } else {
            int16_t max_wheelspeed = MAX4(
                can_data.wheel_speeds.front_right,
                can_data.wheel_speeds.front_left,
                can_data.wheel_speeds.rear_left,
                can_data.wheel_speeds.rear_right
            );

            int16_t mph = (int16_t)(max_wheelspeed * RPM_TO_MPH);
            NXT_setTextFormatted(SPEED, "%d", mph);
        }
    }
}

static inline void update_pedal_telemetry() {
    NXT_setValue(BRK_BAR, (int)((pedal_values.brake / 4095.0) * 100));
    NXT_setValue(THROT_BAR, (int)((pedal_values.throttle / 4095.0) * 100));
}

/**
 * @brief Updates telemetry data on the race dashboard LCD display
 *
 * Only updates on race page. Displays 'S' for stale values.
 */
void race_telemetry_update() {
    update_pedal_telemetry();
    update_car_state_telemetry();
    update_motor_telemetry();
    update_igbt_telemetry();
    update_pack_telemetry();
    update_speed_telemetry();
}