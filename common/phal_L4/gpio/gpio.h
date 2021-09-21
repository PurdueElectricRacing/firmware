/**
 * @file gpio.h
 * @author Adam Busch (busch8@purdue.edu)
 * @brief 
 * @version 0.1
 * @date 2021-03-20
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef _PHAL_GPIO_H_
#define _PHAL_GPIO_H_

#include "stm32l432xx.h"
#include <stdbool.h>

typedef enum {
    OUTPUT    = 0,
    INPUT     = 1,
    ALT_FUNC  = 2,
} GPIOOutputType_t;

typedef enum {
    LOW_SPEED       = 0b00,
    MED_SPEED       = 0b01,
    HIGH_SPEED      = 0b10,
    ULTRA_SPEED     = 0b11,
} GPIOOutputSpeed_t;

typedef enum {
    OPEN_DRAIN    = 0b00,
    PULL_UP       = 0b01,
    PULL_DOWN     = 0b10,
} GPIOPushPull_t;

typedef struct {
    GPIO_TypeDef*       bank; // GPIO Bank for configuration
    uint8_t             pin;  // Pin Number for configruation
    GPIOOutputType_t    type; // Output type of pin
    union 
    {
        //INPUT ONLY FIELDS
        GPIOPushPull_t      push_pull;
        // OUTPUT ONLY FIELDS
        GPIOOutputSpeed_t   ospeed;
        // AF ONLY FIELDS
        uint8_t             af_num; // Anternate function type
    } config;   // Type specific configuration for pins 
} GPIOConfig_t;

bool PHAL_initGPIO(GPIOConfig_t config[], uint8_t config_len);

#endif