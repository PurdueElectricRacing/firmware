#include "stm32l432xx.h"
#include "common/psched/psched.h"
#include "common/phal_L4/can/can.h"
#include "common/phal_L4/rcc/rcc.h"
#include "common/phal_L4/gpio/gpio.h"
#include "common/phal_L4/adc/adc.h"
#include "common/phal_L4/usart/usart.h"
#include "common/phal_L4/tim/tim.h"
#include "common/phal_L4/dma/dma.h"

/* Module Includes */
#include "main.h"
#include "apps.h"
#include "can_parse.h"

GPIOInitConfig_t gpio_config[] = {
  GPIO_INIT_CANRX_PA11,
  GPIO_INIT_CANTX_PA12,
  GPIO_INIT_USART2TX_PA2,
  GPIO_INIT_USART2RX_PA3,
  // EEPROM
  GPIO_INIT_I2C1_SCL_PA9,
  GPIO_INIT_I2C1_SDA_PA10,
  GPIO_INIT_OUTPUT(WC_GPIO_Port, WC_Pin, GPIO_OUTPUT_LOW_SPEED),
  // SPI
  GPIO_INIT_AF(SCK_GPIO_Port, SCK_Pin, 5, GPIO_OUTPUT_LOW_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_OPEN_DRAIN),
  GPIO_INIT_AF(MISO_GPIO_Port, MISO_Pin, 5, GPIO_OUTPUT_LOW_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_OPEN_DRAIN),
  GPIO_INIT_AF(MOSI_GPIO_Port, MOSI_Pin, 5, GPIO_OUTPUT_LOW_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_OPEN_DRAIN),
  GPIO_INIT_OUTPUT(CSB_WHL_GPIO_Port, CSB_WHL_Pin, GPIO_OUTPUT_LOW_SPEED),
  // Throttle
  GPIO_INIT_ANALOG(THTL_1_GPIO_Port, THTL_1_Pin),
  GPIO_INIT_ANALOG(THTL_2_GPIO_Port, THTL_2_Pin),
  // Brake
  GPIO_INIT_ANALOG(BRK_1_GPIO_Port, BRK_1_Pin),
  GPIO_INIT_ANALOG(BRK_2_GPIO_Port, BRK_2_Pin),
  GPIO_INIT_ANALOG(BRK_3_GPIO_Port, BRK_3_Pin),
  // Status LEDs
  GPIO_INIT_OUTPUT(ERR_LED_GPIO_Port, ERR_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
  GPIO_INIT_OUTPUT(HEART_LED_GPIO_Port, HEART_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
  GPIO_INIT_OUTPUT(PRCHG_LED_GPIO_Port, PRCHG_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
  GPIO_INIT_OUTPUT(IMD_LED_GPIO_Port, IMD_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
  GPIO_INIT_OUTPUT(BMS_LED_GPIO_Port, BMS_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
  GPIO_INIT_INPUT(START_BTN_GPIO_Port, START_BTN_Pin, GPIO_INPUT_PULL_DOWN),
};

/* ADC Configuration */
ADCInitConfig_t adc_config = {
    .clock_prescaler = ADC_CLK_PRESC_6,
    .resolution      = ADC_RES_12_BIT,
    .data_align      = ADC_DATA_ALIGN_RIGHT,
    .cont_conv_mode  = true,
    .overrun         = true,
    .dma_mode        = ADC_DMA_CIRCULAR
};
ADCChannelConfig_t adc_channel_config[] = {
    {.channel=THTL_1_ADC_CHNL, .rank=1, .sampling_time=ADC_CHN_SMP_CYCLES_2_5},
    {.channel=THTL_2_ADC_CHNL, .rank=2, .sampling_time=ADC_CHN_SMP_CYCLES_2_5},
    {.channel=BRK_1_ADC_CHNL, .rank=3, .sampling_time=ADC_CHN_SMP_CYCLES_2_5},
    {.channel=BRK_2_ADC_CHNL, .rank=4, .sampling_time=ADC_CHN_SMP_CYCLES_2_5},
    {.channel=BRK_3_ADC_CHNL, .rank=5, .sampling_time=ADC_CHN_SMP_CYCLES_2_5},
};
static uint16_t adc_conversions[5];
dma_init_t adc_dma_config = ADC1_DMA_CONT_CONFIG((uint32_t) adc_conversions, 5, 0b01);

/* USART Confiugration */
dma_init_t usart_tx_dma_config = USART2_TXDMA_CONT_CONFIG(NULL, 1);
dma_init_t usart_rx_dma_config = USART2_RXDMA_CONT_CONFIG(NULL, 2);
usart_init_t huart2 = {
    .baud_rate = 9600,
    .word_length = WORD_8,
    .stop_bits = SB_ONE,
    .parity = PT_NONE,
    .mode = MODE_TX_RX,
    .hw_flow_ctl = HW_DISABLE,
    .ovsample = OV_16,
    .obsample = OB_DISABLE,
    .adv_feature = {
                        .auto_baud = false,
                        .ab_mode = AB_START,
                        .tx_inv = false,
                        .rx_inv = false,
                        .data_inv = false,
                        .tx_rx_swp = false,
                        .overrun = false,
                        .dma_on_rx_err = false,
                        .msb_first = false,
                   },
    .tx_dma_cfg = &usart_tx_dma_config,
    .rx_dma_cfg = &usart_rx_dma_config
};

#define TargetCoreClockrateHz 16000000
ClockRateConfig_t clock_config = {
    .system_source              =SYSTEM_CLOCK_SRC_HSI,
    .system_clock_target_hz     =TargetCoreClockrateHz,
    .ahb_clock_target_hz        =(TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz       =(TargetCoreClockrateHz / (1)),
    .apb2_clock_target_hz       =(TargetCoreClockrateHz / (1)),
};

/* Locals for Clock Rates */
extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

/* Function Prototypes */
void canTxUpdate();
extern void HardFault_Handler();

q_handle_t q_tx_can;
q_handle_t q_rx_can;

int main (void)
{

    /* Data Struct init */
    qConstruct(&q_tx_can, sizeof(CanMsgTypeDef_t));
    qConstruct(&q_rx_can, sizeof(CanMsgTypeDef_t));

    /* HAL Initilization */
    if(0 != PHAL_configureClockRates(&clock_config))
    {
        HardFault_Handler();
    }
    if(!PHAL_initGPIO(gpio_config, sizeof(gpio_config)/sizeof(GPIOInitConfig_t)))
    {
        HardFault_Handler();
    }
    if(!PHAL_initCAN(CAN1, false))
    {
        HardFault_Handler();
    }
    NVIC_EnableIRQ(CAN1_RX0_IRQn);
    if(!PHAL_initUSART(USART2, &huart2, APB2ClockRateHz))
    {
        HardFault_Handler();
    }
    if(!PHAL_initI2C())
    {
        HardFault_Handler();
    }
    if(!PHAL_initADC(ADC1, &adc_config, adc_channel_config, sizeof(adc_channel_config)/sizeof(ADCChannelConfig_t)))
    {
        HardFault_Handler();
    }
    if(!PHAL_initDMA(&adc_dma_config))
    {
        HardFault_Handler();
    }
    PHAL_startTxfer(&adc_dma_config);
    PHAL_startADC(ADC1);

    // Signify start of initialization
    PHAL_writeGPIO(HEART_LED_GPIO_Port, HEART_LED_Pin, 1);

    /* Module Initialization */
    // initCANParse(&q_rx_can);

    /* Task Creation */
    schedInit(SystemCoreClock);

    // taskCreateBackground(canTxUpdate);
    // taskCreateBackground(canRxUpdate);

    // Signify end of initialization
    PHAL_writeGPIO(HEART_LED_GPIO_Port, HEART_LED_Pin, 0);
    schedStart();
    
    return 0;
}

void sendPedalValues()
{

}


void canTxUpdate()
{
    CanMsgTypeDef_t tx_msg;
    if (qReceive(&q_tx_can, &tx_msg) == SUCCESS_G)    // Check queue for items and take if there is one
    {
        PHAL_txCANMessage(&tx_msg);
    }
}

void HardFault_Handler()
{
    PHAL_writeGPIO(ERR_LED_GPIO_Port, ERR_LED_Pin, 1);
    while(1)
    {
        __asm__("nop");
    }
}