#include "pedals.h"

pedals_t pedals = {0};
volatile raw_pedals_t raw_pedals = {0};

pedal_calibration_t pedal_calibration = {.t1max=1550,.t1min=300, // WARNING: DAQ VARIABLE
                                         .t2max=1550,.t2min=300, // IF EEPROM ENABLED,
                                         .b1max=1000,.b1min=700, // VALUE WILL CHANGE
                                         .b2max=900,.b2min=680, // 1400, 400
                                         .b3max=124,.b3min=0};   // 910, 812 3312 3436

uint16_t b3_buff[8] = {0};
uint8_t b3_idx = 0;

uint16_t b3_offset = 0;
uint32_t b3_start_cal_time = 0;
uint8_t  b3_cal_complete = 0;

extern q_handle_t q_tx_can;

void pedalsPeriodic(void)
{
    // Get current values (don't want them changing mid-calculation)
    uint16_t t1 = raw_pedals.t1;
    uint16_t t2 = raw_pedals.t2;
    uint16_t b1 = raw_pedals.b1;
    uint16_t b2 = raw_pedals.b2;
    uint16_t b3_raw = raw_pedals.b3;

    b3_buff[b3_idx++] = b3_raw;
    b3_idx %= 8;
    uint32_t b3_sum = 0;
    for (uint8_t i = 0; i < 8; i++) b3_sum += b3_buff[i];
    uint16_t b3 = MAX_PEDAL_MEAS - (b3_sum / 8);

    // Calibrate minimum brake pot value after 2 seconds
    if (!b3_cal_complete)
    {
        if (b3_start_cal_time == 0) b3_start_cal_time = sched.os_ticks;
        else if (sched.os_ticks - b3_start_cal_time > 2000)
        {
            b3_cal_complete = 1;
            b3_offset = b3 + 10;
        }
        return;
    }
    // subtract offset, prevent wrap around (uint)
    uint16_t diff = b3 - b3_offset;
    b3 = diff > b3 ? 0 : diff;

    bool apps_wiring_fail = false;

    // Check for APPS wiring failure T.4.2.10
    if (t1 <= APPS_IMPLAUS_MIN || t1 >= APPS_IMPLAUS_MAX ||
        t2 <= APPS_IMPLAUS_MIN || t2 >= APPS_IMPLAUS_MAX)
    {
        apps_wiring_fail = true;
    }

    // Check for BSE wiring failure T.4.3.4
    if (b1 <= BSE_IMPLAUS_MIN || b1 >= BSE_IMPLAUS_MAX ||
        b2 <= BSE_IMPLAUS_MIN || b2 >= BSE_IMPLAUS_MAX ||
        b3 >= BSE_IMPLAUS_MAX) // No min check on b3 since min is 0
    {
        if (!pedals.bse_wiring_fail_detected) pedals.bse_wiring_fail_start_time = sched.os_ticks;
        pedals.bse_wiring_fail_detected = true;
    }
    else
    {
        pedals.bse_wiring_fail_detected = false;
        pedals.bse_faulted = false;
    }

    // Scale values based on min and max
    t1 = CLAMP(t1, pedal_calibration.t1min, pedal_calibration.t1max);
    t2 = CLAMP(t2, pedal_calibration.t2min, pedal_calibration.t2max);
    b1 = CLAMP(b1, pedal_calibration.b1min, pedal_calibration.b1max);
    b2 = CLAMP(b2, pedal_calibration.b2min, pedal_calibration.b2max);
    b3 = CLAMP(b3, pedal_calibration.b3min, pedal_calibration.b3max);
    t1 = (uint16_t) ((((uint32_t) (t1 - pedal_calibration.t1min)) * MAX_PEDAL_MEAS) /
                     (pedal_calibration.t1max - pedal_calibration.t1min));
    t2 = (uint16_t) ((((uint32_t) (t2 - pedal_calibration.t2min)) * MAX_PEDAL_MEAS) /
                     (pedal_calibration.t2max - pedal_calibration.t2min));
    b1 = (uint16_t) ((((uint32_t) (b1 - pedal_calibration.b1min)) * MAX_PEDAL_MEAS) /
                     (pedal_calibration.b1max - pedal_calibration.b1min));
    b2 = (uint16_t) ((((uint32_t) (b2 - pedal_calibration.b2min)) * MAX_PEDAL_MEAS) /
                     (pedal_calibration.b2max - pedal_calibration.b2min));
    b3 = (uint16_t) ((((uint32_t) (b3 - pedal_calibration.b3min)) * MAX_PEDAL_MEAS) /
                     (pedal_calibration.b3max - pedal_calibration.b3min));
    // Invert
    // t1 = MAX_PEDAL_MEAS - t1;
    // t2 = MAX_PEDAL_MEAS - t2;

        // Mask
    t1 &= 0xFFFC;
    t2 &= 0xFFFC;
    b1 &= 0xFFFC;
    b2 &= 0xFFFC;
    b3 &= 0xFFFC;


    // APPS implaus check: wiring fail or 10% APPS deviation T.4.2.4 (after scaling)
    if (apps_wiring_fail || ((t2>t1)?(t2-t1):(t1-t2)) >= APPS_IMPLAUS_MAX_DIFF)
    {
        if (!pedals.apps_implaus_detected) pedals.apps_implaus_start_time = sched.os_ticks;
        pedals.apps_implaus_detected = true;
    }
    else
    {
        pedals.apps_implaus_detected = false;
        pedals.apps_faulted = false;
    }

    // If APPS implaus occurs > 100ms, set motor power to 0 T.4.2.5
    // Not necessary to open SDC
    if (pedals.apps_implaus_detected &&
        sched.os_ticks - pedals.apps_implaus_start_time > APPS_IMPLAUS_TIME_MS)
    {
        pedals.apps_faulted = true;
    }

    // If BSE wiring fail occurs > 100ms, set motor power to 0 T.4.3.3
    // Not necessary to open SDC
    if (pedals.bse_wiring_fail_detected &&
        sched.os_ticks - pedals.bse_wiring_fail_start_time > APPS_IMPLAUS_TIME_MS)
    {
        pedals.bse_faulted = true;
    }

    // APPS Brake Plaus Check EV.5.7
    if (!pedals.apps_brake_faulted)
    {
        if (b2 >= APPS_BRAKE_THRESHOLD &&
            t1 >= APPS_THROTTLE_FAULT_THRESHOLD)
        {
            pedals.apps_brake_faulted = true;
        }
    }
    else if (t1 <= APPS_THROTTLE_CLEARFAULT_THRESHOLD)
    {
        pedals.apps_brake_faulted = false;
    }

    if (pedals.apps_faulted || pedals.bse_faulted || pedals.apps_brake_faulted)
    {
        t1 = 0;
    }
    SEND_RAW_THROTTLE_BRAKE(q_tx_can, t1, b3);
}