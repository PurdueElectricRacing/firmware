#include "car.h"

uint16_t mot_left_req;  // 0 - 4095 value
uint16_t mot_right_req; // 0 - 4095 value
volatile Car_t car;
extern q_handle_t q_tx_can;
uint32_t buzzer_start_tick = 0;
volatile ADCReadings_t adc_readings;
uint8_t prchg_set;

static bool checkErrorFaults();
static bool checkFatalFaults();
static void brakeLightUpdate(uint16_t raw_brake);

bool carInit()
{
    car.state = CAR_STATE_INIT;
    PHAL_writeGPIO(SDC_CTRL_GPIO_Port, SDC_CTRL_Pin, 0);
    PHAL_writeGPIO(BUZZER_GPIO_Port, BUZZER_Pin, 0);
    prchg_set = 0;
    mot_left_req = mot_right_req = 0;
}

void carHeartbeat()
{
    SEND_MAIN_HB(q_tx_can, car.state, prchg_set);
}

/**
 * @brief Main task for the car containing a finite state machine
 *        to determine when the car should be driveable
 */
uint32_t last_time = 0;
uint8_t curr = 0;
void carPeriodic()
{

    torqueRequest_t torque_r;

    /* State Independent Operations */

    /**
     * Brake Light Control
     * The on threshold is larger than the off threshold to
     * behave similar to a shmitt trigger, preventing blinking
     * during a transition
     */
    if (can_data.raw_throttle_brake.brake > BRAKE_LIGHT_ON_THRESHOLD)
    {
        if (!car.brake_light)
        {
            PHAL_writeGPIO(BRK_LIGHT_GPIO_Port, BRK_LIGHT_Pin, true);
            car.brake_light = true;
        }
    }
    else if (car.brake_light < BRAKE_LIGHT_OFF_THRESHOLD)
    {
        if (car.brake_light)
        {
            PHAL_writeGPIO(BRK_LIGHT_GPIO_Port, BRK_LIGHT_Pin, false);
            car.brake_light = false;
        }
    }

    if (checkErrorFaults())
    {
        car.state = CAR_STATE_ERROR;
        PHAL_writeGPIO(BUZZER_GPIO_Port, BUZZER_Pin, false); // in case during buzzing
    }
    // A fatal fault has higher prority
    // than an error fault
    if (checkFatalFaults())
    {
        car.state = CAR_STATE_FATAL;
        PHAL_writeGPIO(BUZZER_GPIO_Port, BUZZER_Pin, false); // in case during buzzing
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
        // TODO: stale checking on start button
        // TODO: move brake pressed check from dashboard to here
        // TODO: make the brake signal sent based on the transudcers
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

        // Send torque command to all 2 motors

        // ?? how about set the deadzone on can_data.raw_throttle_brake.throttle 

        // int16_t t_req = can_data.raw_throttle_brake.throttle - ((can_data.raw_throttle_brake.brake > 409) ? can_data.raw_throttle_brake.brake : 0);
        // t_req = t_req < 100 ? 0 : ((t_req - 100) / (4095 - 100) * 4095);
        uint16_t adjusted_throttle = (can_data.raw_throttle_brake.throttle < 100) ? 0 : (can_data.raw_throttle_brake.throttle - 100) * 4095 / (4095 - 100);
        
        //int16_t t_req = adjusted_throttle - ((can_data.raw_throttle_brake.brake > 409) ? can_data.raw_throttle_brake.brake : 0); 
        int16_t t_req = adjusted_throttle; // removing regen brake TODO: revert if regen

        // SEND_TORQUE_REQUEST_MAIN(q_tx_can, t_req, t_req, t_req, t_req);
        

        // t_temp = (t_temp > 469) ? 0 : t_temp + 1;

        // E-diff
        //eDiff(t_req, &torque_r);
        // TODO: fix steering for ediff
        torque_r.torque_left = t_req;
        torque_r.torque_right = t_req;

        // check torque request (FSAE rule)
        if(torque_r.torque_left > t_req)
        {
            torque_r.torque_left = t_req;
        }
        if(torque_r.torque_right > t_req)
        {
            torque_r.torque_right = t_req;
        }

        // No regen :(
        // if (torque_r.torque_left < 0) torque_r.torque_left = 0;
        // if (torque_r.torque_right < 0) torque_r.torque_right = 0;

        SEND_TORQUE_REQUEST_MAIN(q_tx_can, 0, 0, torque_r.torque_left, torque_r.torque_right);
        // bypased for daq testing TODO: remove
        // if (mot_left_req > 4095 || mot_right_req > 4095) mot_left_req = mot_right_req = 0;
        // SEND_TORQUE_REQUEST_MAIN(q_tx_can, 0, 0, (int16_t) mot_left_req, (int16_t) mot_right_req);

        /************ Around the World *************/
        // Meant for testing with vehicle off the ground
        // Periodically cycles each wheel in a circular pattern
        // if (curr) SEND_TORQUE_REQUEST_MAIN(q_tx_can, 0, 0, 0, 410);
        // else SEND_TORQUE_REQUEST_MAIN(q_tx_can, 0, 0, 0, 0);
        /*
        if (sched.os_ticks - last_time > 2000){ 
            //curr++;
            curr = !curr;
            last_time = sched.os_ticks;
        }
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
    uint8_t prchg_stat;
    static uint16_t prchg_time;

    /* Heart Beat Stale */ 
    // is_error += can_data.dashboard_hb.stale;
    // is_error += can_data.front_driveline_hb.stale;
    // TODO: is_error += can_data.rear_driveline_hb.stale;
    // TODO: is_error += can_data.precharge_hb.stale;

    prchg_stat = PHAL_readGPIO(PRCHG_STAT_GPIO_Port, PRCHG_STAT_Pin);

    if (!prchg_stat) {
        ++prchg_time;
    } else {
        prchg_set = 1;
        prchg_time = 0;
    }

    if (prchg_time > (500 / 15)) {
        --prchg_time;
        ++is_error;
        prchg_set = 0;
    }

    /* Precharge */
    // is_error += !PHAL_readGPIO(PRCHG_STAT_GPIO_Port, PRCHG_STAT_Pin);

    /* Dashboard */
    // TODO: is_error += can_data.raw_throttle_brake.stale;

    /* Driveline */
    // Front
    // is_error += can_data.front_driveline_hb.front_left_motor  != 
    //             FRONT_LEFT_MOTOR_CONNECTED;
    // is_error += can_data.front_driveline_hb.front_right_motor != 
    //             FRONT_RIGHT_MOTOR_CONNECTED;

    // Rear
    // TODO: revert
    /*
    is_error += can_data.rear_driveline_hb.rear_left_motor    != 
                REAR_LEFT_MOTOR_CONNECTED;
    is_error += can_data.rear_driveline_hb.rear_right_motor   != 
                REAR_RIGHT_MOTOR_CONNECTED;
                */

    /* Temperature */
    // TODO: if (!DT_ALWAYS_COOL)  is_error += cooling.dt_temp_error;
    // TODO: if (!BAT_ALWAYS_COOL) is_error += cooling.bat_temp_error;

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

    return is_error;
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

    // TODO: is_error += !PHAL_readGPIO(LIPO_BAT_STAT_GPIO_Port, LIPO_BAT_STAT_Pin);

    is_error += (can_data.max_cell_temp.max_temp > 500) ? 1 : 0;

    return is_error;
}

