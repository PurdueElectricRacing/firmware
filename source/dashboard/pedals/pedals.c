#include "pedals.h"
#include "main.h"
#include "common/phal_L4/gpio/gpio.h"
#include "can_parse.h"

pedals_t pedals = {0};
uint16_t thtl_limit = 4096;

pedal_calibration_t pedal_calibration = {.t1max=2015,.t1min=785, // WARNING: DAQ VARIABLE
                                         .t2max=1920,.t2min=550, // IF EEPROM ENABLED,
                                         .b1max=1200,.b1min=450, // VALUE WILL CHANGE
                                         .b2max=1050,.b2min=425, // 1400, 400
                                         .b3max=124,.b3min=0};   // 910, 812 3312 3436

uint16_t b3_buff[8] = {0};
uint16_t t1_buff[10] = {0};
uint16_t t2_buff[10] = {0};
uint16_t b1_buff[10] = {0};
uint16_t b2_buff[10] = {0};
uint8_t b3_idx = 0;
uint8_t t1_idx = 0;
uint8_t t2_idx = 0;
uint8_t b1_idx = 0;
uint8_t b2_idx = 0;

uint16_t filtered_pedals;

uint16_t b3_offset = 0;
uint32_t b3_start_cal_time = 0;
uint8_t  b3_cal_complete = 0;

extern q_handle_t q_tx_can;

void pedalsPeriodic(void)
{
    // Get current values (don't want them changing mid-calculation)
    uint16_t t1 = raw_adc_values.t1;
    uint16_t t2 = raw_adc_values.t2;
    uint16_t b1 = raw_adc_values.b1;
    uint16_t b2 = raw_adc_values.b2;
    uint16_t b3_raw = raw_adc_values.b3;

    b3_buff[b3_idx++] = b3_raw;
    b3_idx %= 8;
    uint32_t b3_sum = 0;
    for (uint8_t i = 0; i < 8; i++) b3_sum += b3_buff[i];
    uint16_t b3 = MAX_PEDAL_MEAS - (b3_sum / 8);

    //3.3R2/(R2 - R1)

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

    // bool apps_wiring_fail = false;

    // Check for APPS wiring failure T.4.2.10; Also handles part of implaus check
    // if ( /* t1 <= APPS_IMPLAUS_MIN || t1 >= APPS_IMPLAUS_MAX || */
    //     t2 <= APPS_IMPLAUS_MIN || t2 >= APPS_IMPLAUS_MAX)
    // {
    //     apps_wiring_fail = true;
    // }
    setFault(ID_APPS_WIRING_T1_FAULT, t1);
    setFault(ID_APPS_WIRING_T2_FAULT, t2);

    // Check for BSE wiring failure T.4.3.4
    // if (b1 <= BSE_IMPLAUS_MIN || b1 >= BSE_IMPLAUS_MAX ||
    //     b2 <= BSE_IMPLAUS_MIN || b2 >= BSE_IMPLAUS_MAX ||
    //     b3 >= BSE_IMPLAUS_MAX) // No min check on b3 since min is 0
    // {
    //     if (!pedals.bse_wiring_fail_detected) pedals.bse_wiring_fail_start_time = sched.os_ticks;
    //     pedals.bse_wiring_fail_detected = true;
    // }
    // else
    // {
    //     pedals.bse_wiring_fail_detected = false;
    //     pedals.bse_faulted = false;
    // }

    // setFault(ID_BSE_WIRING_B1_FAULT, b1);
    // setFault(ID_BSE_WIRING_B2_FAULT, b2);
    setFault(ID_BSE_FAULT, PHAL_readGPIO(BRK_FAIL_TAP_GPIO_Port, BRK_FAIL_TAP_Pin));
    if (PHAL_readGPIO(BRK_STAT_TAP_GPIO_Port, BRK_STAT_TAP_Pin)) {
        setFault(ID_BSPD_FAULT, can_data.orion_currents_volts.pack_current);
    }
    else {
        setFault(ID_BSPD_FAULT, 0);
    }

    float t1_volts = (VREF / 0xFFFU) * t1;
    float t2_volts = (VREF / 0XFFFU) * t2;

    t1 = (t1_volts * RESISTOR_T1) / (VREF - t1_volts);
    t2 = (t2_volts * RESISTOR_T2) / (VREF - t2_volts);

    t1_buff[t1_idx++] = t1;
    t2_buff[t2_idx++] = t2;
    b1_buff[b1_idx++] = b1;
    b2_buff[b2_idx++] = b2;

    t1_idx = t1_idx % 10;
    t2_idx = t2_idx % 10;
    b1_idx %= 10;
    b2_idx %= 10;

    uint32_t t1_avg = 0;
    uint32_t t2_avg = 0;
    uint32_t b1_avg = 0;
    uint32_t b2_avg = 0;

    for (uint8_t i = 0; i < 10; i++) {
        t1_avg += t1_buff[i];
        t2_avg += t2_buff[i];
        b1_avg += b1_buff[i];
        b2_avg += b2_buff[i];
    }

    t1 =  (uint16_t) (t1_avg / 10);
    t2 = (uint16_t) (t2_avg / 10);
    b1 = (uint16_t) (b1_avg / 10);
    b2 = (uint16_t) (b2_avg / 10);



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
    // t1 &= 0xFFFC;
    // t2 &= 0xFFFC;
    // b1 &= 0xFFFC;
    // b2 &= 0xFFFC;
    b3 &= 0xFFFC;


    // APPS implaus check: wiring fail or 10% APPS deviation T.4.2.4 (after scaling)

    //UNCOMMENT once both throttles work
    // if (apps_wiring_fail  ||  ((t2>t1)?(t2-t1):(t1-t2)) >= APPS_IMPLAUS_MAX_DIFF )
    // {
    //     if (!pedals.apps_implaus_detected) pedals.apps_implaus_start_time = sched.os_ticks;
    //     pedals.apps_implaus_detected = true;
    // }
    // else
    // {
    //     pedals.apps_implaus_detected = false;
    //     pedals.apps_faulted = false;
    // }
    setFault(ID_IMPLAUS_DETECTED_FAULT, ((t2>t1)?(t2-t1):(t1-t2)));

    //The following are commented as this is now handled by the fault library
    // If APPS implaus occurs > 100ms, set motor power to 0 T.4.2.5
    // Not necessary to open SDC
    // if (pedals.apps_implaus_detected &&
    //     sched.os_ticks - pedals.apps_implaus_start_time > APPS_IMPLAUS_TIME_MS)
    // {
    //     pedals.apps_faulted = true;
    // }

    // If BSE wiring fail occurs > 100ms, set motor power to 0 T.4.3.3
    // Not necessary to open SDC
    // if (pedals.bse_wiring_fail_detected &&
    //     sched.os_ticks - pedals.bse_wiring_fail_start_time > APPS_IMPLAUS_TIME_MS)
    // {
    //     pedals.bse_faulted = true;
    // }

    // APPS Brake Plaus Check EV.5.7
    // if (!pedals.apps_brake_faulted)
    // {
    //     if (b2 >= APPS_BRAKE_THRESHOLD &&
    //         t2 >= APPS_THROTTLE_FAULT_THRESHOLD)
    //     {
    //         pedals.apps_brake_faulted = true;
    //     }
    // }
    // else if (t2 <= APPS_THROTTLE_CLEARFAULT_THRESHOLD)
    // {
    //     pedals.apps_brake_faulted = false;
    // }
    if (b2 >= APPS_BRAKE_THRESHOLD &&
        t2 >= APPS_THROTTLE_FAULT_THRESHOLD)
    {
        // setFault(ID_APPS_BRAKE_FAULT, true);
    }
    else if (t2 <= APPS_THROTTLE_CLEARFAULT_THRESHOLD)
    {
        // setFault(ID_APPS_BRAKE_FAULT, false);
    }





    //Fault States detected by Main Module, which will exit ready2drive
    // if (pedals.apps_faulted || pedals.bse_faulted || pedals.apps_brake_faulted)
    // {
    //     t2 = 0;
    // }
    t2 = t2 > thtl_limit ? thtl_limit : t2;
    filtered_pedals = t2;

    SEND_RAW_THROTTLE_BRAKE(q_tx_can, raw_adc_values.t1,
                            raw_adc_values.t2, raw_adc_values.b1,
                            raw_adc_values.b2, raw_adc_values.b3);
    SEND_FILT_THROTTLE_BRAKE(q_tx_can, t2, b2);


}
