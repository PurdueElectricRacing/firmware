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
#include "common/common_defs/common_defs.h"
#include "common/bootloader/bootloader_common.h"
#include <math.h>
#include <stdbool.h>


#if (FTR_DRIVELINE_REAR) && (FTR_DRIVELINE_FRONT)
#error "Can not specify both front and rear driveline for the same binary!"
#elif (!FTR_DRIVELINE_REAR) && (!FTR_DRIVELINE_FRONT)
#error "You must define either FTR_DRIVELINE_REAR or FTR_DRIVELINE_FRONT"
#endif

/* Module Includes */
// #include "daq.h"
#include "main.h"
#include "can_parse.h"
#include "common/modules/wheel_speeds/wheel_speeds.h"
#include "shockpot.h"
#include "plettenberg.h"
#include "testbench.h"
#include "MC_PL0/MC_PL0.h"
#include "MC_PL_pp/MC_PL_pp.h"

#include "common/faults/faults.h"

GPIOInitConfig_t gpio_config[] = {
  GPIO_INIT_CANRX_PA11,
  GPIO_INIT_CANTX_PA12,
  // Shock Pots
//   GPIO_INIT_ANALOG(POT_AMP_LEFT_GPIO_Port,  POT_AMP_LEFT_Pin),
//   GPIO_INIT_ANALOG(POT_AMP_RIGHT_GPIO_Port, POT_AMP_RIGHT_Pin),
  // Motor Controllers
  GPIO_INIT_USART1TX_PA9,
  GPIO_INIT_USART1RX_PA10,
  GPIO_INIT_USART2TX_PA2,
  GPIO_INIT_USART2RX_PA15,
  //GPIO_INIT_USART2RX_PA3,
  // Wheel Speed
//   GPIO_INIT_AF(WSPEEDR_GPIO_Port, WSPEEDR_Pin, WHEELSPEEDR_AF, GPIO_OUTPUT_ULTRA_SPEED, GPIO_TYPE_AF, GPIO_INPUT_PULL_UP),
//   GPIO_INIT_AF(WSPEEDL_GPIO_Port, WSPEEDL_Pin, WHEELSPEEDL_AF, GPIO_OUTPUT_ULTRA_SPEED, GPIO_TYPE_AF, GPIO_INPUT_PULL_UP),
  GPIO_INIT_AF(GPIOA, 0, 1, GPIO_OUTPUT_ULTRA_SPEED, GPIO_TYPE_AF, GPIO_INPUT_OPEN_DRAIN),
  GPIO_INIT_AF(GPIOA, 1, 1, GPIO_OUTPUT_ULTRA_SPEED, GPIO_TYPE_AF, GPIO_INPUT_OPEN_DRAIN),
  // EEPROM
  GPIO_INIT_OUTPUT(WC_GPIO_Port, WC_Pin, GPIO_OUTPUT_LOW_SPEED),
//   GPIO_INIT_I2C1_SCL_PB6,
//   GPIO_INIT_I2C1_SDA_PB7,
  // Status LEDs
  GPIO_INIT_OUTPUT(ERR_LED_GPIO_Port, ERR_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
  GPIO_INIT_OUTPUT(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin, GPIO_OUTPUT_LOW_SPEED),
  GPIO_INIT_OUTPUT(CONN_LED_GPIO_Port, CONN_LED_Pin, GPIO_OUTPUT_LOW_SPEED),
};

/* USART Configuration */
// Left Motor Controller
dma_init_t usart_l_tx_dma_config = USART1_TXDMA_CONT_CONFIG(NULL, 1);
dma_init_t usart_l_rx_dma_config = USART1_RXDMA_CONT_CONFIG(NULL, 2);
usart_init_t huart_l = {
    .baud_rate   = 115200,
    .word_length = WORD_8,
    .hw_flow_ctl = HW_DISABLE,
    .mode        = MODE_TX_RX,
    .stop_bits   = SB_ONE,
    .parity      = PT_NONE,
    .obsample    = OB_DISABLE,
    .ovsample    = OV_16,
    .adv_feature.rx_inv    = false,
    .adv_feature.tx_inv    = false,
    .adv_feature.auto_baud = false,
    .adv_feature.data_inv  = false,
    .adv_feature.msb_first = false,
    .adv_feature.overrun   = false,
    .adv_feature.dma_on_rx_err = false,
    .tx_dma_cfg = &usart_l_tx_dma_config,
    .rx_dma_cfg = &usart_l_rx_dma_config
};
// Right Motor Controller
dma_init_t usart_r_tx_dma_config = USART2_TXDMA_CONT_CONFIG(NULL, 1);
dma_init_t usart_r_rx_dma_config = USART2_RXDMA_CONT_CONFIG(NULL, 2);
usart_init_t huart_r = {
    .baud_rate   = 115200,
    .word_length = WORD_8,
    .hw_flow_ctl = HW_DISABLE,
    .mode        = MODE_TX_RX,
    .stop_bits   = SB_ONE,
    .parity      = PT_NONE,
    .obsample    = OB_DISABLE,
    .ovsample    = OV_16,
    .adv_feature.rx_inv    = false,
    .adv_feature.tx_inv    = false,
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

WheelSpeed_t left_wheel =  {.tim=TIM2, .invert=true};
WheelSpeed_t right_wheel = {.tim=TIM2, .invert=true};
// TODO: test invert
WheelSpeeds_t wheel_speeds = {.l=&left_wheel, .r=&right_wheel};

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

uint8_t i;

/* Function Prototypes */
void preflightAnimation(void);
void preflightChecks(void);
void commandTorquePeriodic();
void parseDataPeriodic();
void canTxUpdate();
void usartTxUpdate();
void usartRxUpdate();
void usart1IdleIRQ(motor_t *m, usart_init_t *huart);
void usart2IdleIRQ(micro_t *m, usart_init_t *huart);
void ledUpdate();
void linkDAQVars();
void heartBeat();
extern void HardFault_Handler();

q_handle_t q_tx_can;
q_handle_t q_rx_can;
q_handle_t q_tx_usart_l;
q_handle_t q_tx_usart_r;
motor_t motor_left, motor_right;
micro_t micro;

static ExtU rtU;                       /* External inputs */
static ExtY rtY;                       /* External outputs */

/* TV Definitions */
static RT_MODEL rtM_;
static RT_MODEL *const rtMPtr = &rtM_; /* Real-time model */                       /* Observable states */
static RT_MODEL *const rtM = rtMPtr;
static DW rtDW;                        /* Observable states */

uint32_t N = 0;
float k_avg = 0.1;

int main(void){
    i = 0;
    /* Data Struct init */
    qConstruct(&q_tx_can, sizeof(CanMsgTypeDef_t));
    qConstruct(&q_rx_can, sizeof(CanMsgTypeDef_t));
    qConstruct(&q_tx_usart_l, MC_MAX_TX_LENGTH);
    qConstruct(&q_tx_usart_r, MC_MAX_TX_LENGTH);

    /* HAL Initilization */
    if(0 != PHAL_configureClockRates(&clock_config))
    {
        HardFault_Handler();
    }
    if(!PHAL_initGPIO(gpio_config, sizeof(gpio_config)/sizeof(GPIOInitConfig_t)))
    {
        HardFault_Handler();
    }
    initFaultLibrary(FAULT_NODE_NAME, &q_tx_can, ID_FAULT_SYNC_DRIVELINE);

    /* Task Creation */
    schedInit(SystemCoreClock);
    configureAnim(preflightAnimation, preflightChecks, 50, 750);
    taskCreate(ledUpdate, 500);
    //FL Stuff
    taskCreate(heartBeatTask, 100);
    taskCreate(updateFaults, 5);
    //FL Stuff
    taskCreate(heartBeat, 100);
    taskCreate(commandTorquePeriodic, 15);
    taskCreate(parseDataPeriodic, MC_LOOP_DT);
    // TODO: taskCreate(shockpot1000Hz, 5);
    taskCreate(wheelSpeedsPeriodic, 15);
    //taskCreateBackground(canTxUpdate);
    //taskCreateBackground(canRxUpdate);
    taskCreateBackground(usartTxUpdate);

    // signify end of initialization
    PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);
    schedStart();

    return 0;
}

void preflightChecks(void) {
    static uint8_t state;

    switch (state++)
    {
        case 0:
            huart_l.rx_dma_cfg->circular = true;
            if(!PHAL_initUSART(USART_L, &huart_l, APB1ClockRateHz))
            {
                HardFault_Handler();
            }
            huart_r.rx_dma_cfg->circular = true;
            if(!PHAL_initUSART(USART_R, &huart_r, APB2ClockRateHz))
            {
                HardFault_Handler();
            }
            break;
        case 1:
            //if(!PHAL_initCAN(CAN1, false))
            //{
            //    HardFault_Handler();
            //}
            //NVIC_EnableIRQ(CAN1_RX0_IRQn);
            break;
        case 2:
            // if(!PHAL_initPWMIn(TIM1, APB2ClockRateHz / TIM_CLOCK_FREQ, TI1FP1))
            // {
            //     HardFault_Handler();
            // }
            // if(!PHAL_initPWMChannel(TIM1, CC1, CC_INTERNAL, false))
            // {
            //     HardFault_Handler();
            // }
            // if(!PHAL_initPWMIn(TIM2, APB1ClockRateHz / TIM_CLOCK_FREQ, TI1FP1))
            // {
            //     HardFault_Handler();
            // }
            // if(!PHAL_initPWMChannel(TIM2, CC1, CC_INTERNAL, false))
            // {
            //     HardFault_Handler();
            // }
            break;
        case 3:
            //if(!PHAL_initADC(ADC1, &adc_config, adc_channel_config, 
            //sizeof(adc_channel_config)/sizeof(ADCChannelConfig_t)))
            //{
            //    HardFault_Handler();
            //}
            //if(!PHAL_initDMA(&adc_dma_config))
            //{
            //    HardFault_Handler();
            //}
            //PHAL_startTxfer(&adc_dma_config);
            //PHAL_startADC(ADC1);
            break;
       case 4:
            /* Module init */
            //initCANParse(&q_rx_can);
            // wheelSpeedsInit();
            wheelSpeedsInit(&wheel_speeds);
            rtM->dwork = &rtDW;
            MC_PL0_initialize(rtM);
            break;
        case 5:
            // Left MC
            mcInit(&motor_left,  M_INVERT_LEFT,  &q_tx_usart_l);
            USART_L->CR1 &= ~(USART_CR1_RXNEIE | USART_CR1_TCIE | USART_CR1_TXEIE);
            NVIC_EnableIRQ(USART1_IRQn);
            // initial rx request
            PHAL_usartRxDma(USART_L, &huart_l,
                            (uint16_t *) motor_left.rx_buf,
                            MC_MAX_RX_LENGTH);
            break;
        case 6:
            // Micro Controller
            tiInit(&micro, &q_tx_usart_r);
            USART_R->CR1 &= ~(USART_CR1_RXNEIE | USART_CR1_TCIE | USART_CR1_TXEIE);
            NVIC_EnableIRQ(USART2_IRQn);
            // initial rx request
            PHAL_usartRxDma(USART_R, &huart_r, 
                            (uint16_t *) micro.rx_buf, 
                            TI_MAX_RX_LENGTH);
            break;
        default:
            registerPreflightComplete(1);
            state = 255; // prevent wrap around
    }
}

void preflightAnimation(void) {
    static uint32_t time;

    PHAL_writeGPIO(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin, 0);
    PHAL_writeGPIO(ERR_LED_GPIO_Port, ERR_LED_Pin, 0);
    PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);

    switch (time++ % 2)
    {
        case 0:
            PHAL_writeGPIO(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin, 1);
            break;
        case 1:
            PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);
            break;
    }
}

void heartBeat()
{
    #if (FTR_DRIVELINE_FRONT)
    SEND_FRONT_DRIVELINE_HB(q_tx_can, motor_left.motor_state,
                                      motor_left.link_state,
                                      motor_left.last_link_error,
                                      motor_right.motor_state,
                                      motor_right.link_state,
                                      motor_right.last_link_error);
    #elif (FTR_DRIVELINE_REAR)
    SEND_REAR_DRIVELINE_HB(q_tx_can, motor_left.motor_state,
                                     motor_left.link_state,
                                     motor_left.last_link_error,
                                     motor_right.motor_state,
                                     motor_right.link_state,
                                     motor_right.last_link_error);
    #endif
}

/**
 * @brief Receives torque command from can message
 *        Relays this to the motor controllers
 */
void commandTorquePeriodic()
{
    //#if (FTR_DRIVELINE_FRONT)
    //float pow_left  = (float) CLAMP(micro.Tx_in[0], -4095, 4095);
    //float pow_right = (float) CLAMP(micro.Tx_in[1], -4095, 4095);
    //#elif (FTR_DRIVELINE_REAR)
    //float pow_left  = (float) CLAMP(micro.Tx_in[2], -4095, 4095);
    //float pow_right = (float) CLAMP(micro.Tx_in[3], -4095, 4095);
    //#endif

    float pow_left;
    float pow_right;
    volatile uint32_t time1 = 0;
    volatile uint32_t time2 = 0;

    mcPeriodic(&motor_left);
    //mcPeriodic(&motor_right);
    tiPeriodic(&micro);

    // pow_left = (float) mot_left_req;
    // pow_right = (float) mot_right_req;

    // Power Limiting
    time1 = sched.os_ticks;
    MC_PL_pp(&rtU, &motor_left, &micro);
    //rt_OneStep(rtM);
    //pow_left = rtY.k[2] * 100.0 / 4095.0;
    //pow_right = rtY.k[3] * 100.0 / 4095.0;

    // No Power Limiting
    pow_left = rtU.Tx[2] * 100.0 / 25.0;
    pow_right = rtU.Tx[3] * 100.0 / 25.0;

    // Prevent regenerative braking, functionality not yet implemented
    if (pow_left < 0)  pow_left  = 0.0;
    if (pow_right < 0) pow_right = 0.0;

    //if (pow_left > 0)
    //{
    //    k_avg = (k_avg*N + pow_left)/(N+1);
    //    N = N + 1;
    //}

    // Only drive if ready
    //if (can_data.main_hb.car_state != CAR_STATE_READY2DRIVE || 
        // TODO: fix stale checks can_data.main_hb.stale    ||
        // can_data.torque_request_main.stale               ||
    //    motor_left.motor_state  != MC_CONNECTED             ||
    //    motor_right.motor_state != MC_CONNECTED) 
    //{
    //    pow_left  = 0.0;
    //    pow_right = 0.0;
    //}
    // Continuously send, despite connection state
    // Ensures that if we lost connection we continue
    // to send 0 to stop motor
    mcSetPower(pow_left,  &motor_left);
    tiSetParam(pow_left, &motor_left, &micro, &rtU, &wheel_speeds);
    time2 = time1 - sched.os_ticks;

    //mcSetPower(0, &motor_right);
    //#if (FTR_DRIVELINE_REAR)
    //SEND_REAR_MC_REQ(q_tx_can, pow_left, pow_right);
    //SEND_REAR_POW_LIM_L(q_tx_can, rtY.T[2], rtY.P_g[2]);
    //#endif

}

/**
 * @brief Parses motor controller and sensor
 *        info into can messages, updates
 *        motor controller connection status
 */
void parseDataPeriodic()
{
    //uint16_t shock_l, shock_r;

    /* Update Data Structures */
    //mcPeriodic(&motor_left);
    //mcPeriodic(&motor_right);
    //tiPeriodic(&micro);

    // Only send once both controllers have updated data
    // if (motor_right.data_stale ||
    //     motor_left.data_stale) return;

    // Extract raw shocks from DMA buffer
    //shock_l = raw_shock_pots.pot_left;
    //shock_r = raw_shock_pots.pot_right;
    // Scale from raw 12bit adc to mm * 10 of linear pot travel
    shock_l = (POT_VOLT_MIN_DIST_MM * 10 - ((uint32_t) shock_l) * (POT_VOLT_MIN_DIST_MM - POT_VOLT_MAX_DIST_MM) * 10 / 4095);
    shock_r = (POT_VOLT_MIN_DIST_MM * 10 - ((uint32_t) shock_r) * (POT_VOLT_MIN_DIST_MM - POT_VOLT_MAX_DIST_MM) * 10 / 4095);


#if (FTR_DRIVELINE_REAR)
    SEND_REAR_WHEEL_DATA(q_tx_can, wheel_speeds.left_kph_x100, wheel_speeds.right_kph_x100,
                         shock_l, shock_r);
#elif (FTR_DRIVELINE_FRONT)
    SEND_FRONT_WHEEL_DATA(q_tx_can, wheel_speeds.left_kph_x100, wheel_speeds.right_kph_x100,
                         shock_l, shock_r);
#endif

#if (FTR_DRIVELINE_REAR)
    SEND_REAR_MOTOR_CURRENTS_TEMPS(q_tx_can,
                                   (uint16_t) motor_left.current_x10,
                                   (uint16_t) motor_right.current_x10,
                                   (uint8_t)  motor_left.motor_temp,
                                   (uint8_t)  motor_right.motor_temp,
                                   (uint16_t)  motor_right.voltage_x10);
    SEND_REAR_CONTROLLER_TEMPS(q_tx_can,
                               (uint8_t) motor_left.controller_temp,
                               (uint8_t) motor_right.controller_temp);
#elif (FTR_DRIVELINE_FRONT)
    SEND_FRONT_MOTOR_CURRENTS_TEMPS(q_tx_can,
                                   (uint16_t) motor_left.current_x10,
                                   (uint16_t) motor_right.current_x10,
                                   (uint8_t)  motor_left.motor_temp,
                                   (uint8_t)  motor_right.motor_temp,
                                   (uint16_t)  motor_right.voltage_x10);
#endif
}

void ledUpdate()
{
    PHAL_toggleGPIO(HEARTBEAT_GPIO_Port, HEARTBEAT_Pin);
    // if ((sched.os_ticks - last_can_rx_time_ms) >= CONN_LED_MS_THRESH)
    //      PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);
    // else PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);

    if (i++ == 6) {
        setFault(ID_WLSPD_L_FAULT, 6000);
        PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 1);
    }

    if (i == 13) {
        setFault(ID_WLSPD_L_FAULT, 2000);
        PHAL_writeGPIO(CONN_LED_GPIO_Port, CONN_LED_Pin, 0);
        i = 0;
    }
}

/* USART Message Handling */
uint8_t tmp_left[MC_MAX_TX_LENGTH] = {'\0'};
uint8_t tmp_right[TI_MAX_TX_LENGTH] = {'\0'};
void usartTxUpdate()
{
    if (PHAL_usartTxDmaComplete(&huart_l) &&
        qReceive(&q_tx_usart_l, tmp_left) == SUCCESS_G)
    {
        PHAL_usartTxDma(USART_L, &huart_l, (uint16_t *) tmp_left, strlen(tmp_left));
    }
    if (PHAL_usartTxDmaComplete(&huart_r) &&
        qReceive(&q_tx_usart_r, tmp_right) == SUCCESS_G)
    {
        PHAL_usartTxDma(USART_R, &huart_r, (uint16_t *) tmp_right, strlen(tmp_right));
    }
}

void USART1_IRQHandler(void) {
    if (USART_L->ISR & USART_ISR_IDLE) {
        usart1IdleIRQ(&motor_left, &huart_l);
        USART_L->ICR = USART_ICR_IDLECF;
    }
}

void USART2_IRQHandler(void) {
    if (USART_R->ISR & USART_ISR_IDLE) {
        usart2IdleIRQ(&micro, &huart_r);
        USART_R->ICR = USART_ICR_IDLECF;
    }
}

void usart1IdleIRQ(motor_t *m, usart_init_t *huart)
{
    uint16_t new_loc = 0;
    m->last_rx_time = sched.os_ticks;
    new_loc = MC_MAX_RX_LENGTH - huart->rx_dma_cfg->channel->CNDTR; // extract last location from DMA
    if (new_loc == MC_MAX_RX_LENGTH) new_loc = 0;                   // should never happen
    else if (new_loc < m->last_rx_loc) new_loc += MC_MAX_RX_LENGTH; // wrap around
    if (new_loc - m->last_rx_loc > MC_MAX_TX_LENGTH)                // status msg vs just an echo
    {
        m->last_msg_time = sched.os_ticks;
        m->last_msg_loc = (m->last_rx_loc + 1) % MC_MAX_RX_LENGTH;
    }
    m->last_rx_loc = new_loc % MC_MAX_RX_LENGTH;
}

void usart2IdleIRQ(micro_t *m, usart_init_t *huart)
{
    uint16_t new_loc = 0;
    m->last_rx_time = sched.os_ticks;
    new_loc = TI_MAX_RX_LENGTH - huart->rx_dma_cfg->channel->CNDTR; // extract last location from DMA
    if (new_loc == TI_MAX_RX_LENGTH) new_loc = 0;                   // should never happen
    else if (new_loc < m->last_rx_loc) new_loc += TI_MAX_RX_LENGTH; // wrap around
    if (new_loc - m->last_rx_loc > TI_MAX_TX_LENGTH)                // status msg vs just an echo
    {
        m->last_msg_time = sched.os_ticks;
        m->last_msg_loc = (m->last_rx_loc + 1) % TI_MAX_RX_LENGTH;
    }
    m->last_rx_loc = new_loc % TI_MAX_RX_LENGTH;
}

/* CAN Message Handling */
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

void driveline_front_bl_cmd_CALLBACK(CanParsedData_t *msg_data_a)
{
    if (can_data.driveline_front_bl_cmd.cmd == BLCMD_RST)
        Bootloader_ResetForFirmwareDownload();
}

void driveline_rear_bl_cmd_CALLBACK(CanParsedData_t *msg_data_a)
{
    if (can_data.driveline_rear_bl_cmd.cmd == BLCMD_RST)
        Bootloader_ResetForFirmwareDownload();
}

void HardFault_Handler()
{
    PHAL_writeGPIO(ERR_LED_GPIO_Port, ERR_LED_Pin, 1);
    while(1)
    {
        __asm__("nop");
    }
}

void rt_OneStep(RT_MODEL *const rtM)
{
  static boolean_T OverrunFlag = false;

  /* Disable interrupts here */

  /* Check for overrun */
  if (OverrunFlag) {
    rtmSetErrorStatus(rtM, "Overrun");
    return;
  }

  OverrunFlag = true;

  /* Save FPU context here (if necessary) */
  /* Re-enable timer or interrupt here */
  /* Set model inputs here */

  /* Step the model */
  MC_PL0_step(rtM, &rtU, &rtY);

  /* Get model outputs here */

  /* Indicate task complete */
  OverrunFlag = false;

  /* Disable interrupts here */
  /* Restore FPU context here (if necessary) */
  /* Enable interrupts here */
}