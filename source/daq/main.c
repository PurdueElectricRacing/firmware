#include "main.h"
#include "spmc.h"
#include "common/can_library/generated/DAQ.h"
#include "common/freertos/freertos.h"
#include "common/phal/can.h"
#include "common/phal/gpio.h"
#include "common/phal/rcc.h"
#include "common/phal/rtc.h"
#include "common/phal/spi.h"
#include "common/phal/usart.h"
#include "daq_spi.h"
#include "common/heartbeat/heartbeat.h"

GPIOInitConfig_t gpio_config[] = {
    // LEDs
    GPIO_INIT_OUTPUT(HEARTBEAT_LED_PORT, HEARTBEAT_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(CONNECTION_LED_PORT, CONNECTION_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(ERROR_LED_PORT, ERROR_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(SD_ACTIVITY_LED_PORT, SD_ACTIVITY_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(SD_ERROR_LED_PORT, SD_ERROR_LED_PIN, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(SD_DETECT_LED_PORT, SD_DETECT_LED_PIN, GPIO_OUTPUT_LOW_SPEED),

    // W5500 ETH SPI1
    GPIO_INIT_AF(ETH_SCK_PORT, ETH_SCK_PIN, 5, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_PULL_DOWN),
    GPIO_INIT_AF(ETH_MISO_PORT, ETH_MISO_PIN, 5, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_AF(ETH_MOSI_PORT, ETH_MOSI_PIN, 5, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_PULL_DOWN),
    GPIO_INIT_OUTPUT(ETH_CS_PORT, ETH_CS_PIN, GPIO_OUTPUT_HIGH_SPEED),
    GPIO_INIT_OUTPUT_OPEN_DRAIN(ETH_RST_PORT, ETH_RST_PIN, GPIO_OUTPUT_LOW_SPEED),

    // SDIO
    GPIO_INIT_SDIO_CLK,
    GPIO_INIT_SDIO_CMD,
    GPIO_INIT_SDIO_DT0,
    GPIO_INIT_SDIO_DT1,
    GPIO_INIT_SDIO_DT2,
    GPIO_INIT_SDIO_DT3,
    GPIO_INIT_INPUT(SD_CD_PORT, SD_CD_PIN, GPIO_INPUT_PULL_UP),
    GPIO_INIT_INPUT(LOG_ENABLE_PORT, LOG_ENABLE_PIN, GPIO_INPUT_PULL_DOWN),
    GPIO_INIT_INPUT(PWR_LOSS_PORT, PWR_LOSS_PIN, GPIO_INPUT_OPEN_DRAIN), // SPL EXTI

    // LTE UART
    GPIO_INIT_USART6TX_PC6,
    GPIO_INIT_USART6RX_PC7,

    // CAN1
    GPIO_INIT_CANRX_PA11,
    GPIO_INIT_CANTX_PA12,

    // CAN2
    GPIO_INIT_CAN2RX_PB5,
    GPIO_INIT_CAN2TX_PB6,
};

/* CLOCK CONFIG */
extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

#define TargetCoreClockrateHz 168000000
ClockRateConfig_t clock_config = {
    .clock_source              = CLOCK_SOURCE_HSE,
    .use_pll                   = true,
    .pll_src                   = PLL_SRC_HSE,
    .vco_output_rate_target_hz = 336000000, //288000000,
    .system_clock_target_hz    = TargetCoreClockrateHz,
    .ahb_clock_target_hz       = (TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz      = (TargetCoreClockrateHz / 4),
    .apb2_clock_target_hz      = (TargetCoreClockrateHz / 4),
};

/* SPI CONFIG FOR ETHERNET MODULE */
dma_init_t spi_rx_dma_config    = SPI1_RXDMA_CONT_CONFIG(NULL, 2);
dma_init_t spi_tx_dma_config    = SPI1_TXDMA_CONT_CONFIG(NULL, 1);
SPI_InitConfig_t eth_spi_config = {
    .data_len      = 8,
    .nss_sw        = false,
    .nss_gpio_port = ETH_CS_PORT,
    .nss_gpio_pin  = ETH_CS_PIN,
    .rx_dma_cfg    = &spi_rx_dma_config,
    .tx_dma_cfg    = &spi_tx_dma_config,
    .periph        = SPI1,
};

RTC_timestamp_t fallback_timestamp ={
    .date = {
        .month_bcd = RTC_MONTH_UNKNOWN,
        .weekday   = RTC_WEEKDAY_UNKNOWN,
        .day_bcd   = 0x00,
        .year_bcd  = 0x00
    },
    .time = {
        .hours_bcd   = 0x00,
        .minutes_bcd = 0x00,
        .seconds_bcd = 0x00,
        .time_format = RTC_FORMAT_24_HOUR
    },
};

daq_hub_t daq_hub = {
    // Ethernet
    .eth_state           = ETH_LINK_IDLE,
    .eth_tcp_state       = ETH_TCP_IDLE,
    .eth_error_ct        = 0,
    .eth_last_error_time = 0,
    .eth_last_err        = ETH_ERROR_NONE,
    .eth_last_err_res    = 0,

    // SD Card
    .sd_state           = SD_STATE_IDLE,
    .sd_error_ct        = 0,
    .sd_last_error_time = 0,
    .sd_last_err        = SD_ERROR_NONE,
    .sd_last_err_res    = 0,
    .sd_task_handle     = NULL,

    .rtc_config_state   = RTC_SYNC_PENDING,
    
    .last_file_ms       = 0,
    .last_write_ms      = 0,
    .log_enable_sw      = false,

    .can1_rx_overflow   = 0,
    .sd_rx_overflow     = 0
};

// Static buffer allocations
SPMC_t spmc;
timestamped_frame_t buf;
DEFINE_MUTEX(spi1_lock);

static void configure_interrupts(void);
void shutdown(void);

DEFINE_TASK(sd_update_periodic, 100, osPriorityNormal, 4096); // SD WRITE
DEFINE_TASK(eth_update_periodic, 50, osPriorityNormal, 4096); // BULLET COMMS 
DEFINE_HEARTBEAT_TASK(nullptr);

int main() {
    if (0 != PHAL_configureClockRates(&clock_config)) {
        HardFault_Handler();
    }
    if (!PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t))) {
        HardFault_Handler();
    }
        
    PHAL_writeGPIO(ETH_RST_PORT, ETH_RST_PIN, 1);
    if (!PHAL_SPI_init(&eth_spi_config)) {
        HardFault_Handler();
    }

    if (!PHAL_configureRTC(&fallback_timestamp, false)) {
        HardFault_Handler();
    }

    // ! CAN1 is bricked, use CAN2 on VCAN for now
    // if (!PHAL_initCAN(CAN1, false, VCAN_BAUD_RATE)) {
    //     HardFault_Handler();
    // }
    if (!PHAL_initCAN(CAN2, false, VCAN_BAUD_RATE)) {
        HardFault_Handler();
    }

    daq_spi_register_callbacks(); // Link SPI for ethernet driver

    osKernelInitialize();
    SPMC_init(&spmc);
    configure_interrupts();

    INIT_MUTEX(spi1_lock);

    START_TASK(sd_update_periodic);  // SD WRITE
    START_TASK(eth_update_periodic); // BULLET COMMS
    START_HEARTBEAT_TASK();

    osKernelStart();

    return 0;
}

static void configure_interrupts(void) {
    // Configure exti interupt for power loss pin (PE15)
    // Enable the SYSCFG clock for interrupts
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI15_PE; // Map PE15 to EXTI 15
    EXTI->IMR |= EXTI_IMR_MR15; // Unmask EXTI15
    EXTI->FTSR |= EXTI_FTSR_TR15; // Enable the falling edge trigger (active low reset)
    NVIC_SetPriority(EXTI15_10_IRQn, 15); // allow other interrupts to preempt this one (especially systick and dma)
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

static inline void can_rx_irq_handler(CAN_TypeDef* can_h) {
    portBASE_TYPE xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;

    // TODO: track FIFO overrun and full errors
    if (can_h->RF0R & CAN_RF0R_FOVR0) // FIFO Overrun
        can_h->RF0R &= ~(CAN_RF0R_FOVR0);

    if (can_h->RF0R & CAN_RF0R_FULL0) // FIFO Full
        can_h->RF0R &= ~(CAN_RF0R_FULL0);

    if (can_h->RF0R & CAN_RF0R_FMP0_Msk) // Release message pending
    {
        timestamped_frame_t* rx = &buf;
        rx->ticks_ms    = getTick();
        if (can_h == CAN1) {
            // bus ID is 31st bit of identity field, set to 0 for CAN1
            rx->identity &= (uint32_t) ~(1 << SPMC_BUS_ID_Pos);
        }
        else {
            // CAN2, bus ID set to 1 
            rx->identity |= (uint32_t) (1 << SPMC_BUS_ID_Pos);
        }

        // Get either StdId or ExtId
        if (CAN_RI0R_IDE & can_h->sFIFOMailBox[0].RIR) {
            // Extended ID
            rx->identity |= (uint32_t) 1 << SPMC_IS_EXTID_Pos;
            rx->identity |= CAN_EFF_FLAG | (((CAN_RI0R_EXID | CAN_RI0R_STID) & can_h->sFIFOMailBox[0].RIR) >> CAN_RI0R_EXID_Pos); 
        } else {
            // Standard ID
            rx->identity &= (uint32_t) ~(1 << SPMC_IS_EXTID_Pos);
            rx->identity |= (CAN_RI0R_STID & can_h->sFIFOMailBox[0].RIR) >> CAN_TI0R_STID_Pos;
        }

        rx->payload = (uint64_t) (can_h->sFIFOMailBox[0].RDLR); 
        rx->payload |= (uint64_t) (can_h->sFIFOMailBox[0].RDHR) << 32;

        (void)SPMC_enqueue_from_ISR(&spmc, rx);

        if ((daq_hub.rtc_config_state != RTC_SYNC_COMPLETE) && ((rx->identity & STD_ID_MASK) == GPS_TIME_MSG_ID)) {
            rtc_config_cb(rx);
        }
    } 

    can_h->RF0R |= (CAN_RF0R_RFOM0);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void CAN1_RX0_IRQHandler() {
    can_rx_irq_handler(CAN1);
}

void CAN2_RX0_IRQHandler() {
    /* TODO if main relays CAN2 onto CAN1, then there will be redundant messages in logs */
    can_rx_irq_handler(CAN2);
}

/**
 * @brief Disables high power consumption devices
 *        If file open, flushes it to the sd card
 *        Then unmounts sd card
 */
void shutdown(void) {
    // First, turn off all power consuming devices to increase our write time to sd card
    // To whoever is doing future DAQ rev: also change the GPIO ports here
    // all LEDs go bye bye
    GPIOD->BSRR |= GPIO_CLEAR_BIT(HEARTBEAT_LED_PIN) | GPIO_CLEAR_BIT(CONNECTION_LED_PIN) | GPIO_CLEAR_BIT(ERROR_LED_PIN);
    GPIOA->BSRR |= GPIO_CLEAR_BIT(SD_DETECT_LED_PIN) | GPIO_CLEAR_BIT(SD_ACTIVITY_LED_PIN) | GPIO_CLEAR_BIT(SD_ERROR_LED_PIN);

    PHAL_writeGPIO(ETH_RST_PORT, ETH_RST_PIN, 0);
    PHAL_deinitCAN(CAN1);
    PHAL_deinitCAN(CAN2);

    sd_shutdown();
    // Hooray, we made it, blink an LED to show the world
    PHAL_writeGPIO(SD_DETECT_LED_PORT, SD_DETECT_LED_PIN, 1);

    // wait for power to fully turn off -> if it does not, restart
    uint32_t start_tick = getTick();
    while (getTick() - start_tick < 3000 || PHAL_readGPIO(PWR_LOSS_PORT, PWR_LOSS_PIN) == 0);

    NVIC_SystemReset(); // oof, we assumed wrong, restart and resume execution since the power is still on!
}

// Interrupt handler for power loss detection
// Note: this is set to lowest priority to allow preemption by other interrupts
void EXTI15_10_IRQHandler() {
    if (EXTI->PR & EXTI_PR_PR15) {
        EXTI->PR |= EXTI_PR_PR15; // Clear interrupt
        shutdown();
    }
}

void HardFault_Handler() {
    ERROR_LED_PORT->BSRR = (1 << ERROR_LED_PIN);
    while (1) {
        __asm__("nop");
    }
}
