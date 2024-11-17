#include "common/faults/faults.h"
#include "common/phal_F4_F7/rcc/rcc.h"
#include "common/phal_F4_F7/gpio/gpio.h"
#include "common/phal_F4_F7/adc/adc.h"
#include "common/phal_F4_F7/dma/dma.h"
#include "common/phal_F4_F7/usart/usart.h"
#include "common/psched/psched.h"

#include "nextion.h"
#include "source/dashboard/lcd/lcd.h"
#include "common/queue/queue.h"
#include "stm32f407xx.h"

#include <stdint.h>
#include <string.h>

#include "main.h"

#include "can_parse.h"

volatile raw_adc_values_t raw_adc_values;

#define POT_MIN 0
#define POT_MAX 4095
#define UP_PIN 7
#define DOWN_PIN 5
#define SELECT_PIN 3

/* ADC Configuration */
ADCInitConfig_t adc_config = {
    .adc_number = 1,
    .clock_prescaler = ADC_CLK_PRESC_6,
    .resolution = ADC_RES_12_BIT,
    .data_align = ADC_DATA_ALIGN_RIGHT,
    .cont_conv_mode = true,
    .dma_mode = ADC_DMA_CIRCULAR
};

ADCChannelConfig_t adc_channel_config[] = {
    {.channel = 0, .rank = 1, .sampling_time = ADC_CHN_SMP_CYCLES_480},
};

dma_init_t adc_dma_config = ADC1_DMA_CONT_CONFIG((uint32_t) &raw_adc_values, sizeof(adc_channel_config)/sizeof(ADCChannelConfig_t), 0b01);

GPIOInitConfig_t gpio_config[] = {
    // Page Knob
    GPIO_INIT_ANALOG(GPIOA, 0),

    // LCD
    GPIO_INIT_USART2TX_PA2,
    GPIO_INIT_USART2RX_PA3,

    // Buttons/Switches
    GPIO_INIT_INPUT(GPIOB, 7, GPIO_INPUT_PULL_UP), // UP
    GPIO_INIT_INPUT(GPIOB, 5, GPIO_INPUT_PULL_UP), // DOWN
    GPIO_INIT_INPUT(GPIOB, 3, GPIO_INPUT_PULL_UP), // SELECT

    GPIO_INIT_OUTPUT(GPIOD, 12, GPIO_OUTPUT_LOW_SPEED), // green LED
    GPIO_INIT_OUTPUT(GPIOD, 13, GPIO_OUTPUT_LOW_SPEED), // orange LED
    GPIO_INIT_OUTPUT(GPIOD, 14, GPIO_OUTPUT_LOW_SPEED), // red LED 
};

// USART Configuration for LCD
dma_init_t usart_tx_dma_config = USART2_TXDMA_CONT_CONFIG(NULL, 1);
dma_init_t usart_rx_dma_config = USART2_RXDMA_CONT_CONFIG(NULL, 2);
usart_init_t lcd = {
   .baud_rate   = 115200,
   .word_length = WORD_8,
   .stop_bits   = SB_ONE,
   .parity      = PT_NONE,
   .hw_flow_ctl = HW_DISABLE,
   .ovsample    = OV_16,
   .obsample    = OB_DISABLE,
   .periph      = USART2,
   .wake_addr   = false,
   .usart_active_num = USART2_ACTIVE_IDX,
   .tx_dma_cfg = &usart_tx_dma_config,
   .rx_dma_cfg = &usart_rx_dma_config
};

#define TargetCoreClockrateHz 16000000
ClockRateConfig_t clock_config = {
    .system_source              =SYSTEM_CLOCK_SRC_HSI,
    .vco_output_rate_target_hz  =160000000,
    .system_clock_target_hz     =TargetCoreClockrateHz,
    .ahb_clock_target_hz        =(TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz       =(TargetCoreClockrateHz / (1)),
    .apb2_clock_target_hz       =(TargetCoreClockrateHz / (1)),
};

extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

void HardFault_Handler();
void usartTxUpdate();
void config_inturrupts();
void handle_inputs();
extern void updatePage();
void emulate_fault();

// Communication queues
q_handle_t q_tx_usart;

struct inputs {
    uint8_t up;
    uint8_t down;
    uint8_t select;
    uint8_t page;
} g_inputs;

lcd_t lcd_data = {
    .encoder_position = 0,
};

int main()
{
    qConstruct(&q_tx_usart, NXT_STR_SIZE);
    
    if (0 != PHAL_configureClockRates(&clock_config))
    {
        HardFault_Handler();
    }
    if (!PHAL_initGPIO(gpio_config, sizeof(gpio_config) / sizeof(GPIOInitConfig_t)))
    {
        HardFault_Handler();
    }
    if (!PHAL_initADC(ADC1, &adc_config, adc_channel_config, sizeof(adc_channel_config)/sizeof(ADCChannelConfig_t)))
    {
        HardFault_Handler();
    }
    if (!PHAL_initDMA(&adc_dma_config))
    {
        HardFault_Handler();
    }
    if (false == PHAL_initUSART(&lcd, APB2ClockRateHz))
    {
        HardFault_Handler();
    }
    PHAL_startTxfer(&adc_dma_config);
    PHAL_startADC(ADC1);

    initFaultLibrary(2, &q_tx_can1_s[0], ID_FAULT_SYNC_DASHBOARD);

    config_inturrupts();

    schedInit(APB1ClockRateHz);
    initLCD();

    taskCreate(handle_inputs, 100);
    taskCreate(updatePage,  500);
    taskCreateBackground(usartTxUpdate);
    taskCreate(updateFaultDisplay, 500);
    taskCreate(updateFaultPageIndicators, 500);
    taskCreate(emulate_fault, 5000);


    schedStart();
    // Never reached
    return 0;
}

extern uint16_t fault_buf[5];
void emulate_fault() {
    // fault_buf[0] = 1;
    // fault_buf[1] = 2;
    // fault_buf[2] = 3;
    // fault_buf[3] = 4;
    // fault_buf[4] = 5;
    setFault(ID_TEST_FAULT_1_FAULT, 2);
    setFault(ID_TEST_FAULT_2_FAULT, 2);
    setFault(ID_TEST_FAULT_3_FAULT, 2);
    setFault(ID_TEST_FAULT_4_FAULT, 124);
}

void handle_inputs() {
    if (g_inputs.up) {
        g_inputs.up = 0;
        moveUp();
    }

    if (g_inputs.down) {
        g_inputs.down = 0;
        moveDown();
    }

    if (g_inputs.select) {
        g_inputs.select = 0;
        selectItem();
    }

    // divide the pot value by 8 pages
    g_inputs.page = raw_adc_values.pot_val / ((POT_MAX + 1) / 8);
    lcd_data.encoder_position = g_inputs.page;
}


// LCD USART Communication
uint8_t cmd[100] = {'\0'};
void usartTxUpdate()
{
    if((false == PHAL_usartTxBusy(&lcd)) &&  (SUCCESS_G == qReceive(&q_tx_usart, cmd)))
    {
        PHAL_usartTxDma(&lcd, (uint16_t *) cmd, strlen(cmd));
    }
}

static volatile uint32_t last_click_time;
void EXTI9_5_IRQHandler() {
    if (EXTI->PR & (0x1<<7)) { // check triggered by PB7
        if (sched.os_ticks - last_click_time > 20) { // simple debounce
            last_click_time = sched.os_ticks;
            g_inputs.up = 1;
            PHAL_toggleGPIO(GPIOD, 12);
        }
        EXTI->PR = (0x1<<7); // clear the pending bit
    }

    if (EXTI->PR & (0x1<<5)) { // check triggered by PB5
        if (sched.os_ticks - last_click_time > 20) { // simple debounce
            last_click_time = sched.os_ticks;
            g_inputs.down = 1;
            PHAL_toggleGPIO(GPIOD, 13);
        }
        EXTI->PR = (0x1<<5); // clear the pending bit
    }
}

void EXTI3_IRQHandler() {
    if (EXTI->PR & (0x1<<3)) { // check triggered by PB3
        if (sched.os_ticks - last_click_time > 20) { // simple debounce
            last_click_time = sched.os_ticks;
            g_inputs.select = 1;
            PHAL_toggleGPIO(GPIOD, 14);
        }
        EXTI->PR = (0x1<<3); // clear the pending bit
    }
}

void config_inturrupts() {
    // Enable the SYSCFG clock for interrupts
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    /* Configure UP (GPIO B7) */
    SYSCFG->EXTICR[1] &= ~(0b1111 << 12); // Clear bits 12-15 in EXTICR2 to reset EXTI7 source
    SYSCFG->EXTICR[1] |= (0b0001 << 12);  // Set bits 12-15 to 0001 to select Port B for EXTI7

    EXTI->IMR |= (0x1 << 7); // Enable interrupt on EXTI line 7 by setting bit 7 in IMR
    EXTI->RTSR |= (0x1 << 7); // Enable falling edge trigger on line 7
    EXTI->FTSR &= ~(0x1 << 7); // Clear falling edge trigger on line 7

    /* Configure DOWN (GPIO B5) */
    SYSCFG->EXTICR[1] &= ~(0b1111 << 4);
    SYSCFG->EXTICR[1] |= (0b0001 << 4);

    EXTI->IMR |= (0x1 << 5);
    EXTI->RTSR |= (0x1 << 5);
    EXTI->FTSR &= ~(0x1 << 5);

    /* Configure SELECT (GPIO B3) */
    SYSCFG->EXTICR[0] &= ~(0b1111 << 12);
    SYSCFG->EXTICR[0] |= (0b0001 << 12); 

    EXTI->IMR |= (0x1 << 3);
    EXTI->RTSR |= (0x1 << 3);
    EXTI->FTSR &= ~(0x1 << 3);

    NVIC_EnableIRQ(EXTI9_5_IRQn);
    NVIC_EnableIRQ(EXTI3_IRQn);
}


void HardFault_Handler()
{
    while(1)
    {
        __asm__("nop");
    }
}