#include "pedals.h"
#include "main.h"
#include "common/phal_F4_F7/gpio/gpio.h"
#include "can_parse.h"

pedals_t pedals = {0};
uint16_t thtl_limit = 4096;

pedal_calibration_t pedal_calibration = {.t1max=2000,.t1min=700, // WARNING: DAQ VARIABLE
                                         .t2max=2200,.t2min=800, // IF EEPROM ENABLED,
                                         .b1max=450,.b1min=220, // VALUE WILL CHANGE
                                         .b2max=450,.b2min=420, // 1400, 400
                                         .b3max=124,.b3min=0};   // 910, 812 3312 3436


uint16_t filtered_pedals;       // Acceleration value used on LCD
extern q_handle_t q_tx_can;
bool throttle_latched = false;  // Throttle latched to 0% torque request 

void pedalsPeriodic(void)
{
    // Get current values (don't want them changing mid-calculation)
    uint16_t t1 = raw_adc_values.t1;
    uint16_t t2 = raw_adc_values.t2;
    uint16_t b1 = raw_adc_values.b1;
    uint16_t b2 = raw_adc_values.b2;

    // Check for APPS wiring failure T.4.2.10; Also handles part of implaus check
    setFault(ID_APPS_WIRING_T1_FAULT, t1);
    setFault(ID_APPS_WIRING_T2_FAULT, t2);

    // Check BSE wiring
    setFault(ID_BSE_FAULT, PHAL_readGPIO(BRK_FAIL_TAP_GPIO_Port, BRK_FAIL_TAP_Pin));

    // Convert from raw to scaled output
    float t1_volts = (VREF / 0xFFFU) * t1;
    float t2_volts = (VREF / 0XFFFU) * t2;
    t1 = (t1_volts * RESISTOR_T1) / (VREF - t1_volts);
    t2 = (t2_volts * RESISTOR_T2) / (VREF - t2_volts);

    t1 = CLAMP(t1, pedal_calibration.t1min, pedal_calibration.t1max);
    t2 = CLAMP(t2, pedal_calibration.t2min, pedal_calibration.t2max);
    b1 = CLAMP(b1, pedal_calibration.b1min, pedal_calibration.b1max);
    b2 = CLAMP(b2, pedal_calibration.b2min, pedal_calibration.b2max);
    t1 = (uint16_t) ((((uint32_t) (t1 - pedal_calibration.t1min)) * MAX_PEDAL_MEAS) /
                     (pedal_calibration.t1max - pedal_calibration.t1min));
    t2 = (uint16_t) ((((uint32_t) (t2 - pedal_calibration.t2min)) * MAX_PEDAL_MEAS) /
                     (pedal_calibration.t2max - pedal_calibration.t2min));
    b1 = (uint16_t) ((((uint32_t) (b1 - pedal_calibration.b1min)) * MAX_PEDAL_MEAS) /
                     (pedal_calibration.b1max - pedal_calibration.b1min));
    b2 = (uint16_t) ((((uint32_t) (b2 - pedal_calibration.b2min)) * MAX_PEDAL_MEAS) /
                     (pedal_calibration.b2max - pedal_calibration.b2min));
    
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

    // APPS implaus check: wiring fail or 10% APPS deviation T.4.2.4 (after scaling)
    //setFault(ID_IMPLAUS_DETECTED_FAULT, ((t2>t1)?(t2-t1):(t1-t2)));

    // Cut off throttle when both pressed
    if ((b2 >= APPS_BRAKE_THRESHOLD) && (t2 >= APPS_THROTTLE_FAULT_THRESHOLD))
    {
        throttle_latched = true;
        setFault(ID_APPS_BRAKE_FAULT, true);
    }
    else if (t2 < APPS_THROTTLE_CLEARFAULT_THRESHOLD)
    {
        setFault(ID_APPS_BRAKE_FAULT, false);
    }

    // Throttle will unlatch when released
    if (throttle_latched)
    {
        if (t2 < APPS_THROTTLE_CLEARFAULT_THRESHOLD)
        {
            throttle_latched = false;
        }
    }

    t2 = t2 > thtl_limit ? thtl_limit : t2;

    if (throttle_latched)
    {
        SEND_FILT_THROTTLE_BRAKE(q_tx_can, 0, b2);
    }
    else
    {
        SEND_FILT_THROTTLE_BRAKE(q_tx_can, t2, b2);
    }    
    filtered_pedals = t2;   

    SEND_RAW_THROTTLE_BRAKE(q_tx_can, raw_adc_values.t1, raw_adc_values.t2, raw_adc_values.b1, raw_adc_values.b2, /*raw_adc_values.b3*/0); //no longer use b3
}
