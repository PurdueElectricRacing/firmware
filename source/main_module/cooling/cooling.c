#include "cooling.h"

// TODO: over temp checks
// TODO: read dt flow thermistors
// TODO: read bat flow thermistors

Cooling_t cooling;
volatile uint16_t raw_dt_flow_ct;
volatile uint16_t raw_bat_flow_ct;
uint32_t last_flow_meas_time_ms;
uint32_t dt_pump_start_time_ms;
uint32_t bat_pump_start_time_ms;

bool initCooling()
{
    /* Configure GPIO Interrupts */
    // enable syscfg clock
    RCC->AHB2ENR |= RCC_APB2ENR_SYSCFGEN;
    // set exti gpio source through syscfg (0b0010 means GPIOC)
    SYSCFG->EXTICR[0] |= (0b0010 << (DT_FLOW_RATE_PWM_Pin  & 0b11) * 4) |
                         (0b0010 << (BAT_FLOW_RATE_PWM_Pin & 0b11) * 4);
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
    PHAL_writeGPIO(DT_PUMP_CTRL_GPIO_Port, DT_PUMP_CTRL_Pin, 0);
    PHAL_writeGPIO(DT_RAD_FAN_CTRL_GPIO_Port, DT_RAD_FAN_CTRL_Pin, 0);
    PHAL_writeGPIO(BAT_PUMP_CTRL_GPIO_Port, BAT_PUMP_CTRL_Pin, 0);
    PHAL_writeGPIO(BAT_RAD_FAN_CTRL_GPIO_Port, BAT_RAD_FAN_CTRL_Pin, 0);

    return true;
}

void coolingPeriodic()
{

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

    // Find max motor temperature (CELSIUS)
    uint8_t max_motor_temp = MAX(can_data.front_motor_currents_temps.left_temp,
                                 can_data.front_motor_currents_temps.right_temp);
    max_motor_temp = MAX(max_motor_temp, can_data.rear_motor_currents_temps.left_temp);
    max_motor_temp = MAX(max_motor_temp, can_data.rear_motor_currents_temps.right_temp);

    // Determine if system hot enough to turn on
    if (!cooling.dt_pump && max_motor_temp > DT_PUMP_ON_TEMP_C)
    {
        cooling.dt_pump = 1;
        PHAL_writeGPIO(DT_PUMP_CTRL_GPIO_Port, DT_PUMP_CTRL_Pin, 1);
        cooling.dt_fan = 1;
        PHAL_writeGPIO(DT_RAD_FAN_CTRL_GPIO_Port, DT_RAD_FAN_CTRL_Pin, 1);

        dt_pump_start_time_ms = sched.os_ticks;
        cooling.dt_rose = 0;
    }
    // Determine if system cool enough to turn off
    else if (cooling.dt_pump && max_motor_temp < DT_PUMP_OFF_TEMP_C)
    {
        cooling.dt_pump = 0;
        PHAL_writeGPIO(DT_PUMP_CTRL_GPIO_Port, DT_PUMP_CTRL_Pin, 0);
        cooling.dt_fan = 0;
        PHAL_writeGPIO(DT_RAD_FAN_CTRL_GPIO_Port, DT_RAD_FAN_CTRL_Pin, 0);
        cooling.dt_rose = 0;
    }
    // Check flow rate
    if (cooling.dt_pump && !cooling.dt_rose && 
        ((sched.os_ticks - dt_pump_start_time_ms) / 1000) > DT_FLOW_STARTUP_TIME_S)
        cooling.dt_rose = 1;
    if (cooling.bat_pump && cooling.dt_rose && 
        cooling.dt_liters_p_min < DT_MIN_FLOW_L_M)
    {
        cooling.dt_flow_error = 1;
        // TODO: act on dt flow error
        // TODO: clear dt flow error
    }

    /* BAT COOLANT SYSTEM */

    // TODO: replace with CAN frame
    uint8_t max_bat_temp = 0;

    // Determine if system hot enough to turn on
    if (!cooling.bat_pump && max_bat_temp > BAT_PUMP_ON_TEMP_C)
    {
        cooling.bat_pump = 1;
        PHAL_writeGPIO(BAT_PUMP_CTRL_GPIO_Port, BAT_PUMP_CTRL_Pin, 1);
        cooling.bat_fan = 1;
        PHAL_writeGPIO(BAT_RAD_FAN_CTRL_GPIO_Port, BAT_RAD_FAN_CTRL_Pin, 1);

        bat_pump_start_time_ms = sched.os_ticks;
        cooling.bat_rose = 0;
    }
    // Determine if system cool enough to turn off
    else if (cooling.bat_pump && max_bat_temp < BAT_PUMP_OFF_TEMP_C)
    {
        cooling.bat_pump = 0;
        PHAL_writeGPIO(BAT_PUMP_CTRL_GPIO_Port, BAT_PUMP_CTRL_Pin, 0);
        cooling.bat_fan = 0;
        PHAL_writeGPIO(BAT_RAD_FAN_CTRL_GPIO_Port, BAT_RAD_FAN_CTRL_Pin, 0);
        cooling.bat_rose = 0;
    }
    // Check flow rate
    if (cooling.bat_pump && !cooling.bat_rose && 
        ((sched.os_ticks - bat_pump_start_time_ms) / 1000) > BAT_FLOW_STARTUP_TIME_S)
        cooling.bat_rose = 1;
    if (cooling.bat_pump && cooling.bat_rose && 
        cooling.bat_liters_p_min < BAT_MIN_FLOW_L_M)
    {
        cooling.bat_flow_error = 1;
        // TODO: act on bat flow error
        // TODO: clear bat flow error
    }
}

float rawThermtoCelcius(uint16_t t)
{
    float resistance = THERM_R1 * t / (MAX_THERM - t);
    return THERM_A * log(resistance) + THERM_B;
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