/**
 * @file main.c
 * @brief "Torque Vector" node source code
 *
 * @author Irving Wang (irvingw@purdue.edu)
 *
 */

#include "main.h"

#include <stdint.h>

#include "can_library/generated/TORQUE_VECTOR.h"
#include "common/freertos/freertos.h"
#include "common/phal/can.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"
#include "common/heartbeat/heartbeat.h"
#include "common/phal/usart.h"
#include "common/ublox/nav_pvt.h"

/* PER HAL Initialization Structures */
GPIOInitConfig_t gpio_config[] = {
    // Status LEDs
    GPIO_INIT_OUTPUT(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(ERROR_LED_PORT, ERROR_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(CONNECTION_LED_PORT, CONNECTION_LED_PIN, GPIO_OUTPUT_LOW_SPEED),

    // VCAN
    GPIO_INIT_FDCAN2TX_PB13, // we fly swapped TX/RX
    GPIO_INIT_FDCAN2RX_PB12,

    // GCAN
    // ! these pin are erroneously swapped on the schematic
    // GPIO_INIT_FDCAN1TX_PA12,
    // GPIO_INIT_FDCAN1RX_PA11,

    // Base GPS UART
    GPIO_INIT_USART1TX_PA9,
    GPIO_INIT_USART1RX_PA10,

    // Rover GPS UART
    GPIO_INIT_USART3RX_PB11,
    GPIO_INIT_USART3TX_PB10,
    GPIO_INIT_OUTPUT(ROVER_RESET_PORT, ROVER_RESET_PIN, GPIO_OUTPUT_LOW_SPEED)
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

// USART Configuration for GPS
static constexpr uint32_t GPS_BAUD_RATE = 115'200;
dma_init_t rover_tx_dma_config = USART3_TXDMA_CONT_CONFIG(NULL, 2);
dma_init_t rover_rx_dma_config = USART3_RXDMA_CONT_CONFIG(NULL, 1);
usart_init_t usart3 = {
    .baud_rate        = GPS_BAUD_RATE,
    .word_length      = WORD_8,
    .stop_bits        = SB_ONE,
    .parity           = PT_NONE,
    .hw_flow_ctl      = HW_DISABLE,
    .ovsample         = OV_16,
    .obsample         = OB_DISABLE,
    .periph           = USART3,
    .wake_addr        = false,
    .usart_active_num = USART3_ACTIVE_IDX,
    .tx_dma_cfg       = &rover_tx_dma_config,
    .rx_dma_cfg       = &rover_rx_dma_config,
};
volatile uint8_t rover_gps_rx_buffer[NAV_PVT_TOTAL_LENGTH] = {0}; // Buffer for GPS data reception
NAV_PVT_data_t nav_pvt = {0};

extern void HardFault_Handler(void);
extern void initialize_calibration(void);

void gps_periodic() {
    NAV_PVT_decode(&nav_pvt, (uint8_t *)rover_gps_rx_buffer);
}

// Thread Defines
DEFINE_TASK(CAN_rx_update, 1, osPriorityHigh, STACK_2048);
DEFINE_TASK(CAN_tx_update, 2, osPriorityNormal, STACK_2048);
DEFINE_TASK(gps_periodic, 100, osPriorityLow, STACK_1024);
DEFINE_HEARTBEAT_TASK(nullptr);

// VCU Data
static pVCU_struct pVCU;
static xVCU_struct xVCU;
static yVCU_struct yVCU;

int main(void) {
    // Hardware Initialization
    if (0 != PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }
    if (false == PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
        HardFault_Handler();
    }
    if (false == PHAL_initUSART(&usart3, APB1ClockRateHz)) {
        HardFault_Handler();
    }
    if (false == PHAL_usartRxDma(&usart3, (uint8_t *)rover_gps_rx_buffer, sizeof(rover_gps_rx_buffer), 1)) {
        HardFault_Handler();
    }
    if (false == PHAL_FDCAN_init(FDCAN2, false, VCAN_BAUD_RATE)) {
        HardFault_Handler();
    }

    CAN_library_init();
    initialize_calibration();
    NVIC_SetPriority(FDCAN2_IT0_IRQn, 6);
    NVIC_EnableIRQ(FDCAN2_IT0_IRQn);

    PHAL_writeGPIO(ROVER_RESET_PORT, ROVER_RESET_PIN, 1);

    // Software Initialization
    osKernelInitialize();

    START_TASK(CAN_rx_update);
    START_TASK(CAN_tx_update);
    START_TASK(gps_periodic);
    START_HEARTBEAT_TASK();

    // TV initialization (will break watchdog)
    pVCU = init_pVCU();
    xVCU = init_xVCU();
    yVCU = init_yVCU();

    vcu_step(&pVCU, &xVCU, &yVCU);

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
