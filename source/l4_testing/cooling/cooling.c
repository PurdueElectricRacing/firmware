#include "cooling.h"
#include "stm32l432xx.h"

#define PWM_FREQUENCY 25000  //PWM frequency to be 25kHz

Cooling_t cooling = {0};
volatile uint16_t raw_dt_flow_ct;
volatile uint16_t raw_bat_flow_ct;
uint32_t last_flow_meas_time_ms;
uint32_t dt_pump_start_time_ms;
uint32_t bat_pump_start_time_ms;

extern q_handle_t q_tx_can;
extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;


static void setDtCooling(uint8_t on);
static void setBatCooling(uint8_t on);
uint8_t lowpass(uint8_t new, uint8_t *old, uint8_t curr);

volatile int ccrBatFanTachometer = 0;

bool coolingInit()
{

    // // TODO: Enable GPIO A
    // RCC -> AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    // // TODO: Configure the PA8-9 to be the outputs of TIM1 Ch 1-2
    // // TODO: First we clear their MODER bits
    // GPIOA -> MODER &= 0xFFF0FFFF;

    // // TODO: Then we set them to AF mode
    // GPIOA -> MODER |= 0x000A0000;


    // // TODO: Set PA8-9 to use AF1 since this corresponds to the TIM1 Ch1 -2
    // // AFR[1] -> AFRH
    // GPIOA -> AFR[1] &= 0xFFFFFF00; //clearing pins 8 and 9
    // GPIOA -> AFR[1] |= 0xFFFFFF11; //setting AF1 for those pins


    //Init Timer
    int arrValue = 100;

    //FOR L4 TESTING PURPOSES WE WILL USE TIM1 connected to GPIOA Pin 8 (D9) and GPIOA Pin 9 (D1) for channels 1 and 2 respectively
    //BAT_FAN_CTRL_Pin -> GPIOA Pin 8 (D9)
    //DT_FAN_CTRL_Pin - > GPIOA Pin 9 (D1) 
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    TIM1 -> CR1 &= ~TIM_CR1_CEN; //Turning off counter

    RCC -> APB2ENR |= RCC_APB2ENR_TIM1EN; 
    TIM1 -> PSC = (APB2ClockRateHz / (PWM_FREQUENCY * arrValue)) - 1; 
    TIM1 -> ARR = arrValue - 1; //setting it to 99 so it's easier to use it with Duty Cycle

    //Enabling the MOE bit of the dead-time register
    TIM1 -> BDTR |= TIM_BDTR_MOE;

    //Set Channels 1 and 2 to 110 (Mode 1 up counter) -> (active while CNT <= CCR)
    TIM1 -> CCMR1 &= ~(TIM_CCMR1_OC1M_0);
    TIM1 -> CCMR1 &= ~(TIM_CCMR1_OC2M_0);
    TIM1 -> CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1; 
    TIM1 -> CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1;

    //Setting the preload register
    TIM1 -> CCMR1 |= TIM_CCMR1_OC1PE | TIM_CCMR1_OC2PE;

    // TIM1 -> CR1 |= TIM_CR1_ARPE;

    //Enable Channels 1 and 2 outputs 
    TIM1 -> CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E;

    //Enable counter as long as ccrs are 0
    TIM1 -> CR1 |= TIM_CR1_CEN; // turning on counter

    //Setting pwm to 0
    TIM1 -> CCR1 = 0;
    TIM1 -> CCR2 = 0;

    // Default pin configurations
    setDtCooling(0);
    setBatCooling(0);


    //FAN TACHOMETER PART
    /////////////////////

    //BAT Fan Tachometer: PA0 - TIM2_CH1 -> A0

    //init timer
    TIM2 -> CR1 &= ~TIM_CR1_CEN; //Turning off counter
    TIM2 -> CCER &= ~TIM_CCER_CC1E; //Turning off capture / compare

    RCC -> APB1ENR1 |= RCC_APB1ENR1_TIM2EN; //0b100;
    TIM2 -> PSC = (APB2ClockRateHz / (PWM_FREQUENCY * arrValue)) - 1; 
    TIM2 -> ARR = arrValue - 1; //setting it to 99 so it's easier to use it with Duty Cycle

    TIM2 -> CCR1 = 0; //initializing CCR to 0
    ccrBatFanTachometer = 0;

    TIM2 -> CCMR1 &= ~TIM_CCMR1_CC1S;
    TIM2 -> CCMR1 |= TIM_CCMR1_CC1S_0; //setting as input

    TIM2 -> CCMR1 &= ~TIM_CCMR1_IC1F;
    TIM2 -> CCMR1 |= TIM_CCMR1_IC1F_0 | TIM_CCMR1_IC1F_1; //setting transition stability

    TIM2 -> CCMR1 &= ~TIM_CCMR1_IC1PSC; //setting ccmr1 to capture at every transition

    TIM2 -> CCER |= TIM_CCER_CC1P; //Trigger at Rising edges

    TIM2 -> CCER |= TIM_CCER_CC1E; //enabling capture/compare channel

    TIM2 -> DIER |= TIM_DIER_CC1IE; //

    NVIC_EnableIRQ(TIM2_IRQn);


    TIM2 -> CR1 |= TIM_CR1_CEN;

		/*Read captured value*/

    //DT Fan Tachometer 
    ///////////////////

    ///repeat the steps above but for TIM17
    // TIM17 -> CR1 &= ~TIM_CR1_CEN; //Turning off counter
    // TIM17 -> CCER &= ~TIM_CCER_CC1E; //Turning off capture / compare

    // RCC -> APB2ENR |= RCC_APB2ENR_TIM17EN;
    // TIM17 -> PSC = (APB1ClockRateHz / (PWM_FREQUENCY * arrValue)) - 1; 
    // TIM17 -> ARR = arrValue - 1; //setting it to 99 so it's easier to use it with Duty Cycle

    // TIM17 -> CCMR1 &= ~TIM_CCMR1_CC1S;
    // TIM17 -> CCMR1 |= TIM_CCMR1_CC1S_0; //setting as input


    // TIM17 -> CCER |= TIM_CCER_CC1P | TIM_CCER_CC1NP; //setting edge detection to both rising and falling

    // TIM17 -> CCMR1 &= ~TIM_CCMR1_IC1PSC; //setting cmr1 to capture at every transition


    // TIM17 -> CCER |= TIM_CCER_CC1E; //enabling capture/compare channel

    // TIM17 -> DIER |= TIM_DIER_CC1IE; //selecting Capture compare interrupt flag

    // TIM17 -> CR1 |= TIM_CR1_CEN; //enabling counter

    return true;
}

// void  TIM2_CC_IRQHandler() //(wondering which one I should use?)
// { 
//     //Reset the bit
//     TIM2 -> SR &= ~TIM_SR_CC1IF;  

//     //read the value of the counter to know the pulse width
//     //at the moment of the transition
//     ccrBatFanTachometer = TIM2 -> CCR1;   

//     // Enable the next update event
//     TIM2->EGR |= TIM_EGR_UG; 
// }

void  TIM2_IRQHandler()
{
  
    if (TIM2->SR & TIM_SR_CC1IF)
    {
        TIM2->SR &= ~(TIM_SR_CC1IF); //clear flag

        ccrBatFanTachometer = (TIM2->CCR1) - ccrBatFanTachometer;

        ccrBatFanTachometer = abs(ccrBatFanTachometer);

        // Enable the next update event
        TIM2->EGR |= TIM_EGR_UG; 
    }
}

// int volatile ccrDTFanTachometer = 0;
// void TIM17_IRQHandler()
// {
//     //Check if flag was set
//     if (TIM17 -> SR & TIM_SR_CC1IF)
//     {
//         //read the value of the counter to know the pulse width
//         //at the moment of the transition
//         ccrDTFanTachometer = TIM17 -> CCR1;

//         //Reset the bit
//         TIM17 -> SR &= ~TIM_SR_CC1IF;
//     }
// }

void setBATFan (uint8_t dutyCycle)
{
    TIM1 -> CCR1 = dutyCycle; //Setting duty cycle for channel 1
}

void setDTFan (uint8_t dutyCycle)
{
    TIM1 -> CCR2 = dutyCycle; //Setting duty cycle for channel 1
}

uint8_t lowpass(uint8_t new, uint8_t *old, uint8_t curr) {
    uint8_t i;
    float average = 0;

    old[curr] = new;

    for (i = 0; i < AVG_WINDOW_SIZE; i++) {
        average += (float) old[i];
    }

    return (uint8_t) (average / AVG_WINDOW_SIZE);
}

void coolingPeriodic()
{
    /* WATER TEMP CALCULATIONS */

    // cooling.dt_therm_1_C    = rawThermtoCelcius(adc_readings.dt_therm_1);
    // cooling.dt_therm_2_C    = rawThermtoCelcius(adc_readings.dt_therm_2);
    // cooling.bat_therm_in_C  = rawThermtoCelcius(adc_readings.bat_therm_in);
    // cooling.bat_therm_out_C = rawThermtoCelcius(adc_readings.bat_therm_out);

    // /* FLOW CALCULATIONS */
    // // Convert ticks and time delta to liters per minute
    // if (cooling.dt_delta_t == 0)
    //     cooling.dt_liters_p_min_x10 = 0;
    // else
    //     cooling.dt_liters_p_min_x10 = ((1000 / (float) (cooling.dt_delta_t * 7.5))) * 10;
    // if (cooling.bat_delta_t == 0)
    //     cooling.bat_delta_t = 0;
    // else
    //     cooling.bat_liters_p_min_x10 = ((1000 / (float) (cooling.bat_delta_t * 7.5))) * 10;

    // static uint8_t dt_old[AVG_WINDOW_SIZE];
    // static uint8_t bat_old[AVG_WINDOW_SIZE];
    // static uint8_t curr;
    // cooling.dt_liters_p_min_x10 = lowpass(cooling.dt_liters_p_min_x10, dt_old, curr);
    // cooling.bat_liters_p_min_x10 = lowpass(cooling.bat_liters_p_min_x10, bat_old, curr);
    // ++curr;
    // curr = (curr == AVG_WINDOW_SIZE) ? 0 : curr;

    // //Send CAN messages with flowrates
    // SEND_FLOWRATE_TEMPS(q_tx_can, cooling.bat_liters_p_min_x10, cooling.bat_therm_in_C, cooling.dt_therm_2_C,
    //                     adc_readings.dt_therm_1, adc_readings.dt_therm_2);
    // /* DT COOLANT SYSTEM */

    // // Find max motor temperature (CELSIUS)
    // uint8_t max_motor_temp = MAX(can_data.front_motor_currents_temps.left_temp,
    //                              can_data.front_motor_currents_temps.right_temp);
    // max_motor_temp = MAX(max_motor_temp, can_data.rear_motor_currents_temps.left_temp);
    // max_motor_temp = MAX(max_motor_temp, can_data.rear_motor_currents_temps.right_temp);

    // // Determine if temp error
    // cooling.dt_temp_error = can_data.front_motor_currents_temps.stale ||
    //                         can_data.rear_motor_currents_temps.stale  ||
    //                         max_motor_temp >= DT_ERROR_TEMP_C;
    // // Check flow rate
    // if (cooling.dt_pump && !cooling.dt_rose &&
    //     ((sched.os_ticks - dt_pump_start_time_ms) / 1000) > DT_FLOW_STARTUP_TIME_S)
    //     cooling.dt_rose = 1;
    // if (cooling.dt_pump && cooling.dt_rose &&
    //     cooling.dt_liters_p_min_x10 < DT_MIN_FLOW_L_M * 10)
    // {
    //     cooling.dt_flow_error = 1;
    // }
    // else
    // {
    //     // TODO: how to reset error?
    //     //cooling.dt_flow_error = 0;
    // }

    // max_motor_temp = 0;
    // // Determine if system should be on
    // /*
    // if ((!cooling.dt_flow_error || DT_FLOW_CHECK_OVERRIDE) &&
    // (max_motor_temp > DT_PUMP_ON_TEMP_C || ((prchg_set) &&
    // (cooling.dt_temp_error || DT_ALWAYS_COOL))))
    // {
    //     if (!cooling.dt_pump)
    //     {
    //         setDtCooling(true);
    //     }
    // }
    // // Determine if system should be off
    // else if (cooling.dt_pump)
    // {
    //     setDtCooling(false);
    // }*/
    // setDtCooling(true);

    // /* BAT COOLANT SYSTEM */

    // // TODO: replace with CAN frame
    // uint8_t max_bat_temp = 0;

    // cooling.bat_temp_error = 1||// TODO: replace with CAN frame can_data.bat_temp.stale ||
    //                          max_bat_temp >= BAT_ERROR_TEMP_C;

    // // Check flow rate
    // if (cooling.bat_pump && !cooling.bat_rose &&
    //     ((sched.os_ticks - bat_pump_start_time_ms) / 1000) > BAT_FLOW_STARTUP_TIME_S)
    //     cooling.bat_rose = 1;
    // if (cooling.bat_pump && cooling.bat_rose &&
    //     cooling.bat_liters_p_min_x10 < BAT_MIN_FLOW_L_M * 10)
    // {
    //     cooling.bat_flow_error = 1;
    // }
    // else
    // {
    //     // TODO: how to reset error?
    //     //cooling.bat_flow_error = 0;
    // }

    // max_bat_temp = 0;
    // /*
    // // Determine if system should be on
    // if ((!cooling.bat_flow_error || BAT_FLOW_CHECK_OVERRIDE) &&
    // (max_bat_temp > BAT_PUMP_ON_TEMP_C || ((prchg_set) &&
    // (cooling.bat_temp_error || BAT_ALWAYS_COOL))))
    // {
    //     if (!cooling.bat_pump)
    //     {
    //         setBatCooling(true);
    //     }
    // }
    // // Determine if system should be off
    // else if (cooling.bat_pump)
    // {
    //     setBatCooling(false);
    // } */
    // setBatCooling(true);

}

void setFanPWM(void) {
    // uint8_t set_dt = 0;
    // uint8_t set_bat = 0;
    // static uint16_t time_curr;

    // if (time_curr < cooling.dt_fan_power) {
    //     set_dt = 1;
    // }

    // if (time_curr < cooling.bat_fan_power) {
    //     set_bat = 1;
    // }

    // if (++time_curr == 10) {
    //     time_curr = 0;
    // }

    // PHAL_writeGPIO(DT_RAD_FAN_CTRL_GPIO_Port, DT_RAD_FAN_CTRL_Pin, set_dt);
    // PHAL_writeGPIO(BAT_RAD_FAN_CTRL_GPIO_Port, BAT_RAD_FAN_CTRL_Pin, set_bat);
}

void setDtCooling(uint8_t on)
{
    // if (!cooling.dt_pump && on) dt_pump_start_time_ms = sched.os_ticks;
    // if (!on) cooling.dt_rose = 0;
    // cooling.dt_pump = on;
    // PHAL_writeGPIO(DT_PUMP_CTRL_GPIO_Port, DT_PUMP_CTRL_Pin, on);
    // cooling.dt_fan_power = on ? 4 : 0;
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