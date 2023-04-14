#include "cooling.h"

const float adc_to_ln[] = {0.0f, 3.66932195489841f, 4.36639937878818f, 4.77581023800821f, 5.06745369221869f, 5.2945743802692f, 5.4808889545972f, 5.63904866008963f, 5.77660521538147f, 5.89842968114079f, 6.00784802635851f, 6.1072325688147f, 6.19833497682335f, 6.28248552082528f, 6.36071827325357f, 6.43385300932074f, 6.50255062145495f, 6.56735170457577f, 6.62870409572963f, 6.68698295786607f, 6.74250570608683f, 6.79554328837521f, 6.84632883966985f, 6.89506441066586f, 6.94192626350331f, 6.98706908570186f, 7.03062937713267f, 7.07272819744148f, 7.11347341358249f, 7.1529615527881f, 7.19127934128525f, 7.22850499062208f, 7.26470927971074f, 7.29995647032355f, 7.33430508588904f, 7.3678085773747f, 7.40051589535049f, 7.4324719836627f, 7.4637182072667f, 7.49429272448382f, 7.52423081212813f, 7.55356515048801f, 7.58232607396915f, 7.61054179224856f, 7.63823858600859f, 7.66544098067965f, 7.69217190109277f, 7.71845280950592f, 7.74430382910538f, 7.76974385477934f, 7.7947906527075f, 7.81946095009564f, 7.84377051620407f, 7.86773423566524f, 7.89136617495574f, 7.91467964277671f, 7.9376872450016f, 7.96040093476842f, 7.98283205822374f, 8.00499139636441f, 8.02688920337104f, 8.04853524178147f, 8.06993881481267f, 8.09110879610534f, 8.11205365713481f, 8.13278149250591f, 8.1533000433256f, 8.17361671882747f, 8.19373861640354f, 8.21367254018327f, 8.23342501828559f, 8.25300231885707f, 8.27241046499855f, 8.29165524867247f, 8.31074224367451f, 8.32967681774518f, 8.34846414389012f, 8.36710921097153f, 8.38561683362752f, 8.40399166157109f, 8.42223818831605f, 8.44036075937295f, 8.45836357995433f, 8.47625072222564f, 8.4940261321347f, 8.5116936358502f, 8.529256945837f, 8.5467196665941f, 8.56408530007868f, 8.58135725083814f, 8.59853883087019f, 8.61563326422949f, 8.63264369139813f, 8.64957317343572f, 8.66642469592386f, 8.68320117271867f, 8.69990544952401f, 8.71654030729723f, 8.73310846549834f, 8.74961258519297f, 8.76605527201844f, 8.78243907902203f, 8.7987665093797f, 8.81504001900293f, 8.83126201904121f, 8.84743487828681f, 8.86356092548834f, 8.87964245157913f, 8.8956817118262f, 8.91168092790504f, 8.92764228990538f, 8.94356795827277f, 8.95946006569037f, 8.97532071890552f, 8.9911520005049f, 9.00695597064244f, 9.02273466872357f, 9.03849011504952f, 9.05422431242489f, 9.06993924773203f, 9.08563689347523f, 9.1013192092979f, 9.11698814347564f, 9.13264563438814f, 9.14829361197268f, 9.16393399916203f, 9.17956871330943f, 9.1951996676033f, 9.21082877247429f, 9.22645793699725f, 9.24208907029076f, 9.25772408291671f, 9.27336488828254f, 9.28901340404866f, 9.30467155354376f, 9.32034126719051f, 9.33602448394432f, 9.35172315274794f, 9.36743923400462f, 9.38317470107257f, 9.39893154178373f, 9.41471175998981f, 9.43051737713856f, 9.44635043388364f, 9.46221299173125f, 9.47810713472702f, 9.49403497118674f, 9.50999863547468f, 9.5260002898334f, 9.54204212626923f, 9.55812636849767f, 9.57425527395335f, 9.59043113586931f, 9.60665628543076f, 9.62293309400858f, 9.63926397547845f, 9.65565138863155f, 9.67209783968326f, 9.68860588488691f, 9.70517813325966f, 9.72181724942849f, 9.73852595660465f, 9.75530703969537f, 9.77216334856257f, 9.78909780143873f, 9.80611338851095f, 9.82321317568514f, 9.84040030854287f, 9.85767801650483f, 9.87504961721559f, 9.89251852116555f, 9.91008823656748f, 9.92776237450618f, 9.94554465438154f, 9.96343890966679f, 9.98144909400584f, 9.99957928767527f, 10.0178337044393f, 10.036216698828f, 10.0547327738723f, 10.0733865893315f, 10.0921829704543f, 10.1111269173144f, 10.1302236147708f, 10.1494784431031f, 10.1688969893788f, 10.1884850596172f, 10.208248691817f, 10.2281941699247f, 10.2483280388279f, 10.2686571204663f, 10.2891885311634f, 10.3099297002921f, 10.3308883904023f, 10.3520727189493f, 10.3734911817819f, 10.395152678563f, 10.4170665403205f, 10.4392425593446f, 10.4616910216805f, 10.4844227424891f, 10.5074491045899f, 10.5307821005337f, 10.5544343786057f, 10.5784192932047f, 10.6027509601135f, 10.6274443172393f, 10.6525151914905f, 10.67798037255f, 10.7038576944166f, 10.7301661257206f, 10.756925869971f, 10.7841584770773f, 10.811886967703f, 10.8401359722661f, 10.8689318867084f, 10.8983030475226f, 10.928279928969f, 10.9588953659456f, 10.9901848066264f, 11.022186599771f, 11.0549423225786f, 11.0884971561543f, 11.1229003171347f, 11.1582055558673f, 11.1944717338517f, 11.2317634960773f, 11.2701520576111f, 11.3097161285568f, 11.3505430076665f, 11.3927298829085f, 11.4363853878479f, 11.4816314767016f, 11.5286056997271f, 11.5774639861078f, 11.6283840765351f, 11.6815697964567f, 11.7372564298417f, 11.7957175521278f, 11.8572738252678f, 11.9223044723995f, 11.9912624757751f, 12.0646950489125f, 12.1432717442911f, 12.2278238914625f, 12.3194013308372f, 12.4193564279281f, 12.529472800584f, 12.6521707539224f, 12.7908517893683f, 12.9505131204777f, 13.1389341808807f, 13.3692234553359f, 13.6661709812334f, 14.0862761567494f, 14.8161424382722f
};

Cooling_t cooling;
volatile uint16_t raw_dt_flow_ct;
volatile uint16_t raw_bat_flow_ct;
uint32_t last_flow_meas_time_ms;
uint32_t dt_pump_start_time_ms;
uint32_t bat_pump_start_time_ms;

extern q_handle_t q_tx_can;
extern uint32_t APB1ClockRateHz;


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

    RCC -> APB1ENR1 |= RCC_APB1ENR1_TIM4EN; //0b100;
    TIM4 -> CR1 &= ~TIM_CR1_CEN; //Turning off counter
    TIM4 -> PSC = (APB1ClockRateHz / (PWM_FREQUENCY * 100)) - 1; 
    TIM4 -> ARR = 100 - 1; //setting it to 99 so it's easier to use it with Duty Cycle
    //Enabling the MOE bit of the dead-time register
    // TIM4 -> BDTR |= TIM_BDTR_MOE;
    //Set Channels 1 and 2 to 110 (Mode 1 up counter) -> (active while CNT <= CCR)
    TIM4 -> CCMR1 &= ~(TIM_CCMR1_OC1M_0);
    TIM4 -> CCMR1 &= ~(TIM_CCMR1_OC2M_0);
    TIM4 -> CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1; 
    TIM4 -> CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1;
    //Setting the preload register
    TIM4 -> CCMR1 |= TIM_CCMR1_OC1PE | TIM_CCMR1_OC2PE;
    // TIM1 -> CR1 |= TIM_CR1_ARPE;
    //Enable Channels 1 and 2 outputs 
    TIM4 -> CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E;
    //Setting pwm to 0
    TIM4 -> CCR1 = 100; // inverted
    TIM4 -> CCR2 = 100;
    //Enable counter as long as ccrs are 0
    TIM4 -> CR1 |= TIM_CR1_CEN; // turning on counter

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

    // 568 594
    // Since ADC readings happen ~2ms, the next measurement should be ready
    // temp = rawThermtoCelcius(adc_readings.therm_mux_d);
    temp = THERM_A * adc_to_ln[(adc_readings.therm_mux_d / 16)] + THERM_B;
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

    int8_t drivetrain_right_temp = (DT_THERM_A * adc_to_ln[adc_readings.dt_gb_r/16] + DT_THERM_B);
    int8_t drivetrain_left_temp  = (DT_THERM_A * adc_to_ln[adc_readings.dt_gb_l/16] + DT_THERM_B);

    // Update outputs
    if (cooling.daq_override)
    {
        cooling.out = cooling.out_daq_req;
    }

    setDtFan(cooling.out.dt_fan_power);
    setBatFan(cooling.out.bat_fan_power);
    setBatPump(cooling.out.bat_pump, cooling.out.bat_pump_aux);
    setDtPump(cooling.out.dt_pump);

    SEND_FLOWRATE_TEMPS(q_tx_can, cooling.bat_therm_in_C, cooling.bat_therm_out_C,
                                  cooling.dt_therm_in_C,  cooling.dt_therm_out_C,
                                  cooling.bat_liters_p_min_x10, cooling.dt_liters_p_min_x10,
                                  0, 0);
    SEND_COOLANT_OUT(q_tx_can, cooling.out.bat_fan_power, cooling.out.dt_fan_power,
                               cooling.out.bat_pump, cooling.out.bat_pump_aux,
                               cooling.out.dt_pump);
    SEND_GEARBOX(q_tx_can, drivetrain_left_temp, drivetrain_right_temp);

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
    TIM4->CCR1 = 100 - CLAMP(power, 0, 100);
}

/**
 * @brief Set the DT Fan Speed
 * 
 * @param power 0 - 100%
 */
void setDtFan(uint8_t power)
{
    TIM4->CCR2 = 100 - CLAMP(power, 0, 100);
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

void cooling_driver_request_CALLBACK(CanParsedData_t* data)
{
    if (!cooling.daq_override)
    {
        cooling.out.dt_fan_power = data->cooling_driver_request.dt_fan;
        cooling.out.bat_fan_power = data->cooling_driver_request.batt_fan;
        cooling.out.bat_pump = data->cooling_driver_request.batt_pump;
        cooling.out.bat_pump_aux = data->cooling_driver_request.batt_pump2;
        cooling.out.dt_pump = data->cooling_driver_request.dt_pump;
    }
}