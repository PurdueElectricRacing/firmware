#ifndef PINDEFS_H
#define PINDEFS_H

/**
 * @file pindefs.h
 * @brief "Main Module" node pin definitions
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include <stdint.h>

#include "common/phal/gpio.h"

// Status LEDs
#define HEARTBEAT_LED_PORT  (GPIOA)
#define HEARTBEAT_LED_PIN   (0)
#define ERROR_LED_PORT      (GPIOA)
#define ERROR_LED_PIN       (5)
#define CONNECTION_LED_PORT (GPIOB)
#define CONNECTION_LED_PIN  (2)

// TSAL
#define TSAL_RED_CTRL_PORT   (GPIOC)
#define TSAL_RED_CTRL_PIN    (15)
#define TSAL_GREEN_CTRL_PORT (GPIOA)
#define TSAL_GREEN_CTRL_PIN  (3)
#define TSAL_RTM_ENABLE_PORT (GPIOB)
#define TSAL_RTM_ENABLE_PIN  (0)
#define TSAL_FAULT_PORT      (GPIOB)
#define TSAL_FAULT_PIN       (1)

// Brake and Buzzer
#define BRAKE_LIGHT_PORT (GPIOA)
#define BRAKE_LIGHT_PIN  (1)
#define BUZZER_PORT      (GPIOA)
#define BUZZER_PIN       (2)

// SDC
#define ECU_SDC_CTRL_PORT (GPIOC)
#define ECU_SDC_CTRL_PIN  (14)
#define SDC_MUX_PORT      (GPIOB)
#define SDC_MUX_PIN       (9)
#define SDC_MUX_S3_PORT   (GPIOB)
#define SDC_MUX_S3_PIN    (7)
#define SDC_MUX_S2_PORT   (GPIOB)
#define SDC_MUX_S2_PIN    (6)
#define SDC_MUX_S1_PORT   (GPIOB)
#define SDC_MUX_S1_PIN    (5)
#define SDC_MUX_S0_PORT   (GPIOB)
#define SDC_MUX_S0_PIN    (4)

// Input status pins
#define BMS_STATUS_PORT         (GPIOC)
#define BMS_STATUS_PIN          (13)
#define VBATT_ECU_PORT          (GPIOA)
#define VBATT_ECU_PIN           (4)
#define VMC_ECU_PORT            (GPIOA)
#define VMC_ECU_PIN             (6)
#define PRECHARGE_COMPLETE_PORT (GPIOB)
#define PRECHARGE_COMPLETE_PIN  (10)

#endif // PINDEFS_H
