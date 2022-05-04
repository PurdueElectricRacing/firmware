#include "car.h"

volatile Car_t car;
extern q_handle_t q_tx_can;
uint32_t buzzer_start_tick = 0;
volatile ADCReadings_t adc_readings;

static bool checkErrorFaults();
static bool checkFatalFaults();
static void brakeLightUpdate(uint16_t raw_brake);

bool carInit()
{
    car.state = CAR_STATE_INIT;
    PHAL_writeGPIO(SDC_CTRL_GPIO_Port, SDC_CTRL_Pin, 0);
    PHAL_writeGPIO(BUZZER_GPIO_Port, BUZZER_Pin, 0);
}

void carHeartbeat()
{
    SEND_MAIN_HB(q_tx_can, car.state, 
                 PHAL_readGPIO(PRCHG_STAT_GPIO_Port, PRCHG_STAT_Pin));
}

/**
 * @brief Main task cor the car containing a finite state machine
 *        to determine when the car should be driveable
 */
uint32_t last_time = 0;
uint8_t curr = 0;
void carPeriodic()
{
    static uint16_t t_temp;

    /* State Independent Operations */

    // if (GPIOC->IDR & (1U << 1)) {
    //     asm("nop");
    // } else {
    //     asm("nop");
    // }

    // brakeLightUpdate(can_data.raw_throttle_brake.brake);
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


    if (checkErrorFaults())
    {
        car.state = CAR_STATE_ERROR;
    }
    // A fatal fault has higher prority
    // than an error fault
    if (checkFatalFaults())
    {
        car.state = CAR_STATE_FATAL;
    }

    /* State Dependent Operations */

    // EV.10.4 - Activation sequence
    // Tractive System Active - SDC closed, HV outside accumulator
    // Ready to drive - motors respond to APPS input
    //                  not possible unless:
    //                   - tractive system active
    //                   - brake pedal pressed and held
    //                   - start button press

    if (car.state == CAR_STATE_FATAL)
    {
        // SDC critical error has occured, open sdc
        PHAL_writeGPIO(SDC_CTRL_GPIO_Port, SDC_CTRL_Pin, false); // open SDC
    }
    else if (car.state == CAR_STATE_ERROR)
    {
        // Error has occured, leave HV on but do not drive
        PHAL_writeGPIO(SDC_CTRL_GPIO_Port, SDC_CTRL_Pin, true); // close SDC
        // Recover once error gone
        if (!checkErrorFaults()) car.state = CAR_STATE_INIT;
    }
    else if (car.state == CAR_STATE_INIT)
    {
        PHAL_writeGPIO(SDC_CTRL_GPIO_Port, SDC_CTRL_Pin, true); // close SDC
        if (can_data.start_button.start)
        {
            can_data.start_button.start = 0; // debounce
            car.state = CAR_STATE_BUZZING;
        }
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
        // Check if requesting to exit ready2drive
        if (can_data.start_button.start)
        {
            can_data.start_button.start = 0; // debounce
            car.state = CAR_STATE_INIT;
        }

        // Send torque command to all 4 motors
        int16_t t_req = can_data.raw_throttle_brake.throttle - ((can_data.raw_throttle_brake.brake > 409) ? can_data.raw_throttle_brake.brake : 0);
        t_req = t_req > 820 ? 820 : t_req;
        // if (t_req > 0) {
        //     asm("bkpt");
        // }
        // SEND_TORQUE_REQUEST_MAIN(q_tx_can, t_req, t_req, t_req, t_req);
        if (sched.os_ticks - last_time > 2000){ 
            //curr++;
            curr = !curr;
            last_time = sched.os_ticks;
        }

        // t_temp = (t_temp > 469) ? 0 : t_temp + 1;

        SEND_TORQUE_REQUEST_MAIN(q_tx_can, 0, 0, t_req, 0);
        // if (curr) SEND_TORQUE_REQUEST_MAIN(q_tx_can, 0, 0, 0, 410);
        // else SEND_TORQUE_REQUEST_MAIN(q_tx_can, 0, 0, 0, 0);
        /*
        switch(curr)
        {
            case 0:
            SEND_TORQUE_REQUEST_MAIN(q_tx_can, t_req, 0, 0, 0);//t_req, t_req, t_req);
            break;
            case 1:
            SEND_TORQUE_REQUEST_MAIN(q_tx_can, 0, t_req, 0, 0);//t_req, t_req, t_req);
            break;
            case 2:
            SEND_TORQUE_REQUEST_MAIN(q_tx_can, 0, 0, t_req, 0);//t_req, t_req, t_req);
            break;
            case 3:
            SEND_TORQUE_REQUEST_MAIN(q_tx_can, 0, 0, 0, t_req);//t_req, t_req, t_req);
            break;
        }*/

    }
}

/**
 * @brief  Checks faults that should prevent
 *         the car from driving, but are okay
 *         to leave the sdc closed
 * 
 * @return true  Faults exist
 * @return false No faults have existed for set time
 */
uint32_t last_error_time = 0;
bool error_rose = 0;
bool checkErrorFaults()
{
    uint8_t is_error = 0;
    /* Heart Beat Stale */ 
    // is_error += can_data.dashboard_hb.stale;
    // is_error += can_data.front_driveline_hb.stale;
    // is_error += can_data.rear_driveline_hb.stale;
    //TODO: is_error += can_data.precharge_hb.stale;

    /* Precharge */
    is_error += !PHAL_readGPIO(PRCHG_STAT_GPIO_Port, PRCHG_STAT_Pin);

    /* Dashboard */
    // is_error += can_data.raw_throttle_brake.stale;

    /* Driveline */
    // Front
    // is_error += can_data.front_driveline_hb.front_left_motor  != 
    //             FRONT_LEFT_MOTOR_CONNECTED;
    // is_error += can_data.front_driveline_hb.front_right_motor != 
    //             FRONT_RIGHT_MOTOR_CONNECTED;
    // Rear
    // is_error += can_data.rear_driveline_hb.rear_left_motor    != 
    //             REAR_LEFT_MOTOR_CONNECTED;
    // is_error += can_data.rear_driveline_hb.rear_right_motor   != 
    //             REAR_RIGHT_MOTOR_CONNECTED;

    /* Temperature */
    if (!DT_ALWAYS_COOL)  is_error += cooling.dt_temp_error;
    if (!BAT_ALWAYS_COOL) is_error += cooling.bat_temp_error;

    if (is_error && !error_rose) 
    {
        error_rose = true;
        last_error_time = sched.os_ticks;
    }

    if (!is_error && error_rose &&
        sched.os_ticks - last_error_time > ERROR_FALL_MS)
    {
        error_rose = false;
    }

    return is_error || error_rose;
}

/**
 * @brief  Checks faults that should open the SDC
 * @return true  Faults exist
 * @return false No faults have existed for set time
 */
bool checkFatalFaults()
{
    uint8_t is_error = 0;

    if (!DT_FLOW_CHECK_OVERRIDE)  is_error += cooling.dt_flow_error;
    if (!BAT_FLOW_CHECK_OVERRIDE) is_error += cooling.bat_flow_error;

    return is_error;
}

/**
 * @brief Calculates the low voltage system current
 *        draw based on the output of a current
 *        sense amplifier
 */
void calcLVCurrent()
{
    uint32_t raw = adc_readings.lv_i_sense;
    car.lv_current_mA = (uint16_t) (raw * 1000 * 1000 * LV_ADC_V_IN_V / 
                        (LV_MAX_ADC_RAW * LV_GAIN * LV_R_SENSE_mOHM));
}

static void brakeLightUpdate(uint16_t raw_brake)
{
    if (!car.brake_light)
    {
        if (raw_brake >= BRAKE_LIGHT_ON_THRESHOLD)
        {
            car.brake_light = 1;
            car.brake_blink_ct = 0;
            car.brake_start_time = sched.os_ticks;
            PHAL_writeGPIO(BRK_LIGHT_GPIO_Port, BRK_LIGHT_Pin, true);
        }
    }
    else
    {
        if (raw_brake < BRAKE_LIGHT_OFF_THRESHOLD)
        {
            car.brake_light = 0;
            PHAL_writeGPIO(BRK_LIGHT_GPIO_Port, BRK_LIGHT_Pin, false);
        }
        else if (car.brake_blink_ct != BRAKE_BLINK_CT)
        {
            if (sched.os_ticks - car.brake_start_time > BRAKE_BLINK_PERIOD / 2)
            {
                PHAL_toggleGPIO(BRK_LIGHT_GPIO_Port, BRK_LIGHT_Pin);
                if (PHAL_readGPIO(BRK_LIGHT_GPIO_Port, BRK_LIGHT_Pin)) ++car.brake_blink_ct;
                car.brake_start_time = sched.os_ticks;
            }
        }
    }
}