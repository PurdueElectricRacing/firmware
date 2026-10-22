#ifndef MAIN_H
#define MAIN_H

/**
 * @file main.h
 * @brief "Main Module" node source code
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "common/phal_G4/gpio/gpio.h"

// Status LEDs
#define HEARTBEAT_LED_PORT  (GPIOC)
#define HEARTBEAT_LED_PIN   (7)
#define ERROR_LED_PORT      (GPIOC)
#define ERROR_LED_PIN       (9)
#define CONNECTION_LED_PORT (GPIOC)
#define CONNECTION_LED_PIN  (8)

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
#define BRAKE_LIGHT_PORT (GPIOB)
#define BRAKE_LIGHT_PIN  (0)
#define BUZZER_PORT      (GPIOB)
#define BUZZER_PIN       (1)

// SDC
#define ECU_SDC_CTRL_PORT (GPIOA)
#define ECU_SDC_CTRL_PIN  (5)
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
#define BMS_STATUS_PORT             (GPIOA)
#define BMS_STATUS_PIN              (7)
#define IMD_STATUS_PORT             (GPIOA)
#define IMD_STATUS_PIN              (6)
#define VBATT_ECU_PORT              (GPIOA)
#define VBATT_ECU_PIN               (4)
#define VMC_ECU_PORT                (GPIOA)
#define VMC_ECU_PIN                 (3)
#define NOT_PRECHARGE_COMPLETE_PORT (GPIOC)
#define NOT_PRECHARGE_COMPLETE_PIN  (4)

#endif // MAIN_H
