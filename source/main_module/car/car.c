#include "car.h"
#include "common/modules/wheel_speeds/wheel_speeds.h"

Car_t car;
extern q_handle_t q_tx_can;
extern q_handle_t q_tx_usart_l, q_tx_usart_r;
extern usart_rx_buf_t huart_l_rx_buf, huart_r_rx_buf;
uint8_t daq_buzzer;

/* Wheel Speed Config */
WheelSpeed_t left_wheel =  {.tim=TIM2, .invert=true};
WheelSpeed_t right_wheel = {.tim=TIM5, .invert=true};
// TODO: test invert
WheelSpeeds_t wheel_speeds = {.l=&left_wheel, .r=&right_wheel};

bool validatePrecharge();

bool carInit()
{
    /* Set initial states */
    car = (Car_t) {0}; // Everything to zero
    car.state = CAR_STATE_IDLE;
    car.torque_src = CAR_TORQUE_RAW;
    car.regen_enabled = false;
    PHAL_writeGPIO(SDC_CTRL_GPIO_Port, SDC_CTRL_Pin, car.sdc_close);
    PHAL_writeGPIO(BRK_LIGHT_GPIO_Port, BRK_LIGHT_Pin, car.brake_light);
    PHAL_writeGPIO(BUZZER_GPIO_Port, BUZZER_Pin, car.buzzer);

    daq_buzzer = 0;

    /* Motor Controller Initialization */
    mcInit(&car.motor_l, MC_L_INVERT, &q_tx_usart_l, &huart_l_rx_buf, &car.pchg.pchg_complete);
    mcInit(&car.motor_r, MC_R_INVERT, &q_tx_usart_r, &huart_r_rx_buf, &car.pchg.pchg_complete);


    wheelSpeedsInit(&wheel_speeds);
}

void carHeartbeat()
{
    SEND_MAIN_HB(q_tx_can, car.state, car.pchg.pchg_complete);
    SEND_REAR_MC_STATUS(q_tx_can, car.motor_l.motor_state,
        car.motor_l.link_state, car.motor_l.last_link_error,
        car.motor_r.motor_state, car.motor_r.link_state,
        car.motor_r.last_link_error);
    static uint8_t n;
    if (++n == 5)
    {
        SEND_PRECHARGE_STATE(q_tx_can, car.pchg.v_mc_filt, car.pchg.v_bat_filt,
                                       car.pchg.pchg_complete, car.pchg.pchg_error);
        n = 0;
    }
}

/**
 * @brief Main task for the car containing a finite state machine
 *        to determine when the car should be driveable
 */
uint32_t last_time = 0;
uint8_t curr = 0;
void carPeriodic()
{
    /* Set Default Outputs */
    // Set Outputs that are purely combinational (do not depend on previous events)
    // Default values will typically reflect those in the IDLE state
    car.torque_r.torque_left  = 0.0f;
    car.torque_r.torque_right = 0.0f;
    car.buzzer    = false;
    car.sdc_close = true;

    /* Process Inputs */

    // Start button debounce (only want rising edge)
    car.start_btn_debounced = can_data.start_button.start;
    can_data.start_button.start = false;


    /* State Independent Operations */
    validatePrecharge();
    setFault(ID_PCHG_IMPLAUS_FAULT, car.pchg.pchg_error);
    // setFault(ID_RTD_EXIT_FAULT, !car.pchg.pchg_complete && car.state == CAR_STATE_READY2DRIVE);

    /**
     * Brake Light Control
     * The on threshold is larger than the off threshold to
     * behave similar to a Shmitt-trigger, preventing blinking
     * during a transition
     */
    if (can_data.filt_throttle_brake.brake > BRAKE_LIGHT_ON_THRESHOLD)
    {
        if (!car.brake_light)
        {
            car.brake_light = true;
        }
    }
    else if (can_data.filt_throttle_brake.brake < BRAKE_LIGHT_OFF_THRESHOLD)
    {
        if (car.brake_light)
        {
            car.brake_light = false;
        }
    }

    if (checkFault(ID_RTD_EXIT_FAULT))
    {
        car.state = CAR_STATE_IDLE;
    }
    // An error fault has higher priority
    // than the RTD Exit Fault
    if (errorLatched())
    {
        car.state = CAR_STATE_ERROR;
    }
    // A fatal fault has higher priority
    // than an error fault
    if (fatalLatched())
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
        // Currently latches into this state
        car.sdc_close = false;
    }
    else if (car.state == CAR_STATE_ERROR)
    {
        // Error has occured, leave HV on but do not drive
        // Recover once error gone
        if (!errorLatched()) car.state = CAR_STATE_IDLE;
    }
    else if (car.state == CAR_STATE_IDLE)
    {
        if (car.start_btn_debounced &&
           can_data.filt_throttle_brake.brake > BRAKE_PRESSED_THRESHOLD &&
           car.pchg.pchg_complete)
        {
            car.state = CAR_STATE_BUZZING;
            car.buzzer_start_ms = sched.os_ticks;
        }
    }
    else if (car.state == CAR_STATE_BUZZING)
    {
        // EV.10.5 - Ready to drive sound
        // 1-3 seconds, unique from other sounds
        if (sched.os_ticks - car.buzzer_start_ms > BUZZER_DURATION_MS)
        {
            car.state = CAR_STATE_READY2DRIVE;
        }
    }
    else if (car.state == CAR_STATE_READY2DRIVE)
    {
        // Check if requesting to exit ready2drive
        if (car.start_btn_debounced)
        {
            car.state = CAR_STATE_IDLE;
        }
        else
        {
            float t_req_pedal = 0;
            if (!can_data.filt_throttle_brake.stale)
                t_req_pedal = (float) CLAMP(can_data.filt_throttle_brake.throttle, 0, 4095);

            t_req_pedal = t_req_pedal * 100.0f / 4095.0f;
            // TODO: ensure APPS checks sets throttle to 0 if enough braking
            // t_req = t_req < 100 ? 0 : ((t_req - 100) / (4095 - 100) * 4095);
            // uint16_t adjusted_throttle = (can_data.raw_throttle_brake.throttle < 100) ? 0 : (can_data.raw_throttle_brake.throttle - 100) * 4095 / (4095 - 100);

            torqueRequest_t temp_t_req;
            switch (car.torque_src)
            {
                case CAR_TORQUE_RAW:
                    temp_t_req.torque_left  = t_req_pedal;
                    temp_t_req.torque_right = t_req_pedal;
                    break;
                case CAR_TORQUE_TV:
                    // TODO: TV torque source
                    break;
                case CAR_TORQUE_DAQ:
                    break;
                case CAR_TORQUE_NONE:
                    break;
                default:
                    temp_t_req.torque_left  = 0;
                    temp_t_req.torque_right = 0;
                break;
            }

            // Enforce range
            temp_t_req.torque_left =  CLAMP(temp_t_req.torque_left,  -100.0f, 100.0f);
            temp_t_req.torque_right = CLAMP(temp_t_req.torque_right, -100.0f, 100.0f);

            // EV.4.2.3 - Torque algorithm
            // Any algorithm or electronic control unit that can adjust the
            // requested wheel torque may only lower the total driver
            // requested torque and must not increase it
            if (temp_t_req.torque_left > t_req_pedal)
            {
                temp_t_req.torque_left = t_req_pedal;
            }
            if (temp_t_req.torque_right > t_req_pedal)
            {
                temp_t_req.torque_right = t_req_pedal;
            }

            // Disable Regenerative Braking
            if (!car.regen_enabled)
            {
                if (temp_t_req.torque_left  < 0) temp_t_req.torque_left  = 0.0f;
                if (temp_t_req.torque_right < 0) temp_t_req.torque_right = 0.0f;
            }

            car.torque_r = temp_t_req;
        }
    }
    else if (car.state == CAR_STATE_FAN_CTRL)
    {
        // TODO: control fan speed based on throttle and steering angle
    }

    /* Update System Outputs */
    car.buzzer = car.state == CAR_STATE_BUZZING || daq_buzzer;
    PHAL_writeGPIO(SDC_CTRL_GPIO_Port, SDC_CTRL_Pin, car.sdc_close);
    PHAL_writeGPIO(BRK_LIGHT_GPIO_Port, BRK_LIGHT_Pin, car.brake_light);
    PHAL_writeGPIO(BUZZER_GPIO_Port, BUZZER_Pin, car.buzzer);
    mcSetPower(car.torque_r.torque_left,  &car.motor_l);
    mcSetPower(car.torque_r.torque_right, &car.motor_r);
 }


/**
 * @brief Parses motor controller and sensor
 *        info into can messages, updates
 *        motor controller connection status
 */
void parseMCDataPeriodic(void)
{
    uint16_t shock_l, shock_r;

    /* Update Motor Controller Data Structures */
    mcPeriodic(&car.motor_l);
    mcPeriodic(&car.motor_r);

    // setFault(ID_LEFT_MC_CONN_FAULT, car.pchg.pchg_complete &&
    //             car.motor_l.motor_state != MC_CONNECTED);
    // setFault(ID_RIGHT_MC_CONN_FAULT, car.pchg.pchg_complete &&
    //             car.motor_r.motor_state != MC_CONNECTED);
    // Only send once both controllers have updated data
    // if (motor_right.data_stale ||
    //     motor_left.data_stale) return;

    // TODO: fill with faults

    // Extract raw shocks from DMA buffer
    // shock_l = raw_shock_pots.pot_left;
    // shock_r = raw_shock_pots.pot_right;
    // Scale from raw 12bit adc to mm * 10 of linear pot travel
    // shock_l = (POT_VOLT_MIN_DIST_MM * 10 - ((uint32_t) shock_l) * (POT_VOLT_MIN_DIST_MM - POT_VOLT_MAX_DIST_MM) * 10 / 4095);
    // shock_r = (POT_VOLT_MIN_DIST_MM * 10 - ((uint32_t) shock_r) * (POT_VOLT_MIN_DIST_MM - POT_VOLT_MAX_DIST_MM) * 10 / 4095);

    // SEND_REAR_WHEEL_DATA(q_tx_can, wheel_speeds.left_kph_x100, wheel_speeds.right_kph_x100,
    //                      shock_l, shock_r);
    uint16_t l_speed = (wheel_speeds.l->rad_s / (2*PI));
    uint16_t r_speed = (wheel_speeds.l->rad_s / (2*PI));
    SEND_REAR_WHEEL_SPEEDS(q_tx_can, car.motor_l.rpm, car.motor_r.rpm,
                                     l_speed, r_speed);
    SEND_REAR_MOTOR_CURRENTS_TEMPS(q_tx_can,
                                   (uint16_t) car.motor_l.current_x10,
                                   (uint16_t) car.motor_r.current_x10,
                                   (uint8_t)  car.motor_l.motor_temp,
                                   (uint8_t)  car.motor_r.motor_temp,
                                   (uint16_t) car.motor_r.voltage_x10);
    // TODO: possibly move into cooling
    SEND_REAR_CONTROLLER_TEMPS(q_tx_can,
                               (uint8_t) car.motor_l.controller_temp,
                               (uint8_t) car.motor_r.controller_temp);
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

    // if (!prchg_stat) {
    //     ++prchg_time;
    // } else {
    //     prchg_set = 1;
    //     prchg_time = 0;
    // }

    // if (prchg_time > (500 / 15)) {
    //     --prchg_time;
    //     ++is_error;
    //     prchg_set = 0;
    // }

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

/**
 * @brief Resets the steering angle sensor calibration
 *        Call once after assembly with wheel centered
 *        Device: Bosch F02U.V02.894-01
 */
void calibrateSteeringAngle(uint8_t *success)
{
    // To zero the sensor after assembly:
    // Reset calibration with CCW = 5h
    // Start a new calibration with CCW = 3h
    // The sensor can then be used immediately
    SEND_LWS_CONFIG(q_tx_can, 0x05, 0, 0); // reset cal
    SEND_LWS_CONFIG(q_tx_can, 0x03, 0, 0); // start new
    *success = 1;
}

/**
 * @brief Checks if the precharge circuit is working
 *        correctly based on the V_MV and V_Bat ADC
 *        measurements and PRCHG Complete Signal
 *
 * @return true Precharge Working Properly
 */
bool validatePrecharge()
{
    uint32_t tmp_mc, tmp_bat;
    tmp_mc = tmp_bat = 0;

    // Measure inputs
    car.pchg.v_mc_buff[car.pchg.v_mc_buff_idx++]   = adc_readings.v_mc;
    car.pchg.v_bat_buff[car.pchg.v_bat_buff_idx++] = adc_readings.v_bat;
    car.pchg.pchg_complete = PHAL_readGPIO(PRCHG_STAT_GPIO_Port, PRCHG_STAT_Pin);

    // Update buffers
    car.pchg.v_mc_buff_idx  %= HV_LOW_PASS_SIZE;
    car.pchg.v_bat_buff_idx %= HV_LOW_PASS_SIZE;

    // Sum buffers
    for (uint8_t i = 0; i < HV_LOW_PASS_SIZE; ++i)
    {
        tmp_mc  += car.pchg.v_mc_buff[i];
        tmp_bat += car.pchg.v_bat_buff[i];
    }
    // Take average
    tmp_mc  /= HV_LOW_PASS_SIZE;
    tmp_bat /= HV_LOW_PASS_SIZE;

    // V_outx10 = adc_raw * adc_v_ref / adc_max (Tiffomy outputs volts / 100)
    car.pchg.v_mc_filt  = (tmp_mc  * ((ADC_REF_mV * HV_V_MC_CAL)  / 1000UL)) / 0xFFFUL;
    car.pchg.v_bat_filt = (tmp_bat * ((ADC_REF_mV * HV_V_BAT_CAL) / 1000UL)) / 0xFFFUL;

    // Validate
    if ((car.pchg.pchg_complete && car.pchg.v_mc_filt < ((((uint32_t) car.pchg.v_bat_filt) * 80) / 100)) ||
        (!car.pchg.pchg_complete && car.pchg.v_mc_filt >= ((((uint32_t) car.pchg.v_bat_filt * 95) / 100))))
    {
        car.pchg.pchg_error = true;
    }
    else
    {
        car.pchg.pchg_error = false;
    }

    return car.pchg.pchg_error;
}