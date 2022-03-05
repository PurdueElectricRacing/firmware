/* System Includes */
#include "stm32l432xx.h"
#include "common/psched/psched.h"
#include "common/phal_L4/rcc/rcc.h"
#include "common/phal_L4/gpio/gpio.h"
#include <math.h>
#include "system_stm32l4xx.h"
#include "can_parse.h"
#include "common/phal_L4/can/can.h"
#include "common/phal_L4/quadspi/quadspi.h"
#include "common/phal_L4/spi/spi.h"
#include "shockpot/shockpot.h"
#include "common/phal_L4/adc/adc.h"


/* Module Includes */
#include "main.h"

GPIOInitConfig_t gpio_config[] = {
    GPIO_INIT_ANALOG(POT_AMPL_LEFT_GPIO_Port, POT_AMPL_LEFT_Pin),
    GPIO_INIT_ANALOG(POT_AMPL_RIGHT_GPIO_Port, POT_AMPL_RIGHT_Pin)
};

ADCInitConfig_t adc_config = {
    .clock_prescaler = ADC_CLK_PRESC_6,
    .resolution      = ADC_RES_12_BIT,
    .data_align      = ADC_DATA_ALIGN_RIGHT,
    .cont_conv_mode  = true,
    .overrun         = true,
    .dma_mode        = ADC_DMA_CIRCULAR
};

ADCChannelConfig_t adc_channel_config[] = {
    {.channel=LeftPotADC_Channel, .rank=1, .sampling_time=ADC_CHN_SMP_CYCLES_2_5},
    {.channel=RightPotADC_Channel, .rank=2, .sampling_time=ADC_CHN_SMP_CYCLES_2_5},
};

uint16_t adc_conversions[2];

dma_init_t adc_dma_config = ADC1_DMA_CONT_CONFIG((uint32_t) adc_conversions, 2, 0b01);

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
void ledBlink(void);
void HardFault_Handler(void);

q_handle_t q_tx_can;
q_handle_t q_rx_can;

void canTxUpdate()
{
    CanMsgTypeDef_t tx_msg;
    if (qReceive(&q_tx_can, &tx_msg) == SUCCESS_G)    // Check queue for items and take if there is one
    {
        PHAL_txCANMessage(&tx_msg);
    }
}

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
    if(!PHAL_initADC(ADC1, &adc_config, adc_channel_config, sizeof(adc_channel_config)/sizeof(ADCChannelConfig_t)))
    {
        HardFault_Handler();
    }
    // if (1 != PHAL_initCAN(CAN1, false))
    //     HardFault_Handler();
    if(!PHAL_initDMA(&adc_dma_config))
    {
        HardFault_Handler();
    }
    PHAL_startTxfer(&adc_dma_config);
    PHAL_startADC(ADC1);

    /* Task Creation */
    schedInit(APB1ClockRateHz);
    taskCreate(shockpot1000Hz, 1);
    taskCreate(canTxUpdate, 5);
    schedStart();
    
    return 0;
}