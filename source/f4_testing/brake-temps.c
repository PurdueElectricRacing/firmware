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

 // Guard so cmake doesn't compile all tests
 #if (F4_TESTING_CHOSEN == BRAKE_TEMPS)
 
 #include "common/phal_F4_F7/rcc/rcc.h"
 #include "common/phal_F4_F7/gpio/gpio.h"
 #include "common/phal_F4_F7/usart/usart.h"
 #include "common/phal_F4_F7/adc/adc.h"
 #include "common/psched/psched.h"
 #include "common/log/log.h"
 #include "common/common_defs/common_defs.h"
 
typedef struct __attribute__((packed))
{
    // Do not modify this struct unless
    // you modify the ADC DMA config
    // in main.h to match
    uint16_t b1;
    uint16_t b2;

} raw_adc_values_t;

volatile extern raw_adc_values_t raw_adc_values;

void HardFault_Handler();
void ledblink();

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

#define BRK_1_ADC_CHNL 1
#define BRK_2_ADC_CHNL 3

#define BRK_1_GPIO_Port GPIOA
#define BRK_1_Pin 1
#define BRK_2_GPIO_Port GPIOA
#define BRK_2_Pin 3

ADCChannelConfig_t adc_channel_config[] = {
    {.channel = BRK_1_ADC_CHNL,        .rank = 1,  .sampling_time = ADC_CHN_SMP_CYCLES_480},
    {.channel = BRK_2_ADC_CHNL,        .rank = 2,  .sampling_time = ADC_CHN_SMP_CYCLES_480}
};

dma_init_t adc_dma_config = ADC1_DMA_CONT_CONFIG((uint32_t) &raw_adc_values, sizeof(raw_adc_values) / sizeof(raw_adc_values.b1), 0b01);

GPIOInitConfig_t gpio_config[] = {
    GPIO_INIT_OUTPUT(GPIOD, 12, GPIO_OUTPUT_LOW_SPEED), // F407VGT disco LEDs
    GPIO_INIT_OUTPUT(GPIOD, 13, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(GPIOD, 14, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(GPIOD, 15, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_USART2TX_PA2,
    GPIO_INIT_USART2RX_PA3,

    // Brake
    GPIO_INIT_ANALOG(BRK_1_GPIO_Port, BRK_1_Pin),
    GPIO_INIT_ANALOG(BRK_2_GPIO_Port, BRK_2_Pin),
};

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

void save_brake_temps(void)
{
    // save brake temps
    uint16_t cur_temp1 = raw_adc_values.b1;
    uint16_t cur_temp2 = raw_adc_values.b2;
    debug_printf("Temp 1: %d Temp 2: %d\n", cur_temp1, cur_temp2);
}

int main()
{
    if(0 != PHAL_configureClockRates(&clock_config))
    {
        HardFault_Handler();
    }
    if(false == PHAL_initGPIO(gpio_config, sizeof(gpio_config)/sizeof(GPIOInitConfig_t)))
    {
        HardFault_Handler();
    }
    SysTick_Config(SystemCoreClock / 1000);
    NVIC_EnableIRQ(SysTick_IRQn);
    if(false == PHAL_initUSART(&usart_config, APB1ClockRateHz))
    {
        HardFault_Handler();
    }
    log_yellow("PER PER PER\n");
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
    taskCreate(ledblink, 500);
    taskCreate(save_brake_temps, 100);
    schedStart();

    return 0;
}

void ledblink()
{
    PHAL_toggleGPIO(GPIOD, 13);
}

void HardFault_Handler()
{
while(1)
{
    __asm__("nop");
}
}

#endif // F4_TESTING_CHOSEN == BRAKE_TEMPS
