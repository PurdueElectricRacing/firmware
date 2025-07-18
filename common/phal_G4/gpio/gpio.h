/**
 * @file gpio.h
 * @author Adam Busch (busch8@purdue.edu)
 * @brief GPIO Driver for STM32L432 Devices
 * @version 0.1
 * @date 2021-09-20
 *
 *
 * @copyright Copyright (c) 2021
 *
 *
 */
#ifndef __PHAL_G4_GPIO_H__
#define __PHAL_G4_GPIO_H__

#include "common/phal_G4/phal_G4.h"

/**
 * @brief Configuration type for GPIO Pin
 */
typedef enum
{
    GPIO_TYPE_INPUT = 0b00,  /* Pin input mode */
    GPIO_TYPE_OUTPUT = 0b01, /* Pin output mode */
    GPIO_TYPE_AF = 0b10,     /* Pin alternate function mode */
    GPIO_TYPE_ANALOG = 0b11, /* Pin alternate function mode */
} GPIOPinType_t;

/**
 * @brief Slew rate control for output pins
 */
typedef enum
{
    GPIO_OUTPUT_LOW_SPEED = 0b00,   /* Slew rate control, max 8Mhz */
    GPIO_OUTPUT_MED_SPEED = 0b01,   /* Slew rate control, max 50Mhz */
    GPIO_OUTPUT_HIGH_SPEED = 0b10,  /* Slew rate control, max 100Mhz */
    GPIO_OUTPUT_ULTRA_SPEED = 0b11, /* Slew rate control, max 180Mhz */
} GPIOOutputSpeed_t;

/**
 * @brief Output drive mode selection
 */
typedef enum
{
    GPIO_OUTPUT_PUSH_PULL = 0b0,  /* Drive the output pin high and low */
    GPIO_OUTPUT_OPEN_DRAIN = 0b1, /* Drive the output pin low, high-z otherwise */
} GPIOOutputPull_t;

/**
 * @brief Enable internal pullup/down resistors
 */
typedef enum
{
    GPIO_INPUT_OPEN_DRAIN = 0b00, /* No internal pull up/down */
    GPIO_INPUT_PULL_UP = 0b01,    /* Weak internal pull-up enabled */
    GPIO_INPUT_PULL_DOWN = 0b10,  /* Weak internal pull-down enabled */
} GPIOInputPull_t;

/**
 * @brief Configuration entry for GPIO initilization
 */
typedef struct
{
    GPIO_TypeDef *bank; /* GPIO Bank for configuration */
    uint8_t pin;        /* Pin Number for configruation */
    GPIOPinType_t type; /* Output type of pin */
    struct
    {
        // INPUT ONLY FIELDS
        GPIOInputPull_t pull; /* Push/Pull selection */

        // OUTPUT ONLY FIELDS
        GPIOOutputSpeed_t ospeed; /* Output speed (slew rate) */
        GPIOOutputPull_t otype;   /* Output push/pull */

        // AF ONLY FIELDS
        uint8_t af_num; /* Anternate function type */
    } config;           /* Type specific configuration for pins */
} GPIOInitConfig_t;

/**
 * @brief Create GPIO Init struct to intilize a GPIO pin for input
 *
 * @param gpio_bank GPIO_TypeDef* reference to the GPIO bank for the pin
 * @param pin_num Pin number from GPIO bank to configure
 * @param input_pull_sel Input pullup/pulldown/high-z selection
 */
#define GPIO_INIT_INPUT(gpio_bank, pin_num, input_pull_sel)                                             \
    {                                                                                                   \
        .bank = gpio_bank, .pin = pin_num, .type = GPIO_TYPE_INPUT, .config = {.pull = input_pull_sel } \
    }

/**
 * @brief Create GPIO Init struct to intilize a GPIO pin for output
 *
 * @param gpio_bank GPIO_TypeDef* reference to the GPIO bank for the pin
 * @param pin_num Pin number from GPIO bank to configure
 * @param ospeed_sel Pin output speed selection
 */
#define GPIO_INIT_OUTPUT(gpio_bank, pin_num, ospeed_sel)                                                         \
    {                                                                                                            \
        .bank = gpio_bank, .pin = pin_num, .type = GPIO_TYPE_OUTPUT, .config = {.ospeed = ospeed_sel,            \
                                                                                .otype = GPIO_OUTPUT_PUSH_PULL } \
    }

#define GPIO_INIT_OUTPUT_OPEN_DRAIN(gpio_bank, pin_num, ospeed_sel)                                               \
    {                                                                                                             \
        .bank = gpio_bank, .pin = pin_num, .type = GPIO_TYPE_OUTPUT, .config = {.ospeed = ospeed_sel,             \
                                                                                .otype = GPIO_OUTPUT_OPEN_DRAIN } \
    }
/**
 * @brief Create GPIO Init struct to intilize a GPIO pin for analog
 *
 * @param gpio_bank GPIO_TypeDef* reference to the GPIO bank for the pin
 * @param pin_num Pin number from GPIO bank to configure
 */
#define GPIO_INIT_ANALOG(gpio_bank, pin_num)                        \
    {                                                               \
        .bank = gpio_bank, .pin = pin_num, .type = GPIO_TYPE_ANALOG \
    }

/**
 * @brief Create GPIO Init struct to intilize a GPIO pin for alternate function
 *
 * @param gpio_bank GPIO_TypeDef* reference to the GPIO bank for the pin
 * @param pin_num Pin number from GPIO bank to configure
 * @param alt_func_num Alternate function selection
 * @param ospeed_sel Pin output speed selection
 * @param otype_sel Pin output type selection
 * @param input_pull_sel Input pullup/pulldown/high-z selection
 */
#define GPIO_INIT_AF(gpio_bank, pin_num, alt_func_num, ospeed_sel, otype_sel, input_pull_sel)        \
    {                                                                                                \
        .bank = gpio_bank, .pin = pin_num, .type = GPIO_TYPE_AF, .config = {.af_num = alt_func_num,  \
                                                                            .ospeed = ospeed_sel,    \
                                                                            .otype = otype_sel,      \
                                                                            .pull = input_pull_sel } \
    }

/*
    Useful defines for GPIO Init struct with commonly used peripheral/pin mappings.
    If you find yourself adding the same pin mappings to multiple devices, add a macro below
    to cut down on duplication.
*/

/**
 * @brief Initilize the GPIO perpheral given a list of configuration fields for all of the GPIO pins.
 *        Will also enable the GPIO RCC clock
 *
 * @param config A list of GPIOs to config
 * @param config_len Number of GPIOs in the config list
 * @return true All GPIOs were a valid configuration format
 * @return false Some of the GPIOs had an invalid configuration format
 */
bool PHAL_initGPIO(GPIOInitConfig_t config[], uint8_t config_len);

/**
 * @brief Read the state of the input register for the specific GPIO pin
 *
 * @param bank GPIO Bank of the pin
 * @param pin GPIO pin number
 * @return true GPIO Input true
 * @return false GPIO Input false
 */
static inline bool PHAL_readGPIO(GPIO_TypeDef *bank, uint8_t pin)
{
    return (bank->IDR >> pin) & 0b1;
}

/**
 * @brief Write a logic value to an output pin
 *
 * @param bank GPIO Bank of the pin
 * @param pin GPIO pin number
 * @param value Logical value to write
 */
static inline void PHAL_writeGPIO(GPIO_TypeDef *bank, uint8_t pin, bool value)
{
    bank->BSRR |= 1 << ((!value << 4) | pin); // BSRR has "set" as bottom 16 bits and "reset" as top 16
}

static inline void PHAL_toggleGPIO(GPIO_TypeDef *bank, uint8_t pin)
{
    PHAL_writeGPIO(bank, pin, !PHAL_readGPIO(bank, pin));
}

#endif // __PHAL_G4_GPIO_H__
