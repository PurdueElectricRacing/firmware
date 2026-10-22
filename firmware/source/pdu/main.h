#ifndef MAIN_H
#define MAIN_H

/**
 * @file main.h
 * @brief "PDU" node source code
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "common/phal_G4/gpio/gpio.h"

// Status indicators
#define HEARTBEAT_LED_PORT  (GPIOC)
#define HEARTBEAT_LED_PIN   (15)
#define ERROR_LED_PORT      (GPIOC)
#define ERROR_LED_PIN       (13)
#define CONNECTION_LED_PORT (GPIOC)
#define CONNECTION_LED_PIN  (14)

// Power and load controls
#define DAQ_CTRL_PORT       (GPIOC)
#define DAQ_CTRL_PIN        (0)
#define SDC_CTRL_PORT       (GPIOC)
#define SDC_CTRL_PIN        (1)
#define FAN1_CTRL_PORT      (GPIOC)
#define FAN1_CTRL_PIN       (2)
#define MAIN_CTRL_PORT      (GPIOC)
#define MAIN_CTRL_PIN       (3)
#define SDC_LOOP_CTRL_PORT  (GPIOC)
#define SDC_LOOP_CTRL_PIN   (5)
#define TV_CTRL_PORT        (GPIOB)
#define TV_CTRL_PIN         (10)
#define ABOX_CTRL_PORT      (GPIOD)
#define ABOX_CTRL_PIN       (2)
#define DASH_CTRL_PORT      (GPIOC)
#define DASH_CTRL_PIN       (12)
#define DL_REAR_CTRL_PORT   (GPIOC)
#define DL_REAR_CTRL_PIN    (11)
#define DL_FRONT_CTRL_PORT  (GPIOC)
#define DL_FRONT_CTRL_PIN   (10)
#define DISCHARGE_CTRL_PORT (GPIOA)
#define DISCHARGE_CTRL_PIN  (15)
#define AMK1_CTRL_PORT      (GPIOA)
#define AMK1_CTRL_PIN       (10)
#define AMK2_CTRL_PORT      (GPIOA)
#define AMK2_CTRL_PIN       (9)
#define FAN2_CTRL_PORT      (GPIOA)
#define FAN2_CTRL_PIN       (8)
#define FAN3_CTRL_PORT      (GPIOC)
#define FAN3_CTRL_PIN       (9)
#define FAN4_CTRL_PORT      (GPIOC)
#define FAN4_CTRL_PIN       (8)
#define FAN5_CTRL_PORT      (GPIOC)
#define FAN5_CTRL_PIN       (7)
#define HXFAN_CTRL_PORT     (GPIOC)
#define HXFAN_CTRL_PIN      (6)
#define PUMP1_CTRL_PORT     (GPIOB)
#define PUMP1_CTRL_PIN      (15)
#define PUMP2_CTRL_PORT     (GPIOB)
#define PUMP2_CTRL_PIN      (14)
#define EMETER_CTRL_PORT    (GPIOB)
#define EMETER_CTRL_PIN     (11)

// Analog inputs
#define RAIL_24V_VS_PORT (GPIOA)
#define RAIL_24V_VS_PIN  (3)
#define RAIL_24V_CS_PORT (GPIOB)
#define RAIL_24V_CS_PIN  (0)
#define MUX1_OUT_PORT    (GPIOC)
#define MUX1_OUT_PIN     (4)
#define MUX2_OUT_PORT    (GPIOA)
#define MUX2_OUT_PIN     (4)

// Analog multiplexer controls
#define MUX1_CTRL_A_PORT (GPIOA)
#define MUX1_CTRL_A_PIN  (5)
#define MUX1_CTRL_B_PORT (GPIOA)
#define MUX1_CTRL_B_PIN  (6)
#define MUX1_CTRL_C_PORT (GPIOA)
#define MUX1_CTRL_C_PIN  (7)
#define MUX2_CTRL_A_PORT (GPIOB)
#define MUX2_CTRL_A_PIN  (5)
#define MUX2_CTRL_B_PORT (GPIOB)
#define MUX2_CTRL_B_PIN  (4)
#define MUX2_CTRL_C_PORT (GPIOB)
#define MUX2_CTRL_C_PIN  (1)
#define MUX2_CTRL_D_PORT (GPIOB)
#define MUX2_CTRL_D_PIN  (2)

#endif // MAIN_H
