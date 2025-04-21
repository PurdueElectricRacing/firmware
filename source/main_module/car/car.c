#include "car.h"
#include "main.h"
#include "wheel_speeds.h"

#include <string.h>

Car_t car;
extern uint16_t num_failed_msgs_l, num_failed_msgs_r;
extern WheelSpeeds_t wheel_speeds;
uint8_t daq_buzzer;
uint8_t daq_brake;
uint8_t daq_constant_tq;
uint8_t const_tq_val;
uint8_t buzzer_brake_fault;
sdc_nodes_t sdc_mux;
bool enableAMKs;

// Historical record of Brake stat and Current Sense to tell if BSPD has failed
int16_t hist_current[NUM_HIST_BSPD] = {0};
uint8_t hist_curr_idx;
uint8_t prchg_start;

// Circular buffer for precharge complete
bool pchg_complete_lowpass_buf[PCHG_COMPLETE_LOW_PASS_SIZE];
uint8_t pchg_complete_lowpass_idx;

// TV control vars
bool any_tv_msg_stale;
VCU_mode_t requested_tv_mode;

bool validatePrecharge();

bool carInit()
{
    /* Set initial states */
    car = (Car_t) {0}; // Everything to zero
    car.state = CAR_STATE_IDLE;
    car.torque_src = CAR_TORQUE_TV;
    car.regen_enabled = false;
    car.sdc_close = true; // We want to initialize SDC as "good"
    pchg_complete_lowpass_idx = 0;
    memset(pchg_complete_lowpass_buf, 0, sizeof(pchg_complete_lowpass_buf));
    PHAL_writeGPIO(SDC_CTRL_GPIO_Port, SDC_CTRL_Pin, car.sdc_close);
    PHAL_writeGPIO(BRK_LIGHT_GPIO_Port, BRK_LIGHT_Pin, car.brake_light);
    PHAL_writeGPIO(BUZZER_GPIO_Port, BUZZER_Pin, car.buzzer);

    daq_buzzer = 0;
    daq_constant_tq = 0;
    const_tq_val = 0;
    hist_curr_idx = 0;
    enableAMKs = false;
    amkInit(&car.motor_l, &enableAMKs, INVA_ID);
    amkInit(&car.motor_r, &enableAMKs, INVB_ID);

    PHAL_writeGPIO(SDC_MUX_S0_GPIO_Port, SDC_MUX_S0_Pin, 0);
    PHAL_writeGPIO(SDC_MUX_S1_GPIO_Port, SDC_MUX_S1_Pin, 0);
    PHAL_writeGPIO(SDC_MUX_S2_GPIO_Port, SDC_MUX_S2_Pin, 0);
    PHAL_writeGPIO(SDC_MUX_S3_GPIO_Port, SDC_MUX_S3_Pin, 0);
    wheelSpeedsInit();
}

void carHeartbeat()
{
    SEND_MAIN_HB(car.state, car.pchg.pchg_complete);
    SEND_MAIN_HB_AMK(car.state, car.pchg.pchg_complete);
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

    // TSMS/HVD Disconnecting is not an error, so go back to init state. However, we must keep fatal state latched
    if (checkFault(ID_TSMS_DISC_FAULT) || checkFault(ID_HVD_DISC_FAULT) && car.state != CAR_STATE_FATAL)
        car.state = CAR_STATE_IDLE;

    /* Process Inputs */

    // Start button debounce (only want rising edge)
    car.start_btn_debounced = can_data.start_button.start;
    can_data.start_button.start = false;

    /* Precharge Complete Lowpass */
    pchg_complete_lowpass_buf[pchg_complete_lowpass_idx] = PHAL_readGPIO(PRCHG_STAT_GPIO_Port, PRCHG_STAT_Pin);
    // Increment buffer idx
    pchg_complete_lowpass_idx++;
    pchg_complete_lowpass_idx %= PCHG_COMPLETE_LOW_PASS_SIZE;

    // Update precharge complete status
    uint8_t pchg_complete_num_high = 0;
    for (uint8_t i = 0; i < PCHG_COMPLETE_LOW_PASS_SIZE; i++)
    {
      pchg_complete_num_high += (uint8_t)pchg_complete_lowpass_buf[i];
    }

    float pchg_complete_stat = (float) pchg_complete_num_high / (float) PCHG_COMPLETE_LOW_PASS_SIZE;

    // precharge on average is complete
    car.pchg.pchg_complete = (pchg_complete_stat > 0.5f);

    /* AMK Control */
    enableAMKs = (car.state == CAR_STATE_READY2DRIVE || car.state == CAR_STATE_CONSTANT_TORQUE);

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
        prchg_start = false;
    }
    else if (car.state == CAR_STATE_IDLE)
    {
        prchg_start = false;
        if (sdc_mux.tsms_stat)
        {
            car.state = CAR_STATE_PRECHARGING;
        }
    }
    else if (car.state == CAR_STATE_PRECHARGING)
    {
        if (car.pchg.pchg_complete)
        {
            car.state = CAR_STATE_ENERGIZED;
        }
    }
    else if (car.state == CAR_STATE_ENERGIZED)
    {
        prchg_start = false;

        if (car.start_btn_debounced &&
           can_data.filt_throttle_brake.brake > BRAKE_PRESSED_THRESHOLD)
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
            if (daq_constant_tq)
                car.state = CAR_STATE_CONSTANT_TORQUE;
            else
                car.state = CAR_STATE_READY2DRIVE;
        }
    }
    else if (car.state == CAR_STATE_READY2DRIVE)
    {
        if (!car.pchg.pchg_complete)
        {
            car.state = CAR_STATE_IDLE;
        }

        // Check if requesting to exit ready2drive
        if (car.start_btn_debounced)
        {
            car.state = CAR_STATE_IDLE;
        }
        else
        {
            float t_req_pedal = 0;
            float t_req_tv_l = 0;
            float t_req_tv_r = 0;
            float t_req_equal = 0;

            float s_req_tv_l = 0;
            float s_req_tv_r = 0;
            float s_req_equal = 0;

            any_tv_msg_stale = false;

            requested_tv_mode = VCU_MODE_INVALID;

            if (!can_data.filt_throttle_brake.stale)
                t_req_pedal = (float) CLAMP(can_data.filt_throttle_brake.throttle, 0, 4095);

            if (!can_data.drive_modes.stale && !can_data.VCU_torques_speeds.stale)
            {
              requested_tv_mode = can_data.drive_modes.VCU_mode;
              t_req_tv_l = CLAMP(can_data.VCU_torques_speeds.TO_VT_left, 0, MAX_TV_TORQUE_REQUEST);
              t_req_tv_r = CLAMP(can_data.VCU_torques_speeds.TO_VT_right, 0, MAX_TV_TORQUE_REQUEST);
              t_req_equal = CLAMP(can_data.VCU_torques_speeds.TO_PT_equal, 0, MAX_TV_TORQUE_REQUEST);
            }
            else
            {
              requested_tv_mode = VCU_MODE_INVALID;
              any_tv_msg_stale = true;
            }

            if (car.torque_src == CAR_TORQUE_TV)
            {
              setFault(ID_TV_STALE_FAULT, any_tv_msg_stale);
            }

            // Final check to see if we can run TV
            // Debounce TV stale (if TV goes in/out of being stale)
            if (checkFault(ID_TV_STALE_FAULT))
            {
              requested_tv_mode = VCU_MODE_INVALID;
            }

            // Torque Requests
            t_req_pedal = (int16_t)(t_req_pedal * MAX_DRIVER_TORQUE_REQUEST / 4095.0f);
            // Torque is provided from 0-2100 (i.e. Nm * 100). We command Percent torque (0-210)
            t_req_tv_l = (int16_t)(t_req_tv_l / TV_TORQUE_REQUEST_SCALE);
            t_req_tv_r = (int16_t)(t_req_tv_r / TV_TORQUE_REQUEST_SCALE);
            t_req_equal = (int16_t)(t_req_equal / TV_TORQUE_REQUEST_SCALE);

            torqueRequest_t temp_t_req;
            switch (car.torque_src)
            {
                case CAR_TORQUE_RAW:
                    temp_t_req.torque_left  = t_req_pedal;
                    temp_t_req.torque_right = t_req_pedal;
                    break;
                case CAR_TORQUE_TV:
                    switch(requested_tv_mode)
                    {
                      case VCU_MODE_EQUAL_SPEED:
                      case VCU_MODE_VARIABLE_SPEED:
                        // As of now we should never enter these cases. Never command a speed request with the current settings.
                        s_req_equal = 0;
                        temp_t_req.torque_left = s_req_equal;
                        temp_t_req.torque_right = s_req_equal;
                        break;
                      case VCU_MODE_EQUAL_TORQUE:
                        temp_t_req.torque_left = t_req_pedal;
                        temp_t_req.torque_right = t_req_pedal;
                        break;
                      case VCU_MODE_EQUAL_TORQUE_WITH_SAFETY:
                        temp_t_req.torque_left = t_req_equal;
                        temp_t_req.torque_right = t_req_equal;
                        break;
                      case VCU_MODE_VARIABLE_TORQUE:
                        temp_t_req.torque_left = t_req_tv_l;
                        temp_t_req.torque_right = t_req_tv_r;
                        break;
                      case VCU_MODE_INVALID:
                        temp_t_req.torque_left = t_req_pedal;
                        temp_t_req.torque_right = t_req_pedal;
                    }
                    // EV.4.2.3 - Torque algorithm
                    // Any algorithm or electronic control unit that can adjust the
                    // requested wheel torque may only lower the total driver
                    // requested torque and must not increase it
                    temp_t_req.torque_left = CLAMP(temp_t_req.torque_left, 0, t_req_pedal);
                    temp_t_req.torque_right = CLAMP(temp_t_req.torque_right, 0, t_req_pedal);
                    break;
                case CAR_TORQUE_THROT_MAP:
                    temp_t_req.torque_left  = t_req_equal;
                    temp_t_req.torque_right = t_req_equal;
                    // EV.4.2.3 - Torque algorithm
                    // Any algorithm or electronic control unit that can adjust the
                    // requested wheel torque may only lower the total driver
                    // requested torque and must not increase it
                    if (t_req_pedal == 0)
                    {
                        temp_t_req.torque_left = 0;
                        temp_t_req.torque_right = 0;
                    }
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
            temp_t_req.torque_left =  CLAMP(temp_t_req.torque_left,  MIN_DRIVER_TORQUE_REQUEST, MAX_DRIVER_TORQUE_REQUEST);
            temp_t_req.torque_right = CLAMP(temp_t_req.torque_right, MIN_DRIVER_TORQUE_REQUEST, MAX_DRIVER_TORQUE_REQUEST);

            // Disable Regenerative Braking
            if (!car.regen_enabled)
            {
                if (temp_t_req.torque_left  < 0) temp_t_req.torque_left  = 0.0f;
                if (temp_t_req.torque_right < 0) temp_t_req.torque_right = 0.0f;
            }

            car.torque_r = temp_t_req;
        }
    }
    else if (car.state == CAR_STATE_CONSTANT_TORQUE)
    {
        static bool throttle_pressed;
        static bool brake_pressed;
        // Check if requesting to exit ready2drive
        // if (car.start_btn_debounced)
        // {
        //     car.state = CAR_STATE_IDLE;
        // }
        // else
        // {
            float t_req_pedal = 0;

            if (!can_data.filt_throttle_brake.stale)
                t_req_pedal = (float) CLAMP(can_data.filt_throttle_brake.throttle, 0, 4095);

            torqueRequest_t temp_t_req;
            // Register an INTENTIONAL press of throttle and hold this state untill commanded otherwise
            if (throttle_pressed || (!throttle_pressed && t_req_pedal > 1024))
            {
                brake_pressed = 0;
                temp_t_req.torque_left = (const_tq_val * 10.0f);
                temp_t_req.torque_right = (const_tq_val * 10.0f);
                throttle_pressed = 1;
            }
            if ((can_data.filt_throttle_brake.brake > 50) || brake_pressed)
            {
                throttle_pressed = 0;
                brake_pressed = 1;
                temp_t_req.torque_left = 0.0f;
                temp_t_req.torque_right = 0.0f;
            }

            // Enforce range
            temp_t_req.torque_left =  CLAMP(temp_t_req.torque_left,  MIN_DRIVER_TORQUE_REQUEST, MAX_DRIVER_TORQUE_REQUEST);
            temp_t_req.torque_right = CLAMP(temp_t_req.torque_right, MIN_DRIVER_TORQUE_REQUEST, MAX_DRIVER_TORQUE_REQUEST);

            // Disable Regenerative Braking
            if (!car.regen_enabled)
            {
                if (temp_t_req.torque_left  < 0) temp_t_req.torque_left  = 0.0f;
                if (temp_t_req.torque_right < 0) temp_t_req.torque_right = 0.0f;
            }

            car.torque_r = temp_t_req;
        // }
    }

    /* Update System Outputs */
    car.buzzer = car.state == CAR_STATE_BUZZING || daq_buzzer;
    buzzer_brake_fault = PHAL_readGPIO(BRK_BUZZER_STAT_GPIO_Port, BRK_BUZZER_STAT_Pin);
    PHAL_writeGPIO(SDC_CTRL_GPIO_Port, SDC_CTRL_Pin, car.sdc_close);
    PHAL_writeGPIO(BRK_LIGHT_GPIO_Port, BRK_LIGHT_Pin, car.brake_light | daq_brake);
    PHAL_writeGPIO(BUZZER_GPIO_Port, BUZZER_Pin, car.buzzer);

    /* At this point torque request will be clamped from 0 to 1000 */
    amkSetTorque(&car.motor_l, car.torque_r.torque_left);
    amkSetTorque(&car.motor_r, car.torque_r.torque_right);
 }

/**
 * @brief Parses motor controller and sensor
 *        info into can messages, updates
 *        motor controller connection status
 */
void parseMCDataPeriodic(void)
{
    amkPeriodic(&car.motor_l);
    amkPeriodic(&car.motor_r);
    wheelSpeedsPeriodic();
    SEND_REAR_WHEEL_SPEEDS(wheel_speeds.left_rad_s_x100, wheel_speeds.right_rad_s_x100);
}

void send_shockpots()
{
    uint16_t shock_l = adc_readings.shock_l;
    uint16_t shock_r = adc_readings.shock_r;
    int16_t shock_l_parsed;
    int16_t shock_r_parsed;
    // Will scale linearly from 0 - 3744. so 75 - (percent of 3744 * 75)
    shock_l_parsed = -1 * ((POT_MAX_DIST - (int16_t)((shock_l / (POT_VOLT_MIN_L - POT_VOLT_MAX_L)) * POT_MAX_DIST)) - POT_DIST_DROOP_L);
    shock_r_parsed = -1 * ((POT_MAX_DIST - (int16_t)((shock_r / (POT_VOLT_MIN_R - POT_VOLT_MAX_R)) * POT_MAX_DIST)) - POT_DIST_DROOP_R);
    SEND_SHOCK_REAR(shock_l_parsed, shock_r_parsed);
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

void updateSDCFaults()
{
    if(!can_data.orion_currents_volts.stale)
    {
        hist_current[hist_curr_idx++] = can_data.orion_currents_volts.pack_current;
        hist_curr_idx %= NUM_HIST_BSPD;
    }
    uint8_t brake_fail = 0;
    // Loop through all SDC nodes (no need for precharge complete)
    for (uint8_t i = 0; i <= SDC_TSMS; i++)
    {
        switch(i)
        {
            case (SDC_C_STOP):
                if (!sdc_mux.c_stop_stat && sdc_mux.inertia_stat)
                {
                    setFault(ID_COCKPIT_ESTOP_FAULT, 1);
                }
                else
                {
                    setFault(ID_COCKPIT_ESTOP_FAULT, 0);
                }
                break;
            case (SDC_INERTIA):
                if (!sdc_mux.inertia_stat && sdc_mux.bots_stat)
                {
                    setFault(ID_INERTIA_FAIL_FAULT, 1);
                }
                else
                {
                    setFault(ID_INERTIA_FAIL_FAULT, 0);
                }
                break;
            case (SDC_BOTS):
                //If bots is down, we need to check whether BOTS was tripped or BSPD was tripped
                // if (!sdc_mux.bots_stat && !checkFault(ID_IMD_FAULT) && (PHAL_readGPIO(BMS_STAT_GPIO_Port, BMS_STAT_Pin) || can_data.orion_currents_volts.stale))
                // {
                //     int32_t total_current = 0;
                //     for (int16_t i = 0; i < NUM_HIST_BSPD; i++)
                //     {
                //         total_current += hist_current[i];
                //     }
                //     if (total_current > 5000)
                //     {
                //         setFault(ID_BSPD_LATCHED_FAULT, 1);
                //     }
                //     else if (brake_fail)
                //         setFault(ID_BSPD_LATCHED_FAULT, 1);
                //     else
                //         setFault(ID_BOTS_FAIL_FAULT, 1);
                // }
                // else if (checkFault(ID_BOTS_FAIL_FAULT) && sdc_mux.bots_stat)
                // {
                //     setFault(ID_BOTS_FAIL_FAULT, 0);
                // }
                break;
            case (SDC_L_STOP):
                if (!sdc_mux.l_stop_stat && sdc_mux.r_stop_stat)
                {
                    setFault(ID_LEFT_ESTOP_FAULT, 1);
                }
                else
                {
                    setFault(ID_LEFT_ESTOP_FAULT, 0);
                }
                break;
            case (SDC_R_STOP):
                if (!sdc_mux.r_stop_stat && sdc_mux.main_stat)
                {
                    setFault(ID_RIGHT_ESTOP_FAULT, 1);
                }
                else
                {
                    setFault(ID_RIGHT_ESTOP_FAULT, 0);
                }
                break;
            case (SDC_HVD):
                if (!sdc_mux.hvd_stat && sdc_mux.l_stop_stat)
                {
                    setFault(ID_HVD_DISC_FAULT, 1);
                }
                else
                {
                    setFault(ID_HVD_DISC_FAULT, 0);
                }
                break;
            case (SDC_HUB):
                if (!sdc_mux.r_hub_stat && sdc_mux.hvd_stat)
                {
                    setFault(ID_HUB_DISC_FAULT, 1);
                }
                else
                {
                    setFault(ID_HUB_DISC_FAULT, 0);
                }
                break;
            case (SDC_TSMS):
                if (!sdc_mux.tsms_stat && sdc_mux.r_hub_stat)
                {
                    setFault(ID_TSMS_DISC_FAULT, 1);
                }
                else
                {
                    setFault(ID_TSMS_DISC_FAULT, 0);
                }
                break;
        }
    }
}

/**
 * @brief Update Status of SDC Mux and Send on CAN
 *
 */
void monitorSDCPeriodic()
{
    static uint8_t index = 0;
    static sdc_nodes_t sdc_nodes_raw;
    bool *nodes = (bool *) &sdc_nodes_raw;

    uint8_t stat =  (uint8_t) PHAL_readGPIO(SDC_MUX_DATA_GPIO_Port, SDC_MUX_DATA_Pin);
    *(nodes + index) = stat;

    index++;
    if (index == SDC_MUX_HIGH_IDX)
    {
        index = 0;
        sdc_mux = sdc_nodes_raw;
        SEND_SDC_STATUS(sdc_mux.main_stat, sdc_mux.c_stop_stat, sdc_mux.inertia_stat, sdc_mux.bots_stat,
                sdc_mux.bspd_stat, sdc_mux.bms_stat, sdc_mux.imd_stat, sdc_mux.r_stop_stat, sdc_mux.l_stop_stat,
                sdc_mux.hvd_stat, sdc_mux.emeter_stat, sdc_mux.r_hub_stat, sdc_mux.tsms_stat, sdc_mux.pchg_out_stat);
    }

    PHAL_writeGPIO(SDC_MUX_S0_GPIO_Port, SDC_MUX_S0_Pin, (index & 0x01));
    PHAL_writeGPIO(SDC_MUX_S1_GPIO_Port, SDC_MUX_S1_Pin, (index & 0x02));
    PHAL_writeGPIO(SDC_MUX_S2_GPIO_Port, SDC_MUX_S2_Pin, (index & 0x04));
    PHAL_writeGPIO(SDC_MUX_S3_GPIO_Port, SDC_MUX_S3_Pin, (index & 0x08));
}

void update_lights(void)
{
    // IMD or BMS faulted
    bool unsafe_condition_detected = false;

    // Software checks for IMD or BMS fault
    unsafe_condition_detected |= checkFault(ID_HV_ISOLATION_FAULT);
    unsafe_condition_detected |= can_data.orion_info.dtc_status;

    // Hardware checks for IMD or BMS fault
    unsafe_condition_detected |= !sdc_mux.imd_stat || !sdc_mux.bms_stat;

    PHAL_writeGPIO(SAFE_STAT_G_GPIO_Port, SAFE_STAT_G_GPIO_Pin, !unsafe_condition_detected);
    PHAL_writeGPIO(SAFE_STAT_R_GPIO_Port, SAFE_STAT_R_GPIO_Pin, unsafe_condition_detected);
}
