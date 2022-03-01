#include "car.h"


// TODO: car.c
// Check for torque cmd timeout
// Check torque cmd doesn't exceed request

volatile Car_t car;
extern q_handle_t q_tx_can;
uint32_t buzzer_start_tick = 0;

bool carInit()
{
    car.state = CAR_STATE_INIT;

}

void carHeartbeat()
{

}

void carPeriodic()
{

    /* State Independent Operations */

    // EV.5.7 - APPS plausibility check
    // Shutdown motor pwr if brakes on & APPS > 25% pedal travel
    // Reset once APPS < 5% pedal travel, w/ or w/o brakes


    if (can_data.)


    /* State Dependent Operations */

    // EV.10.4 - Activation sequence
    // Tractive System Active - SDC closed, HV outside accumulator
    // Ready to drive - motors respond to APPS input
    //                  not possible unless:
    //                   - tractive system active
    //                   - brake pedal pressed and held
    //                   - start button press


    if (car.state == CAR_STATE_INIT)
    {
        PHAL_writeGPIO(SDC_CTRL_GPIO_Port, SDC_CTRL_Pin, true); // close SDC
        // TODO: check for preready2drive
    }
    else if (car.state == CAR_STATE_ERROR)
    {
        PHAL_writeGPIO(SDC_CTRL_GPIO_Port, SDC_CTRL_Pin, false); // open SDC
    }
    else if (car.state == CAR_STATE_PREREADY2DRIVE)
    {
        // EV.10.5 - Ready to drive sound
        // 1-3 seconds, unique from other sounds
        if (!PHAL_readGPIO(BUZZER_GPIO_Port, BUZZER_GPIO_Pin))
        {
            PHAL_writeGPIO(BUZZER_GPIO_Port, BUZZER_GPIO_Pin, true);
            buzzer_start_tick = sched.os_ticks;
        }
        // stop buzzer
        else if ((sched.os_ticks - buzzer_start_tick) >
                 (SystemCoreClock * BUZZER_DURATION_MS / 1000)
        {
            PHAL_writeGPIO(BUZZER_GPIO_Port, BUZZER_GPIO_Pin, false);
            car.state = CAR_STATE_READY2DRIVE;
        }
    }
    else if (car.state == CAR_STATE_READY2DRIVE)
    {
        // TODO: check raw torque cmd timeout

    }


}





void startBuzzer(uint16_t time_ms)
{
    // TODO: save timer stop time, convert ms -> os ticks
}
