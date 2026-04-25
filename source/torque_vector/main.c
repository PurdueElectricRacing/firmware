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
#include "common/ublox/nav_relposned.h"
#include "common/utils/linear_algebra.h"
#include "common/utils/max.h"

#include "decouple_imu.h"
#include "gps.h"

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

    // Rover GPS
    GPIO_INIT_USART3RX_PB11,
    GPIO_INIT_USART3TX_PB10,
    GPIO_INIT_OUTPUT(ROVER_RESET_PORT, ROVER_RESET_PIN, GPIO_OUTPUT_LOW_SPEED),

    // Base GPS
    GPIO_INIT_USART1TX_PA9,
    GPIO_INIT_USART1RX_PA10,
    GPIO_INIT_OUTPUT(BASE_RESET_PORT, BASE_RESET_PIN, GPIO_OUTPUT_LOW_SPEED),
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
static constexpr uint32_t GPS_BAUD_RATE = 460'800;
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
volatile uint8_t rover_gps_rx_buffer[NAV_PVT_TOTAL_LENGTH + NAV_RELPOSNED_TOTAL_LENGTH] = {0};
NAV_PVT_data_t nav_pvt = {0};
NAV_RELPOSNED_data_t nav_relposned = {0};

extern void HardFault_Handler(void);

// VCU Data
static pVCU_struct pVCU;
static xVCU_struct xVCU;
static yVCU_struct yVCU;
extern vector3_t gyro_data;
extern vector3_t accel_data;

void vcu_task() {
    // load up the xVCU (input) struct with most recent data
    xVCU.VCU_MODE_REQ = VCU_MODE_AUTOCROSS;

    xVCU.THROT_RAW = can_data.pedals.throttle / 4095.0f / 100.0f; // scale to [1,0]
    xVCU.BRAKE_RAW = can_data.pedals.brake / 4095.0f / 100.0f * -1.0f; // scale to [0,-1]

    xVCU.ST_RAW = can_data.steering_angle.angle * UNPACK_COEFF_STEERING_ANGLE_ANGLE;
    xVCU.VB_RAW = can_data.pack_stats.pack_voltage * UNPACK_COEFF_PACK_STATS_PACK_VOLTAGE;
    xVCU.WM_RAW[0] = can_data.wheel_speeds.front_left;
    xVCU.WM_RAW[1] = can_data.wheel_speeds.front_right;
    xVCU.WM_RAW[2] = can_data.wheel_speeds.rear_left;
    xVCU.WM_RAW[3] = can_data.wheel_speeds.rear_right;
    xVCU.GS_RAW = nav_pvt.groundSpeed * (1000.0f); // convert mm/s to m/s
    xVCU.AV_RAW[0] = gyro_data.x * UNPACK_COEFF_IMU_ANGULAR_RATE_X_AXIS;
    xVCU.AV_RAW[1] = gyro_data.y * UNPACK_COEFF_IMU_ANGULAR_RATE_Y_AXIS;
    xVCU.AV_RAW[2] = gyro_data.z * UNPACK_COEFF_IMU_ANGULAR_RATE_Z_AXIS;
    xVCU.IB_RAW = can_data.pack_stats.pack_current * UNPACK_COEFF_PACK_STATS_PACK_CURRENT;

    int16_t max_motor_temp = MAXOF(
        can_data.motor_temps.front_right,
        can_data.motor_temps.front_left,
        can_data.motor_temps.rear_left,
        can_data.motor_temps.rear_right
    );
    int16_t scaled_motor_temp = max_motor_temp * UNPACK_COEFF_MOTOR_TEMPS_FRONT_RIGHT;
    xVCU.MT_RAW = scaled_motor_temp;

    int16_t max_igbt_temp = MAXOF(
        can_data.igbt_temps.front_right,
        can_data.igbt_temps.front_left,
        can_data.igbt_temps.rear_left,
        can_data.igbt_temps.rear_right
    );
    int16_t scaled_igbt_temp = max_igbt_temp * UNPACK_COEFF_IGBT_TEMPS_FRONT_RIGHT;
    xVCU.IGBT_T_RAW = scaled_igbt_temp;

    xVCU.BT_RAW = 30.0f; // hardcoded to 30C

    xVCU.RG_split_FR_RAW = 0.3f; // todo driver configurable

    // step the VCU model
    vcu_step(&pVCU, &xVCU, &yVCU);

    // todo send yVCU -> torque requests
}

void gps_periodic() {
    NAV_PVT_decode(&nav_pvt, rover_gps_rx_buffer);
    NAV_RELPOSNED_decode(&nav_relposned, (rover_gps_rx_buffer + NAV_PVT_TOTAL_LENGTH));
}

void report_telemetry_10hz() {
    CAN_SEND_gps_coordinates(nav_pvt.latitude, nav_pvt.longitude);
    CAN_SEND_gps_velocity(nav_pvt.velNorth, nav_pvt.velEast);
    CAN_SEND_gps_speed(nav_pvt.groundSpeed, nav_pvt.headingVehicle);
}

void report_telemetry_1hz() {
    CAN_SEND_gps_time(
        (uint8_t)(nav_pvt.year - 2000),
        nav_pvt.month,
        nav_pvt.day,
        nav_pvt.hour,
        nav_pvt.minute,
        nav_pvt.second,
        0
    );
}

// Thread Defines
DEFINE_TASK(CAN_rx_update, 0, osPriorityHigh, STACK_2048);
DEFINE_TASK(CAN_tx_update, 2, osPriorityNormal, STACK_2048);
DEFINE_TASK(vcu_task, 10, osPriorityNormal, STACK_4096);
DEFINE_TASK(gps_periodic, 100, osPriorityLow, STACK_1024);
DEFINE_TASK(report_telemetry_10hz, 100, osPriorityLow, STACK_512);
DEFINE_TASK(report_telemetry_1hz, 1000, osPriorityLow, STACK_512);
DEFINE_HEARTBEAT_TASK(nullptr);

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
    PHAL_writeGPIO(BASE_RESET_PORT, BASE_RESET_PIN, 1);

    // TV initialization
    pVCU = init_pVCU();
    xVCU = init_xVCU();
    yVCU = init_yVCU();

    // Software Initialization
    osKernelInitialize();

    START_TASK(CAN_rx_update);
    START_TASK(CAN_tx_update);
    START_TASK(vcu_task);
    START_TASK(gps_periodic);
    START_TASK(report_telemetry_10hz);
    START_TASK(report_telemetry_1hz);
    START_HEARTBEAT_TASK();

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
