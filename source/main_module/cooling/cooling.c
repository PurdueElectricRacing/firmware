#include "cooling.h"

Cooling_t cooling = {0};
volatile uint16_t raw_dt_flow_ct;
volatile uint16_t raw_bat_flow_ct;
uint32_t last_flow_meas_time_ms;
uint32_t dt_pump_start_time_ms;
uint32_t bat_pump_start_time_ms;

static void setDtCooling(uint8_t on);
static void setBatCooling(uint8_t on);

bool coolingInit()
{
    /* Configure GPIO Interrupts */
    // enable syscfg clock
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    // set exti gpio source through syscfg (0b0010 means GPIOC)
    SYSCFG->EXTICR[0] |= (((uint16_t)0b0010) << (DT_FLOW_RATE_PWM_Pin  & 0b11) * 4) |
                         (((uint16_t)0b0010) << (BAT_FLOW_RATE_PWM_Pin & 0b11) * 4);
    // unmask the interrupt
    EXTI->IMR1 |= (0b1 << DT_FLOW_RATE_PWM_Pin) | 
                  (0b1 << BAT_FLOW_RATE_PWM_Pin);
    // set trigger to rising edge
    EXTI->RTSR1 |= (0b1 << DT_FLOW_RATE_PWM_Pin) |
                   (0b1 << BAT_FLOW_RATE_PWM_Pin);
    // enable the interrupt handlers
    NVIC_EnableIRQ(EXTI1_IRQn);
    NVIC_EnableIRQ(EXTI2_IRQn);

    // Default pin configurations
    setDtCooling(0);
    setBatCooling(0);

    return true;
}

void coolingPeriodic()
{
    /* WATER TEMP CALCULATIONS */

    // cooling.dt_therm_1_C    = rawThermtoCelcius(adc_readings.dt_therm_1);
    // cooling.dt_therm_2_C    = rawThermtoCelcius(adc_readings.dt_therm_2);
    // cooling.bat_therm_in_C  = rawThermtoCelcius(adc_readings.bat_therm_in);
    // cooling.bat_therm_out_C = rawThermtoCelcius(adc_readings.bat_therm_out);

    /* FLOW CALCULATIONS */
    // Calculate time delta
    uint32_t flow_dt_ms = sched.os_ticks - last_flow_meas_time_ms;
    last_flow_meas_time_ms = sched.os_ticks;
    // Store and reset sensor tick counters
    uint16_t dt_flow_ct = raw_dt_flow_ct;
    raw_dt_flow_ct = 0;
    uint16_t bat_flow_ct = raw_bat_flow_ct;
    raw_bat_flow_ct = 0;

    // Convert ticks and time delta to liters per minute
    cooling.dt_liters_p_min =  (uint8_t) ((((uint32_t) dt_flow_ct)  * 1000 * 60) / 
                                          (flow_dt_ms * PULSES_P_LITER));
    cooling.bat_liters_p_min = (uint8_t) ((((uint32_t) bat_flow_ct) * 1000 * 60) / 
                                          (flow_dt_ms * PULSES_P_LITER));

    
    /* DT COOLANT SYSTEM */
    bool next_coolant_state = cooling.dt_pump;

    // Find max motor temperature (CELSIUS)
    uint8_t max_motor_temp = MAX(can_data.front_motor_currents_temps.left_temp,
                                 can_data.front_motor_currents_temps.right_temp);
    max_motor_temp = MAX(max_motor_temp, can_data.rear_motor_currents_temps.left_temp);
    max_motor_temp = MAX(max_motor_temp, can_data.rear_motor_currents_temps.right_temp);

    // Determine if temp error
    cooling.dt_temp_error = can_data.front_motor_currents_temps.stale ||
                            can_data.rear_motor_currents_temps.stale  ||
                            max_motor_temp >= DT_ERROR_TEMP_C;
    // Check flow rate
    if (cooling.dt_pump && !cooling.dt_rose && 
        ((sched.os_ticks - dt_pump_start_time_ms) / 1000) > DT_FLOW_STARTUP_TIME_S)
        cooling.dt_rose = 1;
    if (cooling.dt_pump && cooling.dt_rose && 
        cooling.dt_liters_p_min < DT_MIN_FLOW_L_M)
    {
        cooling.dt_flow_error = 1;
    }
    else
    {
        // TODO: how to reset error?
        //cooling.dt_flow_error = 0;
    }

    // Determine if system should be on
    if (!cooling.dt_pump && !cooling.dt_flow_error && 
       (max_motor_temp > DT_PUMP_ON_TEMP_C || cooling.dt_temp_error))
    {
        setDtCooling(true);
    }
    // Determine if system should be off
    else if (cooling.dt_pump && (max_motor_temp < DT_PUMP_OFF_TEMP_C || cooling.dt_flow_error))
    {
        setDtCooling(false);
    }


    /* BAT COOLANT SYSTEM */
    next_coolant_state = cooling.bat_fan;

    // TODO: replace with CAN frame
    uint8_t max_bat_temp = 0;

    cooling.bat_temp_error = 1||// TODO: replace with CAN frame can_data.bat_temp.stale ||
                             max_bat_temp >= BAT_ERROR_TEMP_C;

    // Check flow rate
    if (cooling.bat_pump && !cooling.bat_rose && 
        ((sched.os_ticks - bat_pump_start_time_ms) / 1000) > BAT_FLOW_STARTUP_TIME_S)
        cooling.bat_rose = 1;
    if (cooling.bat_pump && cooling.bat_rose && 
        cooling.bat_liters_p_min < BAT_MIN_FLOW_L_M)
    {
        cooling.bat_flow_error = 1;
    }
    else
    {
        // TODO: how to reset error?
        //cooling.bat_flow_error = 0;
    }

    // Determine if system should be on
    if (!cooling.bat_pump && !cooling.bat_flow_error && 
       (max_motor_temp > BAT_PUMP_ON_TEMP_C || cooling.bat_temp_error))
    {
        setBatCooling(true);
    }
    // Determine if system should be off
    else if (cooling.bat_pump && (max_motor_temp < BAT_PUMP_OFF_TEMP_C || cooling.bat_flow_error))
    {
        setBatCooling(false);
    }
}

void setDtCooling(uint8_t on)
{
    if (!cooling.dt_pump && on) dt_pump_start_time_ms = sched.os_ticks;
    if (!on) cooling.dt_rose = 0;
    cooling.dt_pump = on;
    PHAL_writeGPIO(DT_PUMP_CTRL_GPIO_Port, DT_PUMP_CTRL_Pin, on);
    cooling.dt_fan = on;
    PHAL_writeGPIO(DT_RAD_FAN_CTRL_GPIO_Port, DT_RAD_FAN_CTRL_Pin, on);
}

void setBatCooling(uint8_t on)
{
    if (!cooling.bat_pump && on) bat_pump_start_time_ms = sched.os_ticks;
    if (!on) cooling.bat_rose = 0;
    cooling.bat_pump = on;
    PHAL_writeGPIO(BAT_PUMP_CTRL_GPIO_Port, BAT_PUMP_CTRL_Pin, on);
    cooling.bat_fan = on;
    PHAL_writeGPIO(BAT_RAD_FAN_CTRL_GPIO_Port, BAT_RAD_FAN_CTRL_Pin, on);
}

float rawThermtoCelcius(uint16_t t)
{
    float resistance;
    resistance = (t == MAX_THERM) ? FLT_MAX : THERM_R1 * t / (MAX_THERM - t);
    return (resistance > 0) ? THERM_A * log(resistance) + THERM_B : 0;
}


/* Interrupt handlers for counting sensor ticks */

void EXTI1_IRQHandler()
{
    // check pin responsible
    if (EXTI->PR1 & (0b1 << DT_FLOW_RATE_PWM_Pin))
    {
        raw_dt_flow_ct++;
        // clear flag
        EXTI->PR1 = (0b1 << DT_FLOW_RATE_PWM_Pin);
    }
}

void EXTI2_IRQHandler()
{
    // check pin responsible
    if (EXTI->PR1 & (0b1 << BAT_FLOW_RATE_PWM_Pin))
    {
        raw_bat_flow_ct++;
        // clear flag
        EXTI->PR1 = (0b1 << BAT_FLOW_RATE_PWM_Pin);
    }
}