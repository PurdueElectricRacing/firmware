/**
 * @file main.c
 * @brief "Main Module" node source code
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 * 
 */

#include "main.h"

#include <stdint.h>

#include "common/amk/amk.h"
#include "common/can_library/generated/MAIN_MODULE.h"
#include "common/freertos/freertos.h"
#include "common/phal/can.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"

/* PER HAL Initialization Structures */
GPIOInitConfig_t gpio_config[] = {
    // Status LEDs
    GPIO_INIT_OUTPUT(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(ERROR_LED_PORT, ERROR_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(CONNECTION_LED_PORT, CONNECTION_LED_PIN, GPIO_OUTPUT_LOW_SPEED),

    // VCAN
    GPIO_INIT_FDCAN2TX_PB13,
    GPIO_INIT_FDCAN2RX_PB12,
    // MCAN
    GPIO_INIT_FDCAN3TX_PA15,
    GPIO_INIT_FDCAN3RX_PA8
};

static constexpr uint32_t TargetCoreClockrateHz = 16000000;
ClockRateConfig_t clock_config = {
    .clock_source           = CLOCK_SOURCE_HSE,
    .use_pll                = false,
    .system_clock_target_hz = TargetCoreClockrateHz,
    .ahb_clock_target_hz    = (TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz   = (TargetCoreClockrateHz / (1)),
    .apb2_clock_target_hz   = (TargetCoreClockrateHz / (1)),
};

/* Locals for Clock Rates */
extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

AMK_t test_amk;
// ! bypass precharge for bench testing
bool is_precharge_complete = true;

/* Wrappers to map generic AMK calls to Inverter A CAN messages */
void inva_set_flush(void) {
    CAN_SEND_INVA_SET(test_amk.set->AMK_Control_bReserve,
                      test_amk.set->AMK_Control_bInverterOn,
                      test_amk.set->AMK_Control_bDcOn,
                      test_amk.set->AMK_Control_bEnable,
                      test_amk.set->AMK_Control_bErrorReset,
                      test_amk.set->AMK_Control_bReserve2,
                      test_amk.set->AMK_TorqueSetpoint,
                      test_amk.set->AMK_PositiveTorqueLimit,
                      test_amk.set->AMK_NegativeTorqueLimit);
}

void inva_log_flush(void) {
    CAN_SEND_INVA_LOG_SET(test_amk.set->AMK_Control_bReserve,
                          test_amk.set->AMK_Control_bInverterOn,
                          test_amk.set->AMK_Control_bDcOn,
                          test_amk.set->AMK_Control_bEnable,
                          test_amk.set->AMK_Control_bErrorReset,
                          test_amk.set->AMK_Control_bReserve2,
                          test_amk.set->AMK_TorqueSetpoint,
                          test_amk.set->AMK_PositiveTorqueLimit,
                          test_amk.set->AMK_NegativeTorqueLimit);
}

extern void HardFault_Handler(void);

void ledblink() {
    PHAL_toggleGPIO(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN);
}

void amk_test_thread() {
    CAN_rx_update();
    // todo more amk test stuff here
    AMK_periodic(&test_amk);

    if (test_amk.state == AMK_STATE_RUNNING) {
        // ! test 1, constant torque
         AMK_set_torque(&test_amk, 5);
    } else {
        // AMK_set_torque(&test_amk, 0);
    }

    CAN_tx_update();
}

defineThreadStack(ledblink, 500, osPriorityLow, 256);
defineThreadStack(amk_test_thread, 15, osPriorityNormal, 2048);

int main(void) {
    // Hardware Initialization
    if (0 != PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }
    if (false == PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
        HardFault_Handler();
    }

    if (false == PHAL_FDCAN_init(FDCAN2, false, VCAN_BAUD_RATE)) {
        HardFault_Handler();
    }
    if (false == PHAL_FDCAN_init(FDCAN3, false, MCAN_BAUD_RATE)) {
        HardFault_Handler();
    }

    NVIC_EnableIRQ(FDCAN2_IT0_IRQn);
    NVIC_EnableIRQ(FDCAN3_IT0_IRQn);
    CAN_library_init();

    // Bench Test Initialization
    is_precharge_complete = true;
    AMK_init(&test_amk,
             inva_set_flush,
             inva_log_flush,
             &can_data.INVA_SET,
             &can_data.INVA_CRIT,
             &can_data.INVA_INFO,
             &can_data.INVA_TEMPS,
             &can_data.INVA_ERR_1,
             &can_data.INVA_ERR_2,
             &is_precharge_complete);

    // ! test 1, constant torque
    // AMK_set_torque(&test_amk, 5); // Request 5% torque for bench test

    // Software Initialization
    osKernelInitialize();

    createThread(ledblink);
    // ! test 2, state machine
    createThread(amk_test_thread);

    // no way home
    osKernelStart();

    return 0;
}

// todo reboot on hardfault
void HardFault_Handler() {
    __disable_irq();
    SysTick->CTRL        = 0;
    ERROR_LED_PORT->BSRR = (1 << ERROR_LED_PIN);
    while (1) {
        __asm__("NOP"); // Halt forever
    }
}
