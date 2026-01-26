#include "cooling.h"
#include "common/can_library/generated/MAIN_MODULE.h"

#include "common_defs.h"

Cooling_t cooling;

bool coolingInit() {
    cooling = (Cooling_t) {0};

    // Select Thermistor 0 on mux to begin checking thermistors
    PHAL_writeGPIO(THERM_MUX_S0_GPIO_Port, THERM_MUX_S0_Pin, 0);
    PHAL_writeGPIO(THERM_MUX_S1_GPIO_Port, THERM_MUX_S1_Pin, 0);
    PHAL_writeGPIO(THERM_MUX_S2_GPIO_Port, THERM_MUX_S2_Pin, 0);

    return true;
}

void coolingPeriodic() {
    /* WATER TEMP CALCULATIONS */
    static uint8_t curr_therm;
    int8_t temp;

    // TODO: test signed temps

    // 568 594
    // Since ADC readin/gs happen ~2ms, the next measurement should be ready
    // Send temps, passing in different thermistor constants for the different thermistors
    if (curr_therm < COOL_LOOP_START_IDX) {
        // Get temp
        temp = rawThermtoCelcius(adc_readings.therm_mux_d, DT_THERM_A, DT_THERM_B, DT_THERM_R1);
    } else {
        // Get temp
        if (curr_therm == B_THERM_IDX)
            temp = rawThermtoCelcius(adc_readings.therm_mux_d, W_THERM_A, W_THERM_B, B_THERM_R1);
        else
            temp = rawThermtoCelcius(adc_readings.therm_mux_d, W_THERM_A, W_THERM_B, W_THERM_R1);
    }
    // Because the select lines are in order, we can just update coolant struct by treating it as an array
    *((int8_t*)(&cooling) + curr_therm) = temp;

    // Update current thermistor
    curr_therm = (curr_therm == THERM_MUX_END_IDX) ? 0 : (curr_therm + 1);

    // Set mux for next value
    PHAL_writeGPIO(THERM_MUX_S0_GPIO_Port, THERM_MUX_S0_Pin, (curr_therm & 0x01));
    PHAL_writeGPIO(THERM_MUX_S1_GPIO_Port, THERM_MUX_S1_Pin, (curr_therm & 0x02));
    PHAL_writeGPIO(THERM_MUX_S2_GPIO_Port, THERM_MUX_S2_Pin, (curr_therm & 0x04));

    // Only send data once all new thermistors are updated to avoid clutter on CAN bus
    if (curr_therm == 0) {
        CAN_SEND_coolant_temps(cooling.bat_therm_in_C, cooling.bat_therm_out_C, cooling.dt_therm_in_C, cooling.dt_therm_out_C);

        CAN_SEND_gearbox(cooling.gb_therm_l_c, cooling.gb_therm_r_c);
    }

    //TODO: After mc parse library, update these with actual faults
    // Find max motor temperature (CELSIUS)
    // uint8_t max_motor_temp = MAX(car.motor_l.motor_temp,
    //                              car.motor_r.motor_temp);
    // Motor Temperature monitor
    // set_fault(FAULT_INDEX_MAIN_MODULE_MOTOR_L_HEAT, car.motor_l.motor_temp);
    // set_fault(FAULT_INDEX_MAIN_MODULE_MOTOR_R_HEAT, car.motor_r.motor_temp);

    // set_fault(FAULT_INDEX_MAIN_MODULE_MOTOR_L_OT, car.motor_l.motor_temp);
    // set_fault(FAULT_INDEX_MAIN_MODULE_MOTOR_R_OT, car.motor_r.motor_temp);

    // Determine if dt/coolant temps are too high
    // DT
    update_fault(FAULT_INDEX_MAIN_MODULE_DT_L_TEMP_HIGH, cooling.gb_therm_l_c);
    update_fault(FAULT_INDEX_MAIN_MODULE_DT_L_TEMP_OT, cooling.gb_therm_l_c);
    update_fault(FAULT_INDEX_MAIN_MODULE_DT_R_TEMP_HIGH, cooling.gb_therm_r_c);
    update_fault(FAULT_INDEX_MAIN_MODULE_DT_R_TEMP_OT, cooling.gb_therm_r_c);

    // Cooling Loop
    // set_fault(FAULT_INDEX_MAIN_MODULE_BAT_COOL_LOOP_HIGH, MAX(cooling.bat_therm_in_C, cooling.bat_therm_out_C));
    update_fault(FAULT_INDEX_MAIN_MODULE_DT_COOL_LOOP_HIGH, MAX(cooling.dt_therm_in_C, cooling.dt_therm_out_C));

    // Disconnect Faults
    // DT
    update_fault(FAULT_INDEX_MAIN_MODULE_DT_L_THERM_DISC, cooling.gb_therm_l_c);
    update_fault(FAULT_INDEX_MAIN_MODULE_DT_R_THERM_DISC, cooling.gb_therm_r_c);

    // Cooling Loop
    // set_fault(FAULT_INDEX_MAIN_MODULE_BATT_CL_DISC, MIN(cooling.bat_therm_in_C, cooling.bat_therm_out_C));
    update_fault(FAULT_INDEX_MAIN_MODULE_DT_CL_DISC, MIN(cooling.dt_therm_in_C, cooling.dt_therm_out_C));
}

float rawThermtoCelcius(uint16_t t, float a, float b, uint16_t r1) {
    float f;
    if (t == MAX_THERM)
        return -290;
    f = t * ADC_REF_fp / MAX_THERM; // Signal voltage
    f = (f * r1) / (ADC_REF_fp - f); // Resistance
    return a * native_log_computation(f) + b;
}

// https://stackoverflow.com/questions/9800636/calculating-natural-logarithm-and-exponent-by-core-c-for-embedded-system

static double native_log_computation(const double n) {
    // Basic logarithm computation.
    static const double euler = 2.7182818284590452354;
    unsigned a                = 0, d;
    double b, c, e, f;
    if (n > 0) {
        for (c = n < 1 ? 1 / n : n; (c /= euler) > 1; ++a)
            ;
        c = 1 / (c * euler - 1), c = c + c + 1, f = c * c, b = 0;
        for (d = 1, c /= 2; e = b, b += 1 / (d * c), b - e /* > 0.0000001 */;)
            d += 2, c *= f;
    } else
        b = (n == 0) / 0.;
    return n < 1 ? -(a + b) : a + b;
}
