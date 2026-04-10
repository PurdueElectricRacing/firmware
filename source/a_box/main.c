/**
 * @file main.c
 * @brief "Abox" node source code
 *
 * @author Irving Wang (irvingw@purdue.edu), Millan Kumar (kumar798@purdue.edu)
 */

#include "main.h"

#include <math.h>
#include <stdint.h>

#include "adbms.h"
#include "common/can_library/faults_common.h"
#include "common/can_library/generated/A_BOX.h"
#include "common/freertos/freertos.h"
#include "common/phal/can.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"
#include "common/phal/adc.h"
#include "common/heartbeat/heartbeat.h"

SPI_InitConfig_t bms_spi_config = {
    .data_len      = 8,
    .nss_sw        = false, // BMS drive CS pin manually to ensure correct timing
    .nss_gpio_port = SPI1_CS_PORT,
    .nss_gpio_pin  = SPI1_CS_PIN,
    .rx_dma_cfg    = nullptr,
    .tx_dma_cfg    = nullptr,
    .periph        = SPI1,
    .cpol          = 0,
    .cpha          = 0,
    .data_rate     = 500'000, // 500 kHz SPI clock for ADBMS6380
};

ADCInitConfig_t adc_config = {
    .prescaler      = ADC_CLK_PRESC_2,
    .resolution     = ADC_RES_12_BIT,
    .data_align     = ADC_DATA_ALIGN_RIGHT,
    .cont_conv_mode = true,
    .dma_mode       = ADC_DMA_CIRCULAR,
    .periph         = ADC1,
};

typedef struct {
    uint16_t isense_raw;
    uint16_t vbatt_raw;
} adc1_dma_buffer_t;
volatile adc1_dma_buffer_t adc1_dma_buffer;
ADCChannelConfig_t adc_channel_config[] = {
    {.channel = ISENSE_ADC_CHANNEL, .rank = 1, .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = VBATT_ADC_CHANNEL, .rank = 2, .sampling_time = ADC_CHN_SMP_CYCLES_480}
};
dma_init_t adc_dma_config = ADC1_DMA_CONT_CONFIG(
    (uint32_t)&adc1_dma_buffer,
    sizeof(adc1_dma_buffer) / sizeof(uint16_t), 0b01
);

/* PER HAL Initilization Structures */
GPIOInitConfig_t gpio_config[] = {
    // Status LEDs
    GPIO_INIT_OUTPUT(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(CONNECTION_LED_PORT, CONNECTION_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(ERROR_LED_PORT, ERROR_LED_PIN, GPIO_OUTPUT_LOW_SPEED),

    // VCAN
    GPIO_INIT_FDCAN1RX_PA11,
    GPIO_INIT_FDCAN1TX_PA12,

    // CCAN
    GPIO_INIT_FDCAN2RX_PB12,
    GPIO_INIT_FDCAN2TX_PB13,

    // SPI for BMS
    GPIO_INIT_OUTPUT(SPI1_CS_PORT, SPI1_CS_PIN, GPIO_OUTPUT_ULTRA_SPEED),
    GPIO_INIT_SPI1SCK_PA5,
    GPIO_INIT_SPI1MISO_PA6,
    GPIO_INIT_SPI1MOSI_PA7,

    // ISENSE and VSENSE
    GPIO_INIT_ANALOG(ISENSE_GPIO_PORT, ISENSE_GPIO_PIN),
    GPIO_INIT_ANALOG(VBATT_GPIO_PORT, VBATT_GPIO_PIN),

    // Input status pins
    GPIO_INIT_INPUT(CHARGER_CONNECTED_PORT, CHARGER_CONNECTED_PIN, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_INPUT(NOT_PRECHARGE_COMPLETE_PORT, NOT_PRECHARGE_COMPLETE_PIN, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_INPUT(IMD_STATUS_PORT, IMD_STATUS_PIN, GPIO_INPUT_OPEN_DRAIN),

    // BMS SDC Control
    GPIO_INIT_OUTPUT(BMS_SDC_CTRL_PORT, BMS_SDC_CTRL_PIN, GPIO_OUTPUT_LOW_SPEED)
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

adbms_bms_t g_bms                              = {0};
uint8_t g_bms_tx_buf[ADBMS_SPI_TX_BUFFER_SIZE] = {0};

static constexpr float MIN_V_FOR_BALANCE     = 3.0f;
static constexpr float MIN_DELTA_FOR_BALANCE = 0.1f;

extern void HardFault_Handler(void);
void bms_task(void);
void check_faults(void);
void report_telemetry(void);
void charging_fsm_periodic(void);

// Thread Defines
DEFINE_TASK(bms_task, 200, osPriorityHigh, STACK_2048);
DEFINE_TASK(CAN_rx_update, 0, osPriorityHigh, STACK_2048);
DEFINE_TASK(CAN_tx_update, 2, osPriorityNormal, STACK_2048);
DEFINE_TASK(check_faults, 10, osPriorityNormal, STACK_512);
DEFINE_TASK(charging_fsm_periodic, ELCON_COMMAND_PERIOD_MS, osPriorityNormal, STACK_512);
DEFINE_TASK(fault_library_periodic, A_BOX_FAULT_SYNC_PERIOD_MS, osPriorityNormal, STACK_1024);
DEFINE_TASK(report_telemetry, PACK_STATS_PERIOD_MS, osPriorityLow, STACK_512);
DEFINE_HEARTBEAT_TASK(nullptr);

int main(void) {
    // Hardware Initilization
    if (0 != PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }
    if (false == PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
        HardFault_Handler();
    }

    // Set CS high to start
    adbms6380_set_cs_high(&bms_spi_config);

    if (!PHAL_SPI_init(&bms_spi_config)) {
        HardFault_Handler();
    }

    adbms_init(&g_bms, &bms_spi_config, g_bms_tx_buf);

    if (false == PHAL_initADC(&adc_config, adc_channel_config, sizeof(adc_channel_config) / sizeof(ADCChannelConfig_t))) {
        HardFault_Handler();
    }
    if (false == PHAL_initDMA(&adc_dma_config)) {
        HardFault_Handler();
    }
    PHAL_startADC(&adc_config);
    PHAL_startTxfer(&adc_dma_config);

    if (false == PHAL_FDCAN_init(FDCAN1, false, VCAN_BAUD_RATE)) {
        HardFault_Handler();
    }
    if (false == PHAL_FDCAN_init(FDCAN2, false, CCAN_BAUD_RATE)) {
        HardFault_Handler();
    }
    NVIC_SetPriority(FDCAN1_IT0_IRQn, 5);
    NVIC_SetPriority(FDCAN2_IT0_IRQn, 5);

    NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
    NVIC_EnableIRQ(FDCAN2_IT0_IRQn);
    CAN_library_init();

    // Kernel initalization
    osKernelInitialize();

    START_TASK(bms_task);
    START_TASK(CAN_rx_update);
    START_TASK(CAN_tx_update);
    START_TASK(check_faults);
    START_TASK(fault_library_periodic);
    START_TASK(report_telemetry);
    START_TASK(charging_fsm_periodic);
    START_HEARTBEAT_TASK();

    osKernelStart(); // no way home

    return 0;
}

static inline float get_isense_correction_offset(float current) {
    // Linear fit to benchtop data to correct for sensor non-idealities, especially at low currents.
    // R^2 = 0.8428
    float abs_current = fabsf(current);
    return -0.0495f * abs_current + 3.3756f;
}

// DHAB S/134 current sensor conversion
static inline float isense_to_current(uint16_t isense_raw) {
    static constexpr float ADC_VREF = 3.3f;
    static constexpr float ADC_MAX  = 4095.0f;
    static constexpr float ADC_TO_VOLTS = ADC_VREF / ADC_MAX;

    static constexpr float DIV_R1   = 2400.0f;
    static constexpr float DIV_R2   = 4700.0f;
    static constexpr float DIV_GAIN = (DIV_R1 + DIV_R2) / DIV_R2;

    static constexpr float V_OFFSET = 2.5f;
    static constexpr float G        = 10.0e-3f;

    float v_adc      = isense_raw * ADC_TO_VOLTS;
    float v_sensor   = v_adc * DIV_GAIN;
    float current    = (v_sensor - V_OFFSET) / G; // data
    float correction = get_isense_correction_offset(current);

    // Apply correction in the correct direction
    if (current < 0.0f) current -= correction;
    else current += correction;

    return current;
}

static inline uint16_t vbatt_to_voltage(uint16_t vbatt_raw) {
    static constexpr float ADC_VREF = 3.3f;
    static constexpr float ADC_MAX  = 4095.0f;
    static constexpr float ADC_TO_VOLTS = ADC_VREF / ADC_MAX;

    static constexpr float RTOP   = 2'375'000.0f;
    static constexpr float RSENSE = 7943.2f;

    static constexpr float DIV_GAIN    = (RTOP + RSENSE) / RSENSE; // ~300
    static constexpr float ANALOG_GAIN = 2.0f; // set to 1.0f if ADC sees 0-2V node

    float v_adc  = vbatt_raw * ADC_TO_VOLTS;
    float voltage = v_adc * DIV_GAIN / ANALOG_GAIN;

    return voltage;
}

void report_telemetry() {
    uint16_t pack_voltage = (uint16_t)(vbatt_to_voltage(adc1_dma_buffer.vbatt_raw) * PACK_COEFF_PACK_STATS_PACK_VOLTAGE);
    int16_t pack_current  = (int16_t)(isense_to_current(adc1_dma_buffer.isense_raw) * PACK_COEFF_PACK_STATS_PACK_CURRENT);

    CAN_SEND_pack_stats(pack_voltage, pack_current, g_bms.avg_therm_temp);

    uint16_t min_cell_voltage = (uint16_t)(g_bms.min_voltage * PACK_COEFF_CHARGING_TELEMETRY_MIN_CELL_VOLTAGE);
    uint16_t max_cell_voltage = (uint16_t)(g_bms.max_voltage * PACK_COEFF_CHARGING_TELEMETRY_MAX_CELL_VOLTAGE);

    CAN_SEND_charging_telemetry(
        pack_voltage,
        pack_current,
        min_cell_voltage,
        max_cell_voltage
    );

    // Report cell voltages one at a time
    static uint8_t module_num      = 0;
    static uint8_t cell_num        = 0;
    adbms_module_t *current_module = &g_bms.modules[module_num];

    float cell_voltage = current_module->cell_voltages[cell_num];
    uint16_t scaled_cell_voltage = (uint16_t)(cell_voltage * PACK_COEFF_CELL_TELEMETRY_CELL_VOLTAGE);
    bool is_balancing  = current_module->is_discharging[cell_num];

    CAN_SEND_cell_telemetry(scaled_cell_voltage, module_num, cell_num, is_balancing);

    if (++cell_num >= ADBMS6380_CELL_COUNT) {
        cell_num = 0;
        if (++module_num >= ADBMS_MODULE_COUNT) {
            module_num = 0;
        }
    }
}

void bms_task() {
    adbms_periodic(&g_bms, MIN_V_FOR_BALANCE, MIN_DELTA_FOR_BALANCE);
}

void check_faults() {
    // IMD
    bool imd_faulted = !PHAL_readGPIO(IMD_STATUS_PORT, IMD_STATUS_PIN);
    update_fault(FAULT_ID_IMD, imd_faulted);

    // BMS
    bool is_bms_disconnected = g_bms.state != ADBMS_STATE_CONNECTED;
    update_fault(FAULT_ID_BMS_DISCONNECTED, is_bms_disconnected);
    PHAL_writeGPIO(BMS_SDC_CTRL_PORT, BMS_SDC_CTRL_PIN, is_clear(FAULT_ID_BMS_DISCONNECTED));

    // Cell voltage bounds
    int16_t scaled_min_voltage = (int16_t)(g_bms.min_voltage * 10.0f);
    update_fault(FAULT_ID_CELL_UNDERVOLTAGE, scaled_min_voltage);

    // Temperature related
    update_fault(FAULT_ID_PACK_OVERTEMP, g_bms.max_therm_temp);
    update_fault(FAULT_ID_PACK_WARM, g_bms.max_therm_temp);
    update_fault(FAULT_ID_PACK_COLD, g_bms.min_therm_temp);
}

// todo reboot on hardfault
void HardFault_Handler() {
    __disable_irq();
    SysTick->CTRL        = 0;
    ERROR_LED_PORT->BSRR = (1 << ERROR_LED_PIN); // Turn on error LED
    while (1) {
        __asm__("NOP"); // Halt forever
    }
}
