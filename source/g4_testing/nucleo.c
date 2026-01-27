
#include "g4_testing.h"
#if (G4_TESTING_CHOSEN == TEST_NUCLEO)

#include "common/freertos/freertos.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"
#include "common/phal/can.h"
#include "main.h"
#include <string.h>

GPIOInitConfig_t gpio_config[] = {
    GPIO_INIT_OUTPUT(GPIOA, 5, GPIO_OUTPUT_LOW_SPEED), // USER LED
    GPIO_INIT_FDCAN1RX_PA11,    // CANFD
    GPIO_INIT_FDCAN1TX_PA12
};

#define TargetCoreClockrateHz 16000000
ClockRateConfig_t clock_config = {
    .clock_source              = CLOCK_SOURCE_HSI,
    .use_pll                   = false,
    .vco_output_rate_target_hz = 160000000,
    .system_clock_target_hz    = TargetCoreClockrateHz,
    .ahb_clock_target_hz       = (TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz      = (TargetCoreClockrateHz / (1)),
    .apb2_clock_target_hz      = (TargetCoreClockrateHz / (1)),
};

extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

void HardFault_Handler();

static void ledblink1(void);
static void can_tx_100hz(void);
static void can_rx_1khz(void);

defineThreadStack(ledblink1, 1000, osPriorityNormal, 64);
defineThreadStack(can_tx_100hz, 10, osPriorityHigh, 256);
defineThreadStack(can_rx_1khz, 1, osPriorityHigh, 256);

defineStaticQueue(q_can_rx, CanMsgTypeDef_t, 256);

int main() {
    osKernelInitialize();

    if (PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }

    if (!PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
        HardFault_Handler();
    }

    if (!PHAL_FDCAN_init(FDCAN1, false, 500000U)) {
        HardFault_Handler();
    }

    uint32_t sids[8] = {0x300, 0x301};
    uint32_t xids[8] = {0x1ABCDE1, 0x1ABCDE2, 0x1ABCDE3};
    PHAL_FDCAN_setFilters(FDCAN1, sids, 2, xids, 3);

    // Create threads
    createThread(ledblink1);
    createThread(can_tx_100hz);
    createThread(can_rx_1khz);

    q_can_rx = createStaticQueue(q_can_rx, CanMsgTypeDef_t, 256);

    // NVIC
    NVIC_SetPriority(FDCAN1_IT0_IRQn, 6);
    NVIC_EnableIRQ(FDCAN1_IT0_IRQn);

    osKernelStart(); // Go!

    return 0;
}

static void ledblink1(void) {
    PHAL_toggleGPIO(GPIOA, 5);
}

uint8_t items[8] = {0};

static void PHAL_FDCAN_testStandard(void) {
    CanMsgTypeDef_t msg;
    msg.Bus            = FDCAN1;
    msg.IDE            = 0;
    msg.StdId          = 0x300 + 4;
    uint8_t payload[8] = {'S', 'T', 'D', 'I', 'D', '_', 'T', 'X'};
    items[0]++;
    msg.DLC            = sizeof(payload);
    memcpy(msg.Data, items, sizeof(items));
    PHAL_FDCAN_send(&msg);
}

static void can_tx_100hz(void)
{
    PHAL_FDCAN_testStandard();
}

void PHAL_FDCAN_rxCallback(CanMsgTypeDef_t* msg) {
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        BaseType_t xHigherPriorityTaskWoken = 0;
        xQueueSendFromISR(q_can_rx, msg, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

volatile CanMsgTypeDef_t rx_frame_0;
static void can_rx_1khz(void) {
    CanMsgTypeDef_t rx_frame;
    while (xQueueReceive(q_can_rx, &rx_frame, (TickType_t)0) == pdTRUE) {
        rx_frame_0 = rx_frame;
        ;
    }
}


void HardFault_Handler() {
    while (1) {
        __asm__("nop");
    }
}

#endif // G4_TESTING_CHOSEN == TEST_NUCLEO
