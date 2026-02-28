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
#include "daq_can.h"
#include "daq_hub.h"
#include "daq_spi.h"

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
    GPIO_INIT_INPUT(LOG_ENABLE_PORT, LOG_ENABLE_PIN, GPIO_INPUT_PULL_UP),
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

RTC_timestamp_t start_time =
    {
        .date = {.month_bcd = RTC_MONTH_UNKNOWN,
                 .weekday   = RTC_WEEKDAY_UNKNOWN,
                 .day_bcd   = 0x00,
                 .year_bcd  = 0x00},
        .time = {.hours_bcd   = 0x00,
                 .minutes_bcd = 0x00,
                 .seconds_bcd = 0x00,
                 .time_format = RTC_FORMAT_24_HOUR},
};

dma_init_t usart_tx_dma_config = USART6_TXDMA_CONT_CONFIG(NULL, 1);
dma_init_t usart_rx_dma_config = USART6_RXDMA_CONT_CONFIG(NULL, 2);
usart_init_t lte_usart_config  = {
     .baud_rate        = 115200,
     .word_length      = WORD_8,
     .stop_bits        = SB_ONE,
     .parity           = PT_NONE,
     .hw_flow_ctl      = HW_DISABLE,
     .ovsample         = OV_16,
     .obsample         = OB_DISABLE,
     .periph           = USART6,
     .wake_addr        = false,
     .usart_active_num = USART6_ACTIVE_IDX,
     .tx_dma_cfg       = &usart_tx_dma_config,
     .rx_dma_cfg       = &usart_rx_dma_config};
DEBUG_PRINTF_USART_DEFINE(&lte_usart_config) // use LTE uart lmao

extern daq_hub_t daq_hub;

// Static buffer allocations
SPMC_t queue;
timestamped_frame_t buf;
defineStaticSemaphore(spi1_lock);

static void configure_interrupts(void);
bool can_parse_error_status(uint32_t err, timestamped_frame_t* frame);
void shutdown(void);

int main() {
    osKernelInitialize();

    SPMC_init(&queue, NULL,  NULL);

    if (0 != PHAL_configureClockRates(&clock_config))
        HardFault_Handler();

    if (!PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t)))
        HardFault_Handler();

    PHAL_writeGPIO(ETH_RST_PORT, ETH_RST_PIN, 1);
    if (!PHAL_SPI_init(&eth_spi_config))
        HardFault_Handler();

    if (!PHAL_configureRTC(&start_time, false))
        HardFault_Handler();

    if (!PHAL_initUSART(&lte_usart_config, APB2ClockRateHz))
        HardFault_Handler();
    log_yellow("PER PER PER\n");

    PHAL_initCAN(CAN1, false, MCAN_BAUD_RATE);

    if (!PHAL_initCAN(CAN2, false, VCAN_BAUD_RATE))
        HardFault_Handler();

    daq_spi_register_callbacks(); // Link SPI for ethernet driver
    daq_hub_init();
    configure_interrupts();

    createStaticSemaphore(spi1_lock);
    createStaticQueue(q_tcp_tx, timestamped_frame_t, TCP_TX_ITEM_COUNT);
    createStaticQueue(q_can1_rx, timestamped_frame_t, DAQ_CAN1_RX_COUNT);
    daq_create_threads();

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

    NVIC_SetPriority(CAN1_RX0_IRQn, 6); // highest RTOS priority
    NVIC_SetPriority(CAN2_RX0_IRQn, 7);
    NVIC_SetPriority(CAN1_SCE_IRQn, 10);

    NVIC_EnableIRQ(CAN1_RX0_IRQn);
    NVIC_EnableIRQ(CAN1_SCE_IRQn);
    NVIC_EnableIRQ(CAN2_RX0_IRQn);
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

// TODO verify with canable that this works 
static void can_rx_irq_handler(CAN_TypeDef* can_h) {
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
        rx->identity = (uint32_t) (1 << 31);
        }
        rx->identity = (uint32_t) ((can_h == CAN1) ? BUS_ID_CAN1 : BUS_ID_CAN2) << 31;

        // Get either StdId or ExtId
        if (CAN_RI0R_IDE & can_h->sFIFOMailBox[0].RIR) {
            // Extended ID
            rx->identity |= (uint32_t) 1 << 30;
            rx->identity |= CAN_EFF_FLAG | (((CAN_RI0R_EXID | CAN_RI0R_STID) & can_h->sFIFOMailBox[0].RIR) >> CAN_RI0R_EXID_Pos); // idk how right ts is
        } else {
            // Standard ID
            rx->identity &= (uint32_t) ~(1 << 30);
            rx->identity |= (CAN_RI0R_STID & can_h->sFIFOMailBox[0].RIR) >> CAN_TI0R_STID_Pos;
        }

        rx->payload = (uint64_t) (can_h->sFIFOMailBox[0].RDLR); 
        rx->payload |= (uint64_t) (can_h->sFIFOMailBox[0].RDHR) << 32;

        SPMC_enqueue_ISR(&queue,rx);

        // i promise ill move this
        #define STD_ID_MASK ((1U < 11) - 1)
        if ((daq_hub.rtc_config_state != RTC_SYNC_COMPLETE) && ((rx->identity & STD_ID_MASK) == GPS_TIME_MSG_ID)) rtc_config_cb(rx);
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

volatile uint32_t error_irq_cnt = 0;

// TODO update to work with new struct
// void CAN1_SCE_IRQHandler() {
//     // TODO track errors
//     uint32_t err_stat;
//     error_irq_cnt++;
//     if (CAN1->MSR & CAN_MSR_ERRI) {
//         err_stat = CAN1->ESR;
//         CAN1->ESR &= ~(CAN_ESR_LEC_Msk);

//         timestamped_frame_t* rx;
//         uint32_t cont;
//         if (bGetHeadForWrite(&b_rx_can, (void**)&rx, &cont) == 0) {
//             rx->ticks_ms = getTick();
//             // can_parse_error_status(err_stat, rx);
//             bCommitWrite(&b_rx_can, 1);
//         }
//         CAN1->MSR |= CAN_MSR_ERRI; // clear interrupt
//     }
// }

// TODO update to work with new struct + figure out what it does bc I have not thought about it
// bool can_parse_error_status(uint32_t err, timestamped_frame_t* frame) {
//     //frame->echo_id = 0xFFFFFFFF;
//     frame->bus_id  = 0;
//     frame->msg_id  = CAN_ERR_FLAG | CAN_ERR_CRTL;
//     frame->dlc     = CAN_ERR_DLC;
//     frame->data[0] = CAN_ERR_LOSTARB_UNSPEC;
//     frame->data[1] = CAN_ERR_CRTL_UNSPEC;
//     frame->data[2] = CAN_ERR_PROT_UNSPEC;
//     frame->data[3] = CAN_ERR_PROT_LOC_UNSPEC;
//     frame->data[4] = CAN_ERR_TRX_UNSPEC;
//     frame->data[5] = 0;
//     frame->data[6] = 0;
//     frame->data[7] = 0;

//     if ((err & CAN_ESR_BOFF) != 0) {
//         frame->msg_id |= CAN_ERR_BUSOFF;
//     }

//     /*
// 	uint8_t tx_error_cnt = (err>>16) & 0xFF;
// 	uint8_t rx_error_cnt = (err>>24) & 0xFF;
// 	*/

//     if (err & CAN_ESR_EPVF) {
//         frame->data[1] |= CAN_ERR_CRTL_RX_PASSIVE | CAN_ERR_CRTL_TX_PASSIVE;
//     } else if (err & CAN_ESR_EWGF) {
//         frame->data[1] |= CAN_ERR_CRTL_RX_WARNING | CAN_ERR_CRTL_TX_WARNING;
//     }

//     uint8_t lec = (err >> 4) & 0x07;
//     if (lec != 0) { /* protocol error */
//         switch (lec) {
//             case 0x01: /* stuff error */
//                 frame->msg_id |= CAN_ERR_PROT;
//                 frame->data[2] |= CAN_ERR_PROT_STUFF;
//                 break;
//             case 0x02: /* form error */
//                 frame->msg_id |= CAN_ERR_PROT;
//                 frame->data[2] |= CAN_ERR_PROT_FORM;
//                 break;
//             case 0x03: /* ack error */
//                 frame->msg_id |= CAN_ERR_ACK;
//                 break;
//             case 0x04: /* bit recessive error */
//                 frame->msg_id |= CAN_ERR_PROT;
//                 frame->data[2] |= CAN_ERR_PROT_BIT1;
//                 break;
//             case 0x05: /* bit dominant error */
//                 frame->msg_id |= CAN_ERR_PROT;
//                 frame->data[2] |= CAN_ERR_PROT_BIT0;
//                 break;
//             case 0x06: /* CRC error */
//                 frame->msg_id |= CAN_ERR_PROT;
//                 frame->data[3] |= CAN_ERR_PROT_LOC_CRC_SEQ;
//                 break;
//             default:
//                 break;
//         }
//     }

//     return true;
// }

/**
 * @brief Disables high power consumption devices
 *        If file open, flushes it to the sd card
 *        Then unmounts sd card
 */
void shutdown(void) {
    daq_shutdown_hook();
    uint32_t start_tick = getTick();
    while (getTick() - start_tick < 3000 || PHAL_readGPIO(PWR_LOSS_PORT, PWR_LOSS_PIN) == 0) // wait for power to fully turn off -> if it does not, restart
    {
        //if (getTick() % 250 == 0) PHAL_toggleGPIO(SD_DETECT_LED_PORT, SD_DETECT_LED_PIN);
    }
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
    PHAL_writeGPIO(ERROR_LED_PORT, ERROR_LED_PIN, 1);
    while (1) {
        __asm__("nop");
    }
}
