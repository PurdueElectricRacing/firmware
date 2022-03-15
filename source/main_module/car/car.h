/**
 * @file car.h
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief  Master Car Control and Safety Checking
 * @version 0.1
 * @date 2022-03-01
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef _CAR_H_
#define _CAR_H_

#include <stdbool.h>
#include "common/phal_L4/gpio/gpio.h"
#include "common/psched/psched.h"
#include "main.h"

#include "can_parse.h"

#define BRAKE_PRESSED_THRESHOLD 0.30f
#define BUZZER_DURATION_MS 2000 // EV.10.5: 1-3s

typedef struct __attribute__((packed))
{
    // Do not modify this struct unless
    // you modify the ADC DMA config
    // in main.h to match
    uint16_t dt_therm_1;
    uint16_t dt_therm_2;
    uint16_t bat_therm_out;
    uint16_t bat_therm_in;
    uint16_t i_sense_c1;
    uint16_t lv_i_sense;
} ADCReadings_t;

volatile extern ADCReadings_t adc_readings;

typedef struct
{
    car_state_t state;
    bool brake_light;
    uint16_t max_cell_temp;
} Car_t;

volatile extern Car_t car;

#endif