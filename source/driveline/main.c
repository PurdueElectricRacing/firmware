/* System Includes */
#include "stm32l432xx.h"
#include "common/phal_L4/usart/usart.h"
#include "common/phal_L4/gpio/gpio.h"
#include "common/phal_L4/can/can.h"
#include "common/phal_L4/rcc/rcc.h"
#include "common/phal_L4/adc/adc.h"
#include "common/phal_L4/i2c/i2c.h"
#include "common/phal_L4/tim/tim.h"
#include "common/phal_L4/dma/dma.h"
#include "common/psched/psched.h"
#include "common/eeprom/eeprom.h"
#include "common/common_defs/common_defs.h"
#include <math.h>
#include <stdbool.h>

#if (FTR_DRIVELINE_REAR) && (FTR_DRIVELINE_FRONT)
#error "Can not specify both front and rear driveline for the same binary!"
#elif (!FTR_DRIVELINE_REAR) && (!FTR_DRIVELINE_FRONT)
#error "You must define either FTR_DRIVELINE_REAR or FTR_DRIVELINE_FRONT"
#endif

/* Module Includes */
#include "main.h"
#include "can_parse.h"
//#include "wheel_speeds.h"
#include "shockpot.h"
#include "plettenberg.h"

GPIOInitConfig_t gpio_config[] = {
  GPIO_INIT_CANRX_PA11,
  GPIO_INIT_CANTX_PA12,
  // Shock Pots
  GPIO_INIT_ANALOG(POT_AMP_LEFT_GPIO_Port,  POT_AMP_LEFT_Pin),
  GPIO_INIT_ANALOG(POT_AMP_RIGHT_GPIO_Port, POT_AMP_RIGHT_Pin),
  // Motor Controllers
  GPIO_INIT_USART1TX_PA9,
  GPIO_INIT_USART1RX_PA10,
  GPIO_INIT_USART2TX_PA2,
  GPIO_INIT_USART2RX_PA3,
  // Wheel Speed
//   GPIO_INIT_AF(WSPEEDR_GPIO_Port, WSPEEDR_Pin, WHEELSPEEDR_AF, GPIO_OUTPUT_ULTRA_SPEED, GPIO_TYPE_AF, GPIO_INPUT_PULL_UP),
//   GPIO_INIT_AF(WSPEEDL_GPIO_Port, WSPEEDL_Pin, WHEELSPEEDL_AF, GPIO_OUTPUT_ULTRA_SPEED, GPIO_TYPE_AF, GPIO_INPUT_PULL_UP),
  // EEPROM
  GPIO_INIT_OUTPUT(WC_GPIO_Port, WC_Pin, GPIO_OUTPUT_LOW_SPEED),
  GPIO_INIT_I2C1_SCL_PB6,
  GPIO_INIT_I2C1_SDA_PB7,
  // Status LEDs
  GPIO_INIT_OUTPUT(ERR_LED_GPIO_Port, ERR_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
  GPIO_INIT_OUTPUT(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin, GPIO_OUTPUT_LOW_SPEED),
  GPIO_INIT_OUTPUT(CONN_LED_GPIO_Port, CONN_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
};

/* USART Configuration */
// Left Motor Controller
dma_init_t usart_l_tx_dma_config = USART2_TXDMA_CONT_CONFIG(NULL, 1);
dma_init_t usart_l_rx_dma_config = USART2_RXDMA_CONT_CONFIG(NULL, 2);
usart_init_t huart_l = {
    .baud_rate   = 115000,
    .word_length = WORD_8,
    .hw_flow_ctl = HW_DISABLE,
    .mode        = MODE_TX_RX,
    .stop_bits   = SB_ONE,
    .parity      = PT_NONE,
    .obsample    = OB_DISABLE,
    .ovsample    = OV_16,
    .adv_feature.rx_inv    = true,
    .adv_feature.tx_inv    = true,
    .adv_feature.auto_baud = false,
    .adv_feature.data_inv  = false,
    .adv_feature.msb_first = false,
    .adv_feature.overrun   = false,
    .adv_feature.dma_on_rx_err = false,
    .tx_dma_cfg = &usart_l_tx_dma_config,
    .rx_dma_cfg = &usart_l_rx_dma_config
};
// Right Motor Controller
dma_init_t usart_r_tx_dma_config = USART1_TXDMA_CONT_CONFIG(NULL, 1);
dma_init_t usart_r_rx_dma_config = USART1_RXDMA_CONT_CONFIG(NULL, 2);
usart_init_t huart_r = {
    .baud_rate   = 115000,
    .word_length = WORD_8,
    .hw_flow_ctl = HW_DISABLE,
    .mode        = MODE_TX_RX,
    .stop_bits   = SB_ONE,
    .parity      = PT_NONE,
    .obsample    = OB_DISABLE,
    .ovsample    = OV_16,
    .adv_feature.rx_inv    = true,
    .adv_feature.tx_inv    = true,
    .adv_feature.auto_baud = false,
    .adv_feature.data_inv  = false,
    .adv_feature.msb_first = false,
    .adv_feature.overrun   = false,
    .adv_feature.dma_on_rx_err = false,
    .tx_dma_cfg = &usart_r_tx_dma_config,
    .rx_dma_cfg = &usart_r_rx_dma_config
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
    {.channel=POT_AMP_LEFT_ADC_CHNL,  .rank=1, .sampling_time=ADC_CHN_SMP_CYCLES_6_5},
    {.channel=POT_AMP_RIGHT_ADC_CHNL, .rank=2, .sampling_time=ADC_CHN_SMP_CYCLES_6_5},
};
dma_init_t adc_dma_config = ADC1_DMA_CONT_CONFIG((uint32_t) &raw_shock_pots, 
                            sizeof(raw_shock_pots) / sizeof(raw_shock_pots.pot_left), 0b01);

#define TargetCoreClockrateHz 16000000
ClockRateConfig_t clock_config = {
    .system_source          = SYSTEM_CLOCK_SRC_HSI,
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

/* Function Prototypes */
void commandTorquePeriodic();
void parseDataPeriodic();
void canTxUpdate();
void usartTxUpdate();
void usartRxUpdate();
void ledBlink();
void heartBeat();
extern void HardFault_Handler();

char usart_rx_buffs[6][MC_MAX_RX_LENGTH] = {'\0'};
typedef struct {
    char *read;
    char *write;
    char *free;
} usart_rx_circ_buf_t;

usart_rx_circ_buf_t c_rx_usart_l, c_rx_usart_r;
q_handle_t q_tx_can;
q_handle_t q_rx_can;
q_handle_t q_tx_usart_l;
q_handle_t q_tx_usart_r;
motor_t motor_left, motor_right;
// wp
int main(void)
{
    /* Data Struct init */
    qConstruct(&q_tx_can, sizeof(CanMsgTypeDef_t));
    qConstruct(&q_rx_can, sizeof(CanMsgTypeDef_t));
    qConstruct(&q_tx_usart_l, MC_MAX_TX_LENGTH);
    qConstruct(&q_tx_usart_r, MC_MAX_TX_LENGTH);

    c_rx_usart_l = (usart_rx_circ_buf_t) {.read =usart_rx_buffs[0], 
                                          .write=usart_rx_buffs[1], 
                                          .free =usart_rx_buffs[2]};
    c_rx_usart_r = (usart_rx_circ_buf_t) {.read =usart_rx_buffs[3], 
                                          .write=usart_rx_buffs[4], 
                                          .free =usart_rx_buffs[5]};

    /* HAL Initilization */
    if(0 != PHAL_configureClockRates(&clock_config))
    {
        HardFault_Handler();
    }
    if(!PHAL_initGPIO(gpio_config, sizeof(gpio_config)/sizeof(GPIOInitConfig_t)))
    {
        HardFault_Handler();
    }
    if(!PHAL_initUSART(USART_L, &huart_l, APB1ClockRateHz))
    {
        HardFault_Handler();
    }
    if(!PHAL_initUSART(USART_R, &huart_r, APB2ClockRateHz))
    {
        HardFault_Handler();
    }
    if(!PHAL_initCAN(CAN1, false))
    {
        HardFault_Handler();
    }
    NVIC_EnableIRQ(CAN1_RX0_IRQn);
    // if(!PHAL_initPWMIn(TIM1, APB2ClockRateHz / TIM_CLOCK_FREQ, TI1FP1))
    // {
    //     HardFault_Handler();
    // }
    // TODO: configure TIM2
    // if(!PHAL_initPWMChannel(TIM1, CC1, CC_INTERNAL, false))
    // {
    //     HardFault_Handler();
    // }
    if(!PHAL_initADC(ADC1, &adc_config, adc_channel_config, 
       sizeof(adc_channel_config)/sizeof(ADCChannelConfig_t)))
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
    PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);

    /* Module init */
    initCANParse(&q_rx_can);
    // wheelSpeedsInit();

    // Motor Controllers
    // Left
    mc_init(&motor_left,  M_INVERT_LEFT,  &q_tx_usart_l);
    // Right
    mc_init(&motor_right, M_INVERT_RIGHT, &q_tx_usart_r);

    /* Task Creation */
    schedInit(SystemCoreClock);
    taskCreate(ledBlink, 500);
    taskCreate(heartBeat, 100);
    taskCreate(commandTorquePeriodic, 15);
    taskCreate(parseDataPeriodic, 15);
    taskCreate(shockpot1000Hz, 1);

    // TODO: shock is very fast, but contains a bunch of floating point arithmetic
    //taskCreate(shockpot1000Hz, 1);
    //taskCreate(wheelSpeedsPeriodic(), 15);

    taskCreateBackground(canTxUpdate);
    taskCreateBackground(canRxUpdate);
    taskCreateBackground(usartTxUpdate);
    taskCreateBackground(usartRxUpdate);

    // signify end of initialization
    PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);
    schedStart();
    
    return 0;
}

void heartBeat()
{
    #if (FTR_DRIVELINE_FRONT)
    //SEND_FRONT_DRIVELINE_HB(q_tx_can, motor_left.);
    #elif (FTR_DRIVELINE_REAR)
    //SEND_REAR_DRIVELINE_HB(q_tx_can, DRIVELINE_STATE_REAR_OKAY);
    #endif
}
/**
 * @brief Receives torque command from can message
 *        Relays this to the motor controllers
 */
void commandTorquePeriodic()
{
    // TODO: fault checks or whatevs
    #if (FTR_DRIVELINE_FRONT)
    float pow_left  = (float) CLAMP(can_data.torque_request_main.front_left, -4095, 4095);
    float pow_right = (float) CLAMP(can_data.torque_request_main.front_right, -4095, 4095);
    #elif (FTR_DRIVELINE_REAR)
    float pow_left  = (float) CLAMP(can_data.torque_request_main.rear_left, -4095, 4095);
    float pow_right = (float) CLAMP(can_data.torque_request_main.rear_right, -4095, 4095);
    #endif
    pow_left  = pow_left  * 100.0 / 4096.0;
    pow_right = pow_right * 100.0 / 4096.0;

    // Only drive if ready
    if (can_data.main_status.car_state != CAR_STATE_READY2DRIVE || 
        can_data.main_status.stale                              ||
        can_data.torque_request_main.stale) 
    {
        pow_left = 0.0;
        pow_right = 0.0;
    }
    // TODO: remove
    // pow_left = 0.1;
    // pow_right = 0.1;
    if (motor_left.motor_state == MC_CONNECTED &&
        motor_right.motor_state == MC_CONNECTED)
    {
        mc_set_power(pow_left, &motor_left);
        mc_set_power(pow_right, &motor_right);
    }
}

/**
 * @brief Parses motor controller and sensor
 *        info into can messages
 */
uint8_t data_ct = 0;
void parseDataPeriodic()
{
    /* Update Motor Controller Data Structures */
    char *tmp;
    // LEFT
    tmp = c_rx_usart_l.read;
    c_rx_usart_l.read = c_rx_usart_l.free;
    c_rx_usart_l.free = tmp;
    mc_parse(c_rx_usart_l.read, &motor_left);
    // RIGHT
    tmp = c_rx_usart_r.read;
    c_rx_usart_r.read = c_rx_usart_r.free;
    c_rx_usart_r.free = tmp;
    mc_parse(c_rx_usart_r.read, &motor_right);

    // Wait until actual data has been pulled from motor controller
    if (!motor_left.data_valid || !motor_right.data_valid) return;

    // TODO: rpm -> ? currently rpm won't fit in uint16_t based on max rpm
    // TODO: shock pots change from raw
#if (FTR_DRIVELINE_REAR)
    SEND_REAR_WHEEL_DATA(q_tx_can, motor_left.rpm, motor_right.rpm,
                         raw_shock_pots.pot_left, raw_shock_pots.pot_right);
#elif (FTR_DRIVELINE_FRONT)
    SEND_REAR_WHEEL_DATA(q_tx_can, motor_left.rpm, motor_right.rpm,
                         raw_shock_pots.pot_left, raw_shock_pots.pot_right);
#endif

    if (data_ct++ % 8 == 0)
    {
#if (FTR_DRIVELINE_REAR)
    SEND_REAR_MOTOR_CURRENTS_TEMPS(q_tx_can, 
                                   (uint16_t) motor_left.phase_current, 
                                   (uint16_t) motor_right.phase_current,
                                   (uint8_t)  motor_left.motor_temp, 
                                   (uint8_t)  motor_right.motor_temp);
#elif (FTR_DRIVELINE_FRONT)
    SEND_REAR_MOTOR_CURRENTS_TEMPS(q_tx_can, 
                                   (uint16_t) motor_left.phase_current, 
                                   (uint16_t) motor_right.phase_current,
                                   (uint8_t)  motor_left.motor_temp, 
                                   (uint8_t)  motor_right.motor_temp);
#endif
    }

}

void ledBlink()
{
    PHAL_toggleGPIO(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin);
    if ((sched.os_ticks - last_can_rx_time_ms) >= CONN_LED_MS_THRESH)
         PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);
    else PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);
}

uint8_t usart_cmd[MC_MAX_TX_LENGTH] = {'\0'};
void usartTxUpdate()
{
    // LEFT
    if (PHAL_usartTxDmaComplete(&huart_l) && 
        qReceive(&q_tx_usart_l, usart_cmd) == SUCCESS_G)
    {
        PHAL_usartTxDma(USART_L, &huart_l, (uint16_t *) usart_cmd, strlen(usart_cmd));
    }
    // RIGHT
    if (PHAL_usartTxDmaComplete(&huart_r) && 
        qReceive(&q_tx_usart_r, usart_cmd) == SUCCESS_G)
    {
        PHAL_usartTxDma(USART_R, &huart_r, (uint16_t *) usart_cmd, strlen(usart_cmd));
    }
}

uint8_t l_times = 0;
uint8_t r_times = 0;
void usartRxUpdate()
{
    // TODO: handle half received DMA messages
    char *tmp;
    // LEFT
    if (PHAL_usartRxDmaComplete(&huart_l))
    {
        if (l_times < 20)
        {
            l_times++;
            PHAL_usartRxDma(USART_L, &huart_l, 
                            (uint16_t *) c_rx_usart_l.write, 
                            MC_MAX_RX_LENGTH);
        }
        else
        {
            // swap free and write
            tmp = c_rx_usart_l.write;
            c_rx_usart_l.write = c_rx_usart_l.free;
            c_rx_usart_l.free = tmp;
            PHAL_usartRxDma(USART_L, &huart_l, 
                            (uint16_t *) c_rx_usart_l.write, 
                            MC_MAX_RX_LENGTH);
        }
    }
    // RIGHT
    if (PHAL_usartRxDmaComplete(&huart_r))
    {
        if (r_times < 20)
        {
            r_times++;
            PHAL_usartRxDma(USART_R, &huart_r, 
                            (uint16_t *) c_rx_usart_r.write, 
                            MC_MAX_RX_LENGTH);
        }
        else
        {
            // swap free and write
            tmp = c_rx_usart_r.write;
            c_rx_usart_r.write = c_rx_usart_r.free;
            c_rx_usart_r.free = tmp;
            PHAL_usartRxDma(USART_R, &huart_r, 
                            (uint16_t *) c_rx_usart_r.write, 
                            MC_MAX_RX_LENGTH);
        }
    }
}

void canTxUpdate()
{
    CanMsgTypeDef_t tx_msg;
    if (qReceive(&q_tx_can, &tx_msg) == SUCCESS_G)    // Check queue for items and take if there is one
    {
        PHAL_txCANMessage(&tx_msg);
    }
}

void CAN1_RX0_IRQHandler()
{
    if (CAN1->RF0R & CAN_RF0R_FOVR0) // FIFO Overrun
        CAN1->RF0R &= !(CAN_RF0R_FOVR0); 

    if (CAN1->RF0R & CAN_RF0R_FULL0) // FIFO Full
        CAN1->RF0R &= !(CAN_RF0R_FULL0); 

    if (CAN1->RF0R & CAN_RF0R_FMP0_Msk) // Release message pending
    {
        CanMsgTypeDef_t rx;
        rx.Bus = CAN1;

        // Get either StdId or ExtId
        if (CAN_RI0R_IDE & CAN1->sFIFOMailBox[0].RIR)
        { 
          rx.ExtId = ((CAN_RI0R_EXID | CAN_RI0R_STID) & CAN1->sFIFOMailBox[0].RIR) >> CAN_RI0R_EXID_Pos;
        }
        else
        {
          rx.StdId = (CAN_RI0R_STID & CAN1->sFIFOMailBox[0].RIR) >> CAN_TI0R_STID_Pos;
        }

        rx.DLC = (CAN_RDT0R_DLC & CAN1->sFIFOMailBox[0].RDTR) >> CAN_RDT0R_DLC_Pos;

        rx.Data[0] = (uint8_t) (CAN1->sFIFOMailBox[0].RDLR >> 0)  & 0xFF;
        rx.Data[1] = (uint8_t) (CAN1->sFIFOMailBox[0].RDLR >> 8)  & 0xFF;
        rx.Data[2] = (uint8_t) (CAN1->sFIFOMailBox[0].RDLR >> 16) & 0xFF;
        rx.Data[3] = (uint8_t) (CAN1->sFIFOMailBox[0].RDLR >> 24) & 0xFF;
        rx.Data[4] = (uint8_t) (CAN1->sFIFOMailBox[0].RDHR >> 0)  & 0xFF;
        rx.Data[5] = (uint8_t) (CAN1->sFIFOMailBox[0].RDHR >> 8)  & 0xFF;
        rx.Data[6] = (uint8_t) (CAN1->sFIFOMailBox[0].RDHR >> 16) & 0xFF;
        rx.Data[7] = (uint8_t) (CAN1->sFIFOMailBox[0].RDHR >> 24) & 0xFF;

        CAN1->RF0R |= (CAN_RF0R_RFOM0); 

        qSendToBack(&q_rx_can, &rx); // Add to queue (qSendToBack is interrupt safe)
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