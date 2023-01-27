#include "cooling.h"

#define PWM_FREQUENCY 25000  //PWM frequency to be 25kHz

Cooling_t cooling = {0};
volatile uint16_t raw_dt_flow_ct;
volatile uint16_t raw_bat_flow_ct;
uint32_t last_flow_meas_time_ms;
uint32_t dt_pump_start_time_ms;
uint32_t bat_pump_start_time_ms;

extern q_handle_t q_tx_can;
extern uint32_t APB1ClockRateHz;


static void setDtCooling(uint8_t on);
static void setBatCooling(uint8_t on);
uint8_t lowpass(uint8_t new, uint8_t *old, uint8_t curr);

bool coolingInit()
{
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


    //Init Timer
    int arrValue = 100;

    TIM4 -> CR1 &= ~TIM_CR1_CEN; //Turning off counter

    RCC -> APB1ENR1 |= RCC_APB1ENR1_TIM4EN; //0b100;
    TIM4 -> PSC = (APB1ClockRateHz / (PWM_FREQUENCY * arrValue)) - 1; 
    TIM4 -> ARR = arrValue - 1; //setting it to 99 so it's easier to use it with Duty Cycle

    //Set Channels 1 and 2 to 110 (Mode 1 up counter) -> (active while CNT <= CCR)
    TIM4 -> CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1; 
    TIM4 -> CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1;

    //Enable Channels 1 and 2 outputs 
    TIM4 -> CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E;

    //Setting pwm to 0
    TIM4 -> CCR1 = 0;
    TIM4 -> CCR2 = 0;

    //Enable counter as long as ccrs are 0
    TIM4 -> CR1 |= TIM_CR1_CEN; // turning on counter

    // Default pin configurations
    setDtCooling(0);
    setBatCooling(0);


    //FAN TACHOMETER PART
    /////////////////////

    //BAT Fan Tachometer: PE0 - TIM16_CH1

    //init timer
    TIM16 -> CR1 &= ~TIM_CR1_CEN; //Turning off counter
    TIM16 -> CCER &= ~TIM_CCER_CC1E; //Turning off capture / compare

    RCC -> APB2ENR |= RCC_APB2ENR_TIM16EN; //0b100;
    TIM16 -> PSC = (APB1ClockRateHz / (PWM_FREQUENCY * arrValue)) - 1; 
    TIM16 -> ARR = arrValue - 1; //setting it to 99 so it's easier to use it with Duty Cycle

//     Select the active input: TIMx_CCR1 must be linked to the TI1 input, so write the CC1S
    // bits to 01 in the TIMx_CCMR1 register. As soon as CC1S becomes different from 00,
    // the channel is configured in input and the TIMx_CCR1 register becomes read-only.Page 1103
    TIM16 -> CCMR1 &= ~TIM_CCMR1_CC1S;
    TIM16 -> CCMR1 |= TIM_CCMR1_CC1S_0; //setting as input


    // 2. Program the appropriate input filter duration in relation with the signal connected to the
    // timer (when the input is one of the TIx (ICxF bits in the TIMx_CCMRx register). Let’s
    // imagine that, when toggling, the input signal is not stable during at least 5 internal clock
    // cycles. We must program a filter duration longer than these 5 clock cycles. We can
    // validate a transition on TI1 when 8 consecutive samples with the new level have been
    // detected (sampled at fDTS frequency). Then write IC1F bits to 0011 in the
    // TIMx_CCMR1 register
    TIM16 -> CCMR1 &= ~TIM_CCMR1_IC1F;
    TIM16 -> CCMR1 |= TIM_CCMR1_IC1F_0 | TIM_CCMR1_IC1F_1; //setting transition stability

    //3. Select the edge of the active transition on the TI1 channel 
    //CC1NP=1, CC1P=1: non-inverted/both edges/ The circuit is sensitive to both TIxFP1 rising
    // and falling edges (capture or trigger operations in reset, external clock
    // or trigger mode), TIxFP1is not inverted (trigger operation in gated
    // mode). This configuration must not be used in encoder mode.

    TIM16 -> CCER |= TIM_CCER_CC1P | TIM_CCER_CC1NP; //setting edge detection to both rising and falling

    // 4. Program the input prescaler. In our example, we wish the capture to be performed at
    // each valid transition, so the prescaler is disabled (write IC1PS bits to ‘00’ in the
    // TIMx_CCMR1 register).
    TIM16 -> CCMR1 &= ~TIM_CCMR1_IC1PSC; //setting cmr1 to capture at every transition

    // 5. Enable capture from the counter into the capture register by setting the CC1E bit in the
    // TIMx_CCER register.
    TIM16 -> CCER |= TIM_CCER_CC1E; //enabling capture/compare channel

    // 6. If needed, enable the related interrupt request by setting the CC1IE bit in the
    // TIMx_DIER register, and/or the DMA request by setting the CC1DE bit in the
    // TIMx_DIER register.
    TIM16 -> DIER |= TIM_DIER_CC1IE; //

    TIM16 -> CR1 |= TIM_CR1_CEN;


    //DT Fan Tachometer 
    ///////////////////

    ///repeat the steps above but for TIM17
    TIM17 -> CR1 &= ~TIM_CR1_CEN; //Turning off counter
    TIM17 -> CCER &= ~TIM_CCER_CC1E; //Turning off capture / compare

    RCC -> APB2ENR |= RCC_APB2ENR_TIM17EN;
    TIM17 -> PSC = (APB1ClockRateHz / (PWM_FREQUENCY * arrValue)) - 1; 
    TIM17 -> ARR = arrValue - 1; //setting it to 99 so it's easier to use it with Duty Cycle

    TIM17 -> CCMR1 &= ~TIM_CCMR1_CC1S;
    TIM17 -> CCMR1 |= TIM_CCMR1_CC1S_0; //setting as input

    TIM17 -> CCER |= TIM_CCER_CC1P | TIM_CCER_CC1NP; //setting edge detection to both rising and falling

    TIM17 -> CCMR1 &= ~TIM_CCMR1_IC1PSC; //setting cmr1 to capture at every transition

    TIM17 -> CCER |= TIM_CCER_CC1E; //enabling capture/compare channel

    TIM17 -> DIER |= TIM_DIER_CC1IE; //selecting Capture compare interrupt flag

    TIM17 -> CR1 |= TIM_CR1_CEN; //enabling counter

    return true;
}

//void  TIM16_CC_IRQHandler() (wondering which one I should use?)
int volatile ccrBatFanTachometer = 0;
void TIM16_IRQHandler()
{
    //Check if flag was set
    if (TIM16 -> SR & TIM_SR_CC1IF)
    {
        //read the value of the counter to know the pulse width
        //at the moment of the transition
        ccrBatFanTachometer = TIM16 -> CCR1;

        //Reset the bit
        TIM16 -> SR &= ~TIM_SR_CC1IF;
    }
}

int volatile ccrDTFanTachometer = 0;
void TIM17_IRQHandler()
{
    //Check if flag was set
    if (TIM17 -> SR & TIM_SR_CC1IF)
    {
        //read the value of the counter to know the pulse width
        //at the moment of the transition
        ccrDTFanTachometer = TIM17 -> CCR1;

        //Reset the bit
        TIM17 -> SR &= ~TIM_SR_CC1IF;
    }
}

void setBATFan (uint8_t dutyCycle)
{
    TIM4 -> CCR1 = dutyCycle; //Setting duty cycle for channel 1
}

void setDTFan (uint8_t dutyCycle)
{
    TIM4 -> CCR2 = dutyCycle; //Setting duty cycle for channel 1
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

    cooling.dt_therm_1_C    = rawThermtoCelcius(adc_readings.dt_therm_1);
    cooling.dt_therm_2_C    = rawThermtoCelcius(adc_readings.dt_therm_2);
    cooling.bat_therm_in_C  = rawThermtoCelcius(adc_readings.bat_therm_in);
    cooling.bat_therm_out_C = rawThermtoCelcius(adc_readings.bat_therm_out);

    /* FLOW CALCULATIONS */
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
    SEND_FLOWRATE_TEMPS(q_tx_can, cooling.bat_liters_p_min_x10, cooling.bat_therm_in_C, cooling.dt_therm_2_C,
                        adc_readings.dt_therm_1, adc_readings.dt_therm_2);
    /* DT COOLANT SYSTEM */

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
    /*
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
    }*/
    setDtCooling(true);

    /* BAT COOLANT SYSTEM */

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
    /*
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
    } */
    setBatCooling(true);

}

void setFanPWM(void) {
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
}

void setDtCooling(uint8_t on)
{
    if (!cooling.dt_pump && on) dt_pump_start_time_ms = sched.os_ticks;
    if (!on) cooling.dt_rose = 0;
    cooling.dt_pump = on;
    PHAL_writeGPIO(DT_PUMP_CTRL_GPIO_Port, DT_PUMP_CTRL_Pin, on);
    cooling.dt_fan_power = on ? 4 : 0;
    // PHAL_writeGPIO(DT_RAD_FAN_CTRL_GPIO_Port, DT_RAD_FAN_CTRL_Pin, on);
}

void setBatCooling(uint8_t on)
{
    /*
    if (!cooling.bat_pump && on) bat_pump_start_time_ms = sched.os_ticks;
    if (!on) cooling.bat_rose = 0;
    cooling.bat_pump = on;
    PHAL_writeGPIO(BAT_PUMP_CTRL_GPIO_Port, BAT_PUMP_CTRL_Pin, on);
    cooling.bat_fan_power = on ? 4 : 0;
    // PHAL_writeGPIO(BAT_RAD_FAN_CTRL_GPIO_Port, BAT_RAD_FAN_CTRL_Pin, on);
    */
}

float rawThermtoCelcius(uint16_t t)
{
    float resistance;
    if (t == MAX_THERM)
        return -290;
    float s = t * 3.3 / MAX_THERM;
    resistance = THERM_R1 * s / (5 - s);
    // resistance = (t == MAX_THERM) ? FLT_MAX : THERM_R1 * (float) t / (MAX_THERM - t);
    return (resistance > 0) ? THERM_A * native_log_computation(resistance) + THERM_B : 0;
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