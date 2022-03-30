#include "car.h"

volatile Car_t car;
extern q_handle_t q_tx_can;
uint32_t buzzer_start_tick = 0;
volatile ADCReadings_t adc_readings;

bool carInit()
{
    car.state = CAR_STATE_INIT;
    PHAL_writeGPIO(SDC_CTRL_GPIO_Port, SDC_CTRL_Pin, 0);
}

void carHeartbeat()
{
    // TODO: precharge state
    SEND_MAIN_STATUS(q_tx_can, car.state, 0, 0);
}

void carPeriodic()
{

    /* State Independent Operations */

    if (can_data.raw_throttle_brake.brake > BRAKE_LIGHT_ON_THRESHOLD)
    {
        if (!car.brake_light)
        {
            PHAL_writeGPIO(BRK_LIGHT_GPIO_Port, BRK_LIGHT_Pin, true);
            car.brake_light = true;
        }
    }
    else if (car.brake_light)
    {
        PHAL_writeGPIO(BRK_LIGHT_GPIO_Port, BRK_LIGHT_Pin, false);
        car.brake_light = false;
    }


    /* State Dependent Operations */

    // EV.10.4 - Activation sequence
    // Tractive System Active - SDC closed, HV outside accumulator
    // Ready to drive - motors respond to APPS input
    //                  not possible unless:
    //                   - tractive system active
    //                   - brake pedal pressed and held
    //                   - start button press

    if (car.state == CAR_STATE_ERROR)
    {
        PHAL_writeGPIO(SDC_CTRL_GPIO_Port, SDC_CTRL_Pin, false); // open SDC
    }
    else if (car.state == CAR_STATE_INIT)
    {
        PHAL_writeGPIO(SDC_CTRL_GPIO_Port, SDC_CTRL_Pin, true); // close SDC
        if (can_data.start_button.start)
        {
            can_data.start_button.start = 0; // debounce
            if (can_data.raw_throttle_brake.brake > BRAKE_PRESSED_THRESHOLD)
            {
                // TODO: & no critical faults on other systems
                // TODO: revert
                // car.state = CAR_STATE_PRECHARGING;
                car.state = CAR_STATE_BUZZING;
            }
        }
    }
    else if (car.state == CAR_STATE_PRECHARGING)
    {
        // TODO: wait for precharge done?
    }
    else if (car.state == CAR_STATE_BUZZING)
    {
        // EV.10.5 - Ready to drive sound
        // 1-3 seconds, unique from other sounds
        if (!PHAL_readGPIO(BUZZER_GPIO_Port, BUZZER_Pin))
        {
            PHAL_writeGPIO(BUZZER_GPIO_Port, BUZZER_Pin, true);
            buzzer_start_tick = sched.os_ticks;
        }
        // stop buzzer
        else if (sched.os_ticks - buzzer_start_tick > BUZZER_DURATION_MS)
        {
            PHAL_writeGPIO(BUZZER_GPIO_Port, BUZZER_Pin, false);
            car.state = CAR_STATE_READY2DRIVE;
            // TODO: temp test
            PHAL_writeGPIO(DT_PUMP_CTRL_GPIO_Port, DT_PUMP_CTRL_Pin, 1);
            PHAL_writeGPIO(DT_RAD_FAN_CTRL_GPIO_Port, DT_RAD_FAN_CTRL_Pin, 1);
            PHAL_writeGPIO(BAT_PUMP_CTRL_GPIO_Port, BAT_PUMP_CTRL_Pin, 1);
            PHAL_writeGPIO(BAT_RAD_FAN_CTRL_GPIO_Port, BAT_RAD_FAN_CTRL_Pin, 1);
        }
    }
    else if (car.state == CAR_STATE_READY2DRIVE)
    {
        // TODO: check raw torque cmd timeout
        // TODO: check for faults from other systems
    }

}

void calcLVCurrent()
{
    uint32_t raw = adc_readings.lv_i_sense;
    car.lv_current_mA = (uint16_t) (raw * 1000 * 1000 * LV_ADC_V_IN_V / 
                        (LV_MAX_ADC_RAW * LV_GAIN * LV_R_SENSE_mOHM));
}