/**
 * @file led_blink.c
 * @author Eileen Yoon (eyn@purdue.edu)
 * @brief  Demo for F4 testing structure
 *         - psched LED blink + uart serial printf
 * @version 0.1
 * @date 2025-01-16
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "f4_testing.h"
#include "common/common_defs/common_defs.h"
#include "common/psched/psched.h"
#include "common/phal_F4_F7/usart/usart.h"
#include "common/phal_F4_F7/gpio/gpio.h"
#include "common/phal_F4_F7/can/can.h"
#include "common/phal_F4_F7/rcc/rcc.h"
#include "common/phal_F4_F7/adc/adc.h"
#include "common/phal_F4_F7/spi/spi.h"
#include "common/phal_F4_F7/dma/dma.h"
#include "common/faults/faults.h"

// Guard so cmake doesn't compile all tests
#if (F4_TESTING_CHOSEN == TEST_LED_BLINK)

#include "common/phal_F4_F7/rcc/rcc.h"
#include "common/phal_F4_F7/gpio/gpio.h"
#include "common/phal_F4_F7/usart/usart.h"
#include "common/psched/psched.h"
#include "common/log/log.h"

void HardFault_Handler();
void ledblink();
void throttle_read();

// Brake
#define BRK_1_GPIO_Port             (GPIOA)
#define BRK_1_Pin                   (0)
#define BRK_1_ADC_CHNL              (0)
#define BRK_2_GPIO_Port             (GPIOA)
#define BRK_2_Pin                   (1)
#define BRK_2_ADC_CHNL              (1)

// Throttle
#define THTL_1_GPIO_Port            (GPIOA)
#define THTL_1_Pin                  (2)
#define THTL_1_ADC_CHNL             (2)
#define THTL_2_GPIO_Port            (GPIOA)
#define THTL_2_Pin                  (3)
#define THTL_2_ADC_CHNL             (3)

GPIOInitConfig_t gpio_config[] = {
    GPIO_INIT_OUTPUT(GPIOD, 12, GPIO_OUTPUT_LOW_SPEED), // F407VGT disco LEDs
    GPIO_INIT_OUTPUT(GPIOD, 13, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(GPIOD, 14, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(GPIOD, 15, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_USART2TX_PA2,
    GPIO_INIT_USART2RX_PA3,

    // CAN
    GPIO_INIT_CANRX_PD0,
    GPIO_INIT_CANTX_PD1,

    // Throttle
    GPIO_INIT_ANALOG(THTL_1_GPIO_Port, THTL_1_Pin),
    GPIO_INIT_ANALOG(THTL_2_GPIO_Port, THTL_2_Pin),
    // Brake
    GPIO_INIT_ANALOG(BRK_1_GPIO_Port, BRK_1_Pin),
    GPIO_INIT_ANALOG(BRK_2_GPIO_Port, BRK_2_Pin),
};

ADCChannelConfig_t adc_channel_config[] = {
    {.channel = THTL_1_ADC_CHNL,        .rank = 1,  .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = THTL_2_ADC_CHNL,        .rank = 2,  .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = BRK_1_ADC_CHNL,         .rank = 3,  .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = BRK_2_ADC_CHNL,         .rank = 4,  .sampling_time = ADC_CHN_SMP_CYCLES_480},
};

typedef struct __attribute__((packed))
{
    // Do not modify this struct unless
    // you modify the ADC DMA config
    // in main.h to match
    uint16_t t1;
    uint16_t t2;
    uint16_t b1;
    uint16_t b2;
} raw_adc_values_t;
volatile raw_adc_values_t raw_adc_values;

/* ADC Configuration */
ADCInitConfig_t adc_config = {
    .clock_prescaler = ADC_CLK_PRESC_6,
    .resolution      = ADC_RES_12_BIT,
    .data_align      = ADC_DATA_ALIGN_RIGHT,
    .cont_conv_mode  = true,
    .dma_mode        = ADC_DMA_CIRCULAR,
    .adc_number      = 1,
};

dma_init_t adc_dma_config = ADC1_DMA_CONT_CONFIG((uint32_t) &raw_adc_values, sizeof(raw_adc_values) / sizeof(raw_adc_values.t1), 0b01);

extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

volatile uint32_t tick_ms; // Systick 1ms counter

#define TargetCoreClockrateHz 16000000
ClockRateConfig_t clock_config = {
    .clock_source               =CLOCK_SOURCE_HSI,
    .use_pll                    =false,
    .vco_output_rate_target_hz  =160000000,
    .system_clock_target_hz     =TargetCoreClockrateHz,
    .ahb_clock_target_hz        =(TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz       =(TargetCoreClockrateHz / (1)),
    .apb2_clock_target_hz       =(TargetCoreClockrateHz / (1)),
};

dma_init_t usart_tx_dma_config = USART2_TXDMA_CONT_CONFIG(NULL, 1);
dma_init_t usart_rx_dma_config = USART2_RXDMA_CONT_CONFIG(NULL, 2);
usart_init_t usart_config = {
   .baud_rate   = 115200,
   .word_length = WORD_8,
   .stop_bits   = SB_ONE,
   .parity      = PT_NONE,
   .hw_flow_ctl = HW_DISABLE,
   .ovsample    = OV_16,
   .obsample    = OB_DISABLE,
   .periph      = USART2,
   .wake_addr = false,
   .usart_active_num = USART2_ACTIVE_IDX,
   .tx_dma_cfg = &usart_tx_dma_config,
   .rx_dma_cfg = &usart_rx_dma_config
};
DEBUG_PRINTF_USART_DEFINE(&usart_config) // use LTE uart lmao

int main()
{
    if(0 != PHAL_configureClockRates(&clock_config))
    {
        HardFault_Handler();
    }
    if(!PHAL_initGPIO(gpio_config, sizeof(gpio_config)/sizeof(GPIOInitConfig_t)))
    {
        HardFault_Handler();
    }
    SysTick_Config(SystemCoreClock / 1000);
    NVIC_EnableIRQ(SysTick_IRQn);

    if(!PHAL_initUSART(&usart_config, APB1ClockRateHz))
    {
        HardFault_Handler();
    }
    log_yellow("PER PER PER\n");

    if (false == PHAL_initCAN(CAN1, false, VCAN_BPS))
    {
        HardFault_Handler();
    }
    NVIC_EnableIRQ(CAN1_RX0_IRQn);

    if (false == PHAL_initADC(ADC1, &adc_config, adc_channel_config, sizeof(adc_channel_config)/sizeof(ADCChannelConfig_t)))
    {
        HardFault_Handler();
    }
    if (false == PHAL_initDMA(&adc_dma_config))
    {
        HardFault_Handler();
    }
    PHAL_startTxfer(&adc_dma_config);
    PHAL_startADC(ADC1);

    schedInit(APB1ClockRateHz);
    taskCreate(ledblink, 250);
    taskCreate(throttle_read, 50);
    schedStart();

    return 0;
}

void ledblink()
{
    PHAL_toggleGPIO(GPIOD, 13);
}

#define ID_RAW_THROTTLE_BRAKE 0x10000285
#define DLC_RAW_THROTTLE_BRAKE 8

typedef union {
    struct {
        uint64_t throttle: 12;
        uint64_t throttle_right: 12;
        uint64_t brake: 12;
        uint64_t brake_right: 12;
        uint64_t brake_pot: 12;
    } raw_throttle_brake;
    uint8_t raw_data[8];
} __attribute__((packed)) CanParsedData_t;

#define CAN_TX_BLOCK_TIMEOUT (30 * 16000) // clock rate 16MHz, 15ms * 16000 cyc / ms
// Sends all pending messages in the tx queue, doesn't require systick to be active
static void canTxSendToBack(CanMsgTypeDef_t *tx_msg)
{
    #if 0
    uint32_t t = 0;
    while (!PHAL_txMailboxFree(CAN1, 0) && (t++ < CAN_TX_BLOCK_TIMEOUT));
    if (t < CAN_TX_BLOCK_TIMEOUT) PHAL_txCANMessage(tx_msg, 0);
    #endif
}

#define SEND_RAW_THROTTLE_BRAKE(throttle_, throttle_right_, brake_, brake_right_, brake_pot_) do {\
        CanMsgTypeDef_t msg = {.Bus=CAN1, .ExtId=ID_RAW_THROTTLE_BRAKE, .DLC=DLC_RAW_THROTTLE_BRAKE, .IDE=1};\
        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\
        data_a->raw_throttle_brake.throttle = throttle_;\
        data_a->raw_throttle_brake.throttle_right = throttle_right_;\
        data_a->raw_throttle_brake.brake = brake_;\
        data_a->raw_throttle_brake.brake_right = brake_right_;\
        data_a->raw_throttle_brake.brake_pot = brake_pot_;\
        canTxSendToBack(&msg);\
    } while(0)

void throttle_read()
{
    SEND_RAW_THROTTLE_BRAKE(raw_adc_values.t1, raw_adc_values.t2, raw_adc_values.b1, raw_adc_values.b2, 0);
}

void HardFault_Handler()
{
    while(1)
    {
        __asm__("nop");
    }
}

#endif // F4_TESTING_CHOSEN == TEST_LED_BLINK
