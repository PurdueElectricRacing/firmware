#include "car.h"


// TODO: car.c
// Check for torque cmd timeout

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
        if (can_data.start_button.start)
        {
            can_data.start_button.start = 0; // debounce
            if (can_data.raw_throttle_brake.brake > BRAKE_PRESSED_THRESHOLD)
            {
                // TODO: & no critical faults on other systems
                PHAL_writeGPIO(SDC_CTRL_GPIO_Port, SDC_CTRL_Pin, true); // close SDC
                // TODO: command precharge
                car.state = CAR_STATE_PRECHARGING;
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
        }
    }
    else if (car.state == CAR_STATE_READY2DRIVE)
    {
        // TODO: check raw torque cmd timeout
        // TODO: check for faults from other systems

    }

}