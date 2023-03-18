#include "cooling.h"

Cooling_t cooling;
volatile uint16_t raw_dt_flow_ct;
volatile uint16_t raw_bat_flow_ct;
uint32_t last_flow_meas_time_ms;
uint32_t dt_pump_start_time_ms;
uint32_t bat_pump_start_time_ms;

extern q_handle_t q_tx_can;


// static void setDtCooling(uint8_t on);
// static void setBatCooling(uint8_t on);
static void setDtPump(bool on);
static void setBatPump(bool on, bool on_aux);
static void setBatFan(uint8_t power);
static void setDtFan(uint8_t power);
uint8_t lowpass(uint8_t new, uint8_t *old, uint8_t curr);

bool coolingInit()
{
    cooling = (Cooling_t) {0};
    /* Configure GPIO Interrupts */
    // // enable syscfg clock
    // RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    // // set exti gpio source through syscfg (0b0010 means GPIOC)
    // SYSCFG->EXTICR[1] |= (((uint16_t)0b0010) << (DT_FLOW_RATE_PWM_Pin  - 4) * 4) |
    //                      (((uint16_t)0b0010) << (BAT_FLOW_RATE_PWM_Pin - 4) * 4);
    // // unmask the interrupt
    // EXTI->IMR1 |= /*(0b1 << DT_FLOW_RATE_PWM_Pin) | */
    //               (0b1 << BAT_FLOW_RATE_PWM_Pin);
    // // set trigger to rising edge
    // EXTI->RTSR1 |= (0b1 << DT_FLOW_RATE_PWM_Pin) |
    //                (0b1 << BAT_FLOW_RATE_PWM_Pin);
    // // enable the interrupt handlers
    // NVIC_EnableIRQ(EXTI9_5_IRQn);

    // Default pin configurations
    // setDtCooling(0);
    // setBatCooling(0);
    setDtPump(0);
    setBatPump(0, 0);
    setDtFan(0);
    setBatFan(0);

    return true;
}

uint8_t lowpass(uint8_t new, uint8_t *old, uint8_t curr) {
    uint8_t i;
    float   average = 0;

    old[curr] = new;

    for (i = 0; i < AVG_WINDOW_SIZE; i++) {
        average += (float) old[i];
    }

    return (uint8_t) (average / AVG_WINDOW_SIZE);
}

void coolingPeriodic()
{
    /* WATER TEMP CALCULATIONS */
    static uint8_t curr_therm;
    float temp;

    // TODO: test signed temps

    // Since ADC readings happen ~2ms, the next measurement should be ready
    temp = rawThermtoCelcius(adc_readings.therm_mux_d);
    switch(curr_therm)
    {
        case THERM_MUX_BAT_IN:
            cooling.bat_therm_in_C  = temp;
            break;
        case THERM_MUX_BAT_OUT:
            cooling.bat_therm_out_C = temp;
            break;
        case THERM_MUX_DT_IN:
            cooling.dt_therm_in_C = temp;
            break;
        case THERM_MUX_DT_OUT:
            cooling.dt_therm_out_C = temp;
            break;
    }
    curr_therm = (curr_therm + 1) & 0x03;
    PHAL_writeGPIO(THERM_MUX_S0_GPIO_Port, THERM_MUX_S0_Pin, curr_therm & 0x01);
    PHAL_writeGPIO(THERM_MUX_S1_GPIO_Port, THERM_MUX_S1_Pin, curr_therm & 0x02);

    // Update outputs
    if (cooling.daq_override)
    {
        cooling.out = cooling.out_daq_req;
    }
    else
    {
        // TODO: dashbaord controlled
        cooling.out = (Cooling_output_t) {0};
    }

    setDtFan(cooling.out.dt_fan_power);
    setBatFan(cooling.out.bat_fan_power);
    setBatPump(cooling.out.bat_pump, cooling.out.bat_pump_aux);
    setDtPump(cooling.out.dt_pump);


    SEND_FLOWRATE_TEMPS(q_tx_can, cooling.bat_therm_in_C, cooling.bat_therm_out_C,
                                  cooling.dt_therm_in_C,  cooling.dt_therm_out_C,
                                  cooling.bat_liters_p_min_x10, cooling.dt_liters_p_min_x10,
                                  0, 0);

    return;
    /*
     FLOW CALCULATIONS
    // Convert ticks and time delta to liters per minute
    if (cooling.dt_delta_t == 0)
        cooling.dt_liters_p_min_x10 = 0;
    else
        cooling.dt_liters_p_min_x10 = ((1000 / (float) (cooling.dt_delta_t * 7.5))) * 10;
    if (cooling.bat_delta_t == 0)
        cooling.bat_delta_t = 0;
    else
        cooling.bat_liters_p_min_x10 = ((1000 / (float) (cooling.bat_delta_t * 7.5))) * 10;

    static uint8_t dt_old[AVG_WINDOW_SIZE];
    static uint8_t bat_old[AVG_WINDOW_SIZE];
    static uint8_t curr;
    cooling.dt_liters_p_min_x10 = lowpass(cooling.dt_liters_p_min_x10, dt_old, curr);
    cooling.bat_liters_p_min_x10 = lowpass(cooling.bat_liters_p_min_x10, bat_old, curr);
    ++curr;
    curr = (curr == AVG_WINDOW_SIZE) ? 0 : curr;

    //Send CAN messages with flowrates
    // SEND_FLOWRATE_TEMPS(q_tx_can, cooling.bat_liters_p_min_x10, cooling.bat_therm_in_C, cooling.dt_therm_2_C,
    //                     adc_readings.dt_therm_1, adc_readings.dt_therm_2);
     DT COOLANT SYSTEM 

    // Find max motor temperature (CELSIUS)
    uint8_t max_motor_temp = MAX(car.motor_l.motor_temp,
                                 car.motor_r.motor_temp);

    // Determine if temp error
    // TODO: add in stale checking for temperatures
    cooling.dt_temp_error = 0;//(car.pch car.motor_l.data_stale ||
                            //max_motor_temp >= DT_ERROR_TEMP_C;
    // Check flow rate
    if (cooling.dt_pump && !cooling.dt_rose &&
        ((sched.os_ticks - dt_pump_start_time_ms) / 1000) > DT_FLOW_STARTUP_TIME_S)
        cooling.dt_rose = 1;
    if (cooling.dt_pump && cooling.dt_rose &&
        cooling.dt_liters_p_min_x10 < DT_MIN_FLOW_L_M * 10)
    {
        cooling.dt_flow_error = 1;
    }
    else
    {
        // TODO: how to reset error?
        //cooling.dt_flow_error = 0;
    }

    max_motor_temp = 0;
    // Determine if system should be on
    
    if ((!cooling.dt_flow_error || DT_FLOW_CHECK_OVERRIDE) &&
    (max_motor_temp > DT_PUMP_ON_TEMP_C || ((prchg_set) &&
    (cooling.dt_temp_error || DT_ALWAYS_COOL))))
    {
        if (!cooling.dt_pump)
        {
            setDtCooling(true);
        }
    }
    // Determine if system should be off
    else if (cooling.dt_pump)
    {
        setDtCooling(false);
    }
    setDtCooling(true);

     BAT COOLANT SYSTEM 

    // TODO: replace with CAN frame
    uint8_t max_bat_temp = 0;

    cooling.bat_temp_error = 1||// TODO: replace with CAN frame can_data.bat_temp.stale ||
                             max_bat_temp >= BAT_ERROR_TEMP_C;

    // Check flow rate
    if (cooling.bat_pump && !cooling.bat_rose &&
        ((sched.os_ticks - bat_pump_start_time_ms) / 1000) > BAT_FLOW_STARTUP_TIME_S)
        cooling.bat_rose = 1;
    if (cooling.bat_pump && cooling.bat_rose &&
        cooling.bat_liters_p_min_x10 < BAT_MIN_FLOW_L_M * 10)
    {
        cooling.bat_flow_error = 1;
    }
    else
    {
        // TODO: how to reset error?
        //cooling.bat_flow_error = 0;
    }

    max_bat_temp = 0;
    
    // Determine if system should be on
    if ((!cooling.bat_flow_error || BAT_FLOW_CHECK_OVERRIDE) &&
    (max_bat_temp > BAT_PUMP_ON_TEMP_C || ((prchg_set) &&
    (cooling.bat_temp_error || BAT_ALWAYS_COOL))))
    {
        if (!cooling.bat_pump)
        {
            setBatCooling(true);
        }
    }
    // Determine if system should be off
    else if (cooling.bat_pump)
    {
        setBatCooling(false);
    } 
    setBatCooling(true);
    */
}

/*void setFanPWM(void) {
    uint8_t set_dt = 0;
    uint8_t set_bat = 0;
    static uint16_t time_curr;

    if (time_curr < cooling.dt_fan_power) {
        set_dt = 1;
    }

    if (time_curr < cooling.bat_fan_power) {
        set_bat = 1;
    }

    if (++time_curr == 10) {
        time_curr = 0;
    }

    // PHAL_writeGPIO(DT_RAD_FAN_CTRL_GPIO_Port, DT_RAD_FAN_CTRL_Pin, set_dt);
    // PHAL_writeGPIO(BAT_RAD_FAN_CTRL_GPIO_Port, BAT_RAD_FAN_CTRL_Pin, set_bat);
}*/

void setDtPump(bool on)
{
    PHAL_writeGPIO(DT_PUMP_CTRL_GPIO_Port, DT_PUMP_CTRL_Pin, on);
    /*
    if (!cooling.dt_pump && on) dt_pump_start_time_ms = sched.os_ticks;
    if (!on) cooling.dt_rose = 0;
    cooling.dt_pump = on;
    PHAL_writeGPIO(DT_PUMP_CTRL_GPIO_Port, DT_PUMP_CTRL_Pin, on);
    cooling.dt_fan_power = on ? 4 : 0;
    // PHAL_writeGPIO(DT_RAD_FAN_CTRL_GPIO_Port, DT_RAD_FAN_CTRL_Pin, on);
    */
}

void setBatPump(bool on, bool on_aux)
{
    PHAL_writeGPIO(BAT_PUMP_CTRL_1_GPIO_Port, BAT_PUMP_CTRL_1_Pin, on);
    PHAL_writeGPIO(BAT_PUMP_CTRL_2_GPIO_Port, BAT_PUMP_CTRL_2_Pin, on_aux);
    /*
    if (!cooling.bat_pump && on) bat_pump_start_time_ms = sched.os_ticks;
    if (!on) cooling.bat_rose = 0;
    cooling.bat_pump = on;
    PHAL_writeGPIO(BAT_PUMP_CTRL_GPIO_Port, BAT_PUMP_CTRL_Pin, on);
    cooling.bat_fan_power = on ? 4 : 0;
    // PHAL_writeGPIO(BAT_RAD_FAN_CTRL_GPIO_Port, BAT_RAD_FAN_CTRL_Pin, on);
    */
}

/**
 * @brief Set the Bat Fan Speed
 * 
 * @param power 0 - 100%
 */
void setBatFan(uint8_t power)
{
    // TODO: PWM Control
    PHAL_writeGPIO(BAT_FAN_CTRL_GPIO_Port, BAT_FAN_CTRL_Pin, power > 0);
}

/**
 * @brief Set the DT Fan Speed
 * 
 * @param power 0 - 100%
 */
void setDtFan(uint8_t power)
{
    // TODO: PWM Control
    PHAL_writeGPIO(DT_FAN_CTRL_GPIO_Port, DT_FAN_CTRL_Pin, power > 0);
}

float rawThermtoCelcius(uint16_t t)
{
    float f;
    if (t == MAX_THERM)
        return -290;
    f = t * 3.3f / MAX_THERM; // Signal voltage
    f = THERM_R1 * f / (5 - f); // Resistance
    // resistance = (t == MAX_THERM) ? FLT_MAX : THERM_R1 * (float) t / (MAX_THERM - t);
    return (f >= 0) ? THERM_A * native_log_computation(f) + THERM_B : 0;
}

// https://stackoverflow.com/questions/9800636/calculating-natural-logarithm-and-exponent-by-core-c-for-embedded-system

static double native_log_computation(const double n) {
    // Basic logarithm computation.
    static const double euler = 2.7182818284590452354 ;
    unsigned a = 0, d;
    double b, c, e, f;
    if (n > 0) {
        for (c = n < 1 ? 1 / n : n; (c /= euler) > 1; ++a);
        c = 1 / (c * euler - 1), c = c + c + 1, f = c * c, b = 0;
        for (d = 1, c /= 2; e = b, b += 1 / (d * c), b - e/* > 0.0000001 */;)
            d += 2, c *= f;
    } else b = (n == 0) / 0.;
    return n < 1 ? -(a + b) : a + b;
}