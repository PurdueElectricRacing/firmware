/**
 * @file main.c
 * @brief "Main Module" node source code
 * 
 * @author Irving Wang (irvingw@purdue.edu)
 */

#include "main.h"
#include "pindefs.h"

#include "common/common_defs/common_defs.h"
#include "common/can_library/generated/MAIN_MODULE.h"
#include "common/freertos/freertos.h"
#include "common/phal/can.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"
#include "common/amk/amk.h"


// Global data structures
car_t g_car;
torque_request_t g_torque_request;
SDC_state_t g_SDC_state;

/* PER HAL Initialization Structures */
GPIOInitConfig_t gpio_config[] = {
    // Status LEDs
    GPIO_INIT_OUTPUT(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(ERROR_LED_PORT, ERROR_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(CONNECTION_LED_PORT, CONNECTION_LED_PIN, GPIO_OUTPUT_LOW_SPEED),

    // TSAL
    GPIO_INIT_OUTPUT(TSAL_RED_CTRL_PORT, TSAL_RED_CTRL_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(TSAL_GREEN_CTRL_PORT, TSAL_GREEN_CTRL_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(TSAL_RTM_ENABLE_PORT, TSAL_RTM_ENABLE_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_INPUT(TSAL_FAULT_PORT, TSAL_FAULT_PIN, GPIO_INPUT_PULL_DOWN),

    // Brake and Buzzer
    GPIO_INIT_OUTPUT(BRAKE_LIGHT_PORT, BRAKE_LIGHT_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(BUZZER_PORT, BUZZER_PIN, GPIO_OUTPUT_LOW_SPEED),

    // SDC
    GPIO_INIT_OUTPUT(ECU_SDC_CTRL_PORT, ECU_SDC_CTRL_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_INPUT(SDC_MUX_PORT, SDC_MUX_PIN, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_OUTPUT(SDC_MUX_S3_PORT, SDC_MUX_S3_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(SDC_MUX_S2_PORT, SDC_MUX_S2_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(SDC_MUX_S1_PORT, SDC_MUX_S1_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(SDC_MUX_S0_PORT, SDC_MUX_S0_PIN, GPIO_OUTPUT_LOW_SPEED),

    // Input status pins
    GPIO_INIT_INPUT(BMS_STATUS_PORT, BMS_STATUS_PIN, GPIO_INPUT_PULL_DOWN),
    GPIO_INIT_INPUT(VBATT_ECU_PORT, VBATT_ECU_PIN, GPIO_INPUT_PULL_DOWN),
    GPIO_INIT_INPUT(VMC_ECU_PORT, VMC_ECU_PIN, GPIO_INPUT_PULL_DOWN),
    GPIO_INIT_INPUT(PRECHARGE_COMPLETE_PORT, PRECHARGE_COMPLETE_PIN, GPIO_INPUT_PULL_DOWN),

    // VCAN
    GPIO_INIT_FDCAN2TX_PB13,
    GPIO_INIT_FDCAN2RX_PB12,

    // MCAN
    GPIO_INIT_FDCAN3TX_PA15,
    GPIO_INIT_FDCAN3RX_PA8
};

static constexpr uint32_t TargetCoreClockrateHz = 16'000'000;
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

extern void HardFault_Handler(void);

void heartbeat_task() {
    // preflight animation for the first 1.5 seconds after boot
    if (OS_TICKS <= PREFLIGHT_DURATION_MS) {
        static uint32_t sweep_index = 0;

        // Creates a sweeping pattern
        switch (sweep_index++ % 3) {
            case 0:
                PHAL_writeGPIO(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, 1);
                PHAL_writeGPIO(CONNECTION_LED_PORT, CONNECTION_LED_PIN, 0);
                PHAL_writeGPIO(ERROR_LED_PORT, ERROR_LED_PIN, 0);
                break;
            case 1:
                PHAL_writeGPIO(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, 0);
                PHAL_writeGPIO(CONNECTION_LED_PORT, CONNECTION_LED_PIN, 1);
                PHAL_writeGPIO(ERROR_LED_PORT, ERROR_LED_PIN, 0);
                break;
            case 2:
                PHAL_writeGPIO(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, 0);
                PHAL_writeGPIO(CONNECTION_LED_PORT, CONNECTION_LED_PIN, 0);
                PHAL_writeGPIO(ERROR_LED_PORT, ERROR_LED_PIN, 1);
                break;
        }

        return;
    }

    PHAL_toggleGPIO(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN);

    if (OS_TICKS - last_can_rx_time_ms >= CONN_LED_TIMEOUT_MS) {
        PHAL_writeGPIO(CONNECTION_LED_PORT, CONNECTION_LED_PIN, 1);
    } else {
        PHAL_writeGPIO(CONNECTION_LED_PORT, CONNECTION_LED_PIN, 0);
    }
}

void can_worker_task() {
    CAN_rx_update();
    CAN_tx_update();
}

void poll_input_pins() {
    // check SDC state by cycling through the mux and checking the input
    static uint8_t sdc_poll_index = 0;
    PHAL_writeGPIO(SDC_MUX_S0_PORT, SDC_MUX_S0_PIN, (sdc_poll_index >> 0) & 0x1);
    PHAL_writeGPIO(SDC_MUX_S1_PORT, SDC_MUX_S1_PIN, (sdc_poll_index >> 1) & 0x1);
    PHAL_writeGPIO(SDC_MUX_S2_PORT, SDC_MUX_S2_PIN, (sdc_poll_index >> 2) & 0x1);
    PHAL_writeGPIO(SDC_MUX_S3_PORT, SDC_MUX_S3_PIN, (sdc_poll_index >> 3) & 0x1);
    
    osDelay(1); // delay to allow mux signals to stabilize
    
    bool node_status = PHAL_readGPIO(SDC_MUX_PORT, SDC_MUX_PIN);
    g_SDC_state &= ~(1 << sdc_poll_index); // clear the bit for the current node
    g_SDC_state |= (node_status << sdc_poll_index);

    // update the poll index for the next cycle (0-15)
    sdc_poll_index = (sdc_poll_index + 1) & 0xF;

    g_car.is_precharge_complete = PHAL_readGPIO(PRECHARGE_COMPLETE_PORT, PRECHARGE_COMPLETE_PIN);
}

void AMK_task() {
    AMK_periodic(&g_car.front_right);
    AMK_periodic(&g_car.front_left);
    AMK_periodic(&g_car.rear_left);
    AMK_periodic(&g_car.rear_right);
}

defineThreadStack(heartbeat_task, HEARTBEAT_PERIOD_MS, osPriorityLow, 256);
defineThreadStack(poll_input_pins, 0, osPriorityIdle, 256); // the delay is within the thread
defineThreadStack(can_worker_task, 10, osPriorityHigh, 1024);
defineThreadStack(fsm_periodic, 15, osPriorityNormal, 2048);
defineThreadStack(AMK_task, 15, osPriorityNormal, 1024);

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

    car_init();

    // Software Initialization
    osKernelInitialize();

    createThread(heartbeat_task);
    createThread(can_worker_task);
    createThread(poll_input_pins);
    createThread(fsm_periodic);
    createThread(AMK_task);

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
