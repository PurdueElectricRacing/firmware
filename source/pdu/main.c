/* System Includes */
#include "common/phal_F4_F7/gpio/gpio.h"
#include "common/phal_F4_F7/rcc/rcc.h"
#include "common/psched/psched.h"

/* Module Includes */
#include "main.h"

GPIOInitConfig_t gpio_config[] = {
    // Status Indicators
    GPIO_INIT_OUTPUT(ERR_LED_GPIO_Port, ERR_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(CONN_LED_GPIO_Port, CONN_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin, GPIO_OUTPUT_LOW_SPEED),
    // CAN
    GPIO_INIT_CANRX_PD0,
    GPIO_INIT_CANTX_PD1,
    // EEPROM
    GPIO_INIT_SPI2_SCK_PB13,
    GPIO_INIT_SPI2_MISO_PB14,
    GPIO_INIT_SPI2_MOSI_PB15,
    GPIO_INIT_OUTPUT(EEPROM_nWP_GPIO_Port, EEPROM_nWP_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(EEPROM_NSS_GPIO_Port, EEPROM_NSS_Pin, GPIO_OUTPUT_LOW_SPEED),
    // LED CTRL
    GPIO_INIT_SPI1_SCK_PB3,
    GPIO_INIT_SPI1_MOSI_PB5,
    GPIO_INIT_OUTPUT(LED_CTRL_LAT_GPIO_Port, LED_CTRL_LAT_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_OUTPUT(LED_CTRL_BLANK_GPIO_Port, LED_CTRL_BLANK_Pin, GPIO_OUTPUT_LOW_SPEED),
    // Flow Rate
    GPIO_INIT_AF(FLOW_RATE_1_GPIO_Port, FLOW_RATE_1_Pin, FLOW_RATE_1_AF, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_PULL_DOWN),
    GPIO_INIT_AF(FLOW_RATE_2_GPIO_Port, FLOW_RATE_2_Pin, FLOW_RATE_2_AF, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_PULL_DOWN),
    // Fan Control
    GPIO_INIT_AF(FAN_1_PWM_GPIO_Port, FAN_1_PWM_Pin, FAN_1_PWM_AF, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_AF(FAN_2_PWM_GPIO_Port, FAN_2_PWM_Pin, FAN_2_PWM_AF, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_PUSH_PULL, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_AF(FAN_1_TACH_GPIO_Port, FAN_1_TACH_Pin, FAN_1_TACH_AF, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_PULL_DOWN),
    GPIO_INIT_AF(FAN_2_TACH_GPIO_Port, FAN_2_TACH_Pin, FAN_2_TACH_AF, GPIO_OUTPUT_HIGH_SPEED, GPIO_OUTPUT_OPEN_DRAIN, GPIO_INPUT_PULL_DOWN),
    // Pump Switches
    GPIO_INIT_OUTPUT(PUMP_1_CTRL_GPIO_Port, PUMP_1_CTRL_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_ANALOG(PUMP_1_IMON_GPIO_Port, PUMP_1_IMON_Pin),
    GPIO_INIT_OUTPUT(PUMP_2_CTRL_GPIO_Port, PUMP_2_CTRL_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_ANALOG(PUMP_2_IMON_GPIO_Port, PUMP_2_IMON_Pin),
    // Auxiliary Switch
    GPIO_INIT_OUTPUT(AUX_HP_CTRL_GPIO_Port, AUX_HP_CTRL_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_ANALOG(AUX_HP_IMON_GPIO_Port, AUX_HP_IMON_Pin),
    // SDC Switch
    GPIO_INIT_OUTPUT(SDC_CTRL_GPIO_Port, SDC_CTRL_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_ANALOG(SDC_IMON_GPIO_Port, SDC_IMON_Pin),
    // Fan Switches
    GPIO_INIT_OUTPUT(FAN_1_CTRL_GPIO_Port, FAN_1_CTRL_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_INPUT(FAN_1_NFLT_GPIO_Port, FAN_1_NFLT_Pin, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_ANALOG(FAN_1_CS_GPIO_Port, FAN_1_CS_Pin),
    GPIO_INIT_OUTPUT(FAN_2_CTRL_GPIO_Port, FAN_2_CTRL_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_INPUT(FAN_2_NFLT_GPIO_Port, FAN_2_NFLT_Pin, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_ANALOG(FAN_2_CS_GPIO_Port, FAN_2_CS_Pin),
    // Main Module
    GPIO_INIT_OUTPUT(MAIN_CTRL_GPIO_Port, MAIN_CTRL_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_INPUT(MAIN_NFLT_GPIO_Port, MAIN_NFLT_Pin, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_ANALOG(MAIN_CS_GPIO_Port, MAIN_CS_Pin),
    // Dashboard
    GPIO_INIT_OUTPUT(DASH_CTRL_GPIO_Port, DASH_CTRL_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_INPUT(DASH_NFLT_GPIO_Port, DASH_NFLT_Pin, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_ANALOG(DASH_CS_GPIO_Port, DASH_CS_Pin),
    // ABox
    GPIO_INIT_OUTPUT(ABOX_CTRL_GPIO_Port, ABOX_CTRL_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_INPUT(ABOX_NFLT_GPIO_Port, ABOX_NFLT_Pin, GPIO_INPUT_OPEN_DRAIN),
    GPIO_INIT_ANALOG(ABOX_CS_GPIO_Port, ABOX_CS_Pin),
    // Bullet
    GPIO_INIT_OUTPUT(BLT_CTRL_GPIO_Port, BLT_CTRL_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_INPUT(BLT_NFLT_GPIO_Port, BLT_NFLT_Pin, GPIO_INPUT_OPEN_DRAIN),
    // 5V Critical Switch
    GPIO_INIT_OUTPUT(CRIT_5V_CTRL_GPIO_Port, CRIT_5V_CTRL_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_INPUT(CRIT_5V_NFLT_GPIO_Port, CRIT_5V_NFLT_Pin, GPIO_INPUT_OPEN_DRAIN),
    // 5V Non-Critical Switch
    GPIO_INIT_OUTPUT(NCRIT_5V_CTRL_GPIO_Port, NCRIT_5V_CTRL_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_INPUT(NCRIT_5V_NFLT_GPIO_Port, NCRIT_5V_NFLT_Pin, GPIO_INPUT_OPEN_DRAIN),
    // DAQ
    GPIO_INIT_OUTPUT(DAQ_CTRL_GPIO_Port, DAQ_CTRL_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_INPUT(DAQ_NFLT_GPIO_Port, DAQ_NFLT_Pin, GPIO_INPUT_OPEN_DRAIN),
    // 5V Fan
    GPIO_INIT_OUTPUT(FAN_5V_CTRL_GPIO_Port, FAN_5V_CTRL_Pin, GPIO_OUTPUT_LOW_SPEED),
    GPIO_INIT_INPUT(FAN_5V_NFLT_GPIO_Port, FAN_5V_NFLT_Pin, GPIO_INPUT_OPEN_DRAIN),
    // LV Battery BMS
    GPIO_INIT_INPUT(LV_BMS_STAT_GPIO_Port, LV_BMS_STAT_Pin, GPIO_INPUT_PULL_DOWN),
    GPIO_INIT_USART3TX_PC10,
    GPIO_INIT_USART3RX_PC11,
    // LV Status
    GPIO_INIT_ANALOG(LV_24V_V_SENSE_GPIO_Port, LV_24V_V_SENSE_Pin),
    GPIO_INIT_ANALOG(LV_24V_I_SENSE_GPIO_Port, LV_24V_I_SENSE_Pin),
    GPIO_INIT_ANALOG(LV_5V_V_SENSE_GPIO_Port, LV_5V_V_SENSE_Pin),
    GPIO_INIT_ANALOG(LV_5V_I_SENSE_GPIO_Port, LV_5V_I_SENSE_Pin),
    GPIO_INIT_ANALOG(LV_3V3_V_SENSE_GPIO_Port, LV_3V3_V_SENSE_Pin),
    GPIO_INIT_ANALOG(EXTERNAL_THERM_GPIO_Port, EXTERNAL_THERM_Pin),
};

/* ADC Configuration */
ADCInitConfig_t adc_config = {
    .clock_prescaler = ADC_CLK_PRESC_16,
    .resolution      = ADC_RES_12_BIT,
    .data_align      = ADC_DATA_ALIGN_RIGHT,
    .cont_conv_mode  = true,
    .overrun         = true,
    .dma_mode        = ADC_DMA_CIRCULAR
};

/* With 11 items, 16 prescaler, and 640 sample time, each channel gets read every 1.4ms */
volatile ADCReadings_t adc_readings;
ADCChannelConfig_t adc_channel_config[] = {
    {.channel=PUMP_1_IMON_ADC_CHNL,    .rank=1,  .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
    {.channel=PUMP_2_IMON_ADC_CHNL,    .rank=2,  .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
    {.channel=AUX_HP_IMON_ADC_CHNL,    .rank=3,  .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
    {.channel=SDC_IMON_ADC_CHNL,       .rank=4,  .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
    {.channel=FAN_1_CS_ADC_CHNL,       .rank=5,  .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
    {.channel=FAN_2_CS_ADC_CHNL,       .rank=6,  .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
    {.channel=MAIN_CS_ADC_CHNL,        .rank=7,  .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
    {.channel=DASH_CS_ADC_CHNL,        .rank=8,  .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
    {.channel=ABOX_CS_ADC_CHNL,        .rank=9,  .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
    {.channel=LV_24V_V_SENSE_ADC_CHNL, .rank=10, .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
    {.channel=LV_24V_I_SENSE_ADC_CHNL, .rank=11, .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
    {.channel=LV_5V_V_SENSE_ADC_CHNL,  .rank=12, .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
    {.channel=LV_5V_I_SENSE_ADC_CHNL,  .rank=13, .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
    {.channel=LV_3V3_V_SENSE_ADC_CHNL, .rank=14, .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
    {.channel=EXTERNAL_THERM_ADC_CHNL, .rank=15, .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
    {.channel=INTERNAL_THERM_ADC_CHNL, .rank=16, .sampling_time=ADC_CHN_SMP_CYCLES_640_5},
};
dma_init_t adc_dma_config = ADC1_DMA_CONT_CONFIG((uint32_t) &adc_readings,
            sizeof(adc_readings) / sizeof(adc_readings.lv_3v3_v_sense), 0b01);

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
void heatBeatLED();

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

    /* Task Creation */
    schedInit(APB1ClockRateHz);
    /* Schedule Periodic tasks here */
    taskCreate(heatBeatLED, 500);
    schedStart();
    return 0;
}

void heatBeatLED()
{
    PHAL_toggleGPIO(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin);
}

void HardFault_Handler()
{
    while(1)
    {
        __asm__("nop");
    }
}