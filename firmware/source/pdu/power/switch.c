#include "common/phal_G4/gpio/gpio.h"
#include "main.h"

typedef enum {
    SWITCH_SDC = 0,
    SWITCH_SDC_LOOP,
    SWITCH_DISCHARGE,
    SWITCH_TV,
    SWITCH_EMETER,
    SWITCH_ABOX,
    SWITCH_DASH,
    SWITCH_DL_REAR,
    SWITCH_DL_FRONT,
    SWITCH_MAIN,
    SWITCH_DAQ,
    SWITCH_HXFAN,
    SWITCH_FAN5,
    SWITCH_FAN4,
    SWITCH_FAN3,
    SWITCH_FAN2,
    SWITCH_FAN1,
    SWITCH_AMK1,
    SWITCH_AMK2,
    SWITCH_PUMP1,
    SWITCH_PUMP2,
    NUM_SWITCHES
} switch_id_t;

typedef struct {
    GPIO_TypeDef* ctrl_port;
    uint8_t ctrl_pin;
    // todo analog mux address
} switch_t;

switch_t switches[NUM_SWITCHES] = {
    [SWITCH_SDC]       = {.ctrl_port = CTRL1_PORT, .ctrl_pin = CTRL1_PIN},
    [SWITCH_SDC_LOOP]  = {.ctrl_port = CTRL2_PORT, .ctrl_pin = CTRL2_PIN},
    [SWITCH_DISCHARGE] = {.ctrl_port = CTRL3_PORT, .ctrl_pin = CTRL3_PIN},
    [SWITCH_TV]        = {.ctrl_port = CTRL4_PORT, .ctrl_pin = CTRL4_PIN},
    [SWITCH_EMETER]    = {.ctrl_port = CTRL5_PORT, .ctrl_pin = CTRL5_PIN},
    [SWITCH_ABOX]      = {.ctrl_port = CTRL6_PORT, .ctrl_pin = CTRL6_PIN},
    [SWITCH_DASH]      = {.ctrl_port = CTRL7_PORT, .ctrl_pin = CTRL7_PIN},
    [SWITCH_DL_REAR]   = {.ctrl_port = CTRL8_PORT, .ctrl_pin = CTRL8_PIN},
    [SWITCH_DL_FRONT]  = {.ctrl_port = CTRL9_PORT, .ctrl_pin = CTRL9_PIN},
    [SWITCH_MAIN]      = {.ctrl_port = CTRL10_PORT, .ctrl_pin = CTRL10_PIN},
    [SWITCH_DAQ]       = {.ctrl_port = CTRL11_PORT, .ctrl_pin = CTRL11_PIN},
    [SWITCH_HXFAN]     = {.ctrl_port = CTRL12_PORT, .ctrl_pin = CTRL12_PIN},
    [SWITCH_FAN5]      = {.ctrl_port = CTRL13_PORT, .ctrl_pin = CTRL13_PIN},
    [SWITCH_FAN4]      = {.ctrl_port = CTRL14_PORT, .ctrl_pin = CTRL14_PIN},
    [SWITCH_FAN3]      = {.ctrl_port = CTRL15_PORT, .ctrl_pin = CTRL15_PIN},
    [SWITCH_FAN2]      = {.ctrl_port = CTRL16_PORT, .ctrl_pin = CTRL16_PIN},
    [SWITCH_FAN1]      = {.ctrl_port = CTRL17_PORT, .ctrl_pin = CTRL17_PIN},
    [SWITCH_AMK1]      = {.ctrl_port = CTRL18_PORT, .ctrl_pin = CTRL18_PIN},
    [SWITCH_AMK2]      = {.ctrl_port = CTRL19_PORT, .ctrl_pin = CTRL19_PIN},
    [SWITCH_PUMP1]     = {.ctrl_port = CTRL20_PORT, .ctrl_pin = CTRL20_PIN},
    [SWITCH_PUMP2]     = {.ctrl_port = CTRL21_PORT, .ctrl_pin = CTRL21_PIN},
};
