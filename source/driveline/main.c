#include "stm32l432xx.h"
#include "common/phal_L4/can/can.h"
#include "common/psched/psched.h"
#include "common/phal_L4/can/can.h"
#include "common/phal_L4/rcc/rcc.h"
#include "common/phal_L4/gpio/gpio.h"
#include "common/phal_L4/adc/adc.h"
#include "common/phal_L4/i2c/i2c.h"
#include "common/phal_L4/tim/tim.h"
#include "common/phal_L4/dma/dma.h"
#include "common/eeprom/eeprom.h"
#include "common/phal_L4/usart/usart.h"
#include "can_parse.h"
#include "main.h"

#include <stdbool.h>

#if (FTR_DRIVELINE_REAR) && (FTR_DRIVELINE_FRONT)
#error "Can not specify both front and rear driveline for the same binary!"
#elif (!FTR_DRIVELINE_REAR) && (!FTR_DRIVELINE_FRONT)
#error "You must define either FTR_DRIVELINE_REAR or FTR_DRIVELINE_FRONT"
#endif

uint16_t data[78] = {'\0'};
char translated[78] = {'a'}; 
motor_data_t processed_data;
int prev_mot_temp = 0;
int prev_con_temp = 0;

GPIOInitConfig_t gpio_config[] = {
  GPIO_INIT_CANRX_PA11,
  GPIO_INIT_CANTX_PA12,
  GPIO_INIT_I2C3_SCL_PA7,
  GPIO_INIT_I2C3_SDA_PB4,
  GPIO_INIT_USART1TX_PA9,
  GPIO_INIT_USART1RX_PA10,
  GPIO_INIT_OUTPUT(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_OUTPUT_LOW_SPEED),
  GPIO_INIT_OUTPUT(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_OUTPUT_LOW_SPEED),
  GPIO_INIT_OUTPUT(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_OUTPUT_LOW_SPEED),
  GPIO_INIT_INPUT(BUTTON_1_GPIO_Port, BUTTON_1_Pin, GPIO_INPUT_PULL_DOWN),
  GPIO_INIT_AF(TIM1_GPIO_Port, TIM1_Pin, TIM1_AF, GPIO_OUTPUT_ULTRA_SPEED, GPIO_TYPE_AF, GPIO_INPUT_PULL_UP),
  GPIO_INIT_ANALOG(POT_GPIO_Port, POT_Pin),
  GPIO_INIT_ANALOG(POT2_GPIO_Port, POT2_Pin),
  GPIO_INIT_ANALOG(POT3_GPIO_Port, POT3_Pin)
};

usart_handle_t huart1 = {
    .instance = USART1,
    .baud_rate = 115000,
    .word_length = WORD_8,
    .hw_flow_ctl = HW_DISABLE,
    .mode = MODE_TX_RX,
    .stop_bits = SB_ONE,
    .parity = PT_NONE,
    .obsample = OB_DISABLE,
    .ovsample = OV_16,
    .adv_feature.rx_inv = true,
    .adv_feature.tx_inv = true,
    .adv_feature.auto_baud = false,
    .adv_feature.data_inv = false,
    .adv_feature.dma_on_rx_err = false,
    .adv_feature.msb_first = false,
    .adv_feature.overrun = false,
};


#define TargetCoreClockrateHz 16000000
ClockRateConfig_t clock_config = {
    .system_source              =SYSTEM_CLOCK_SRC_HSI,
    .system_clock_target_hz     =TargetCoreClockrateHz,
    .ahb_clock_target_hz        =(TargetCoreClockrateHz / 1),
    .apb1_clock_target_hz       =(TargetCoreClockrateHz / (1)),
    .apb2_clock_target_hz       =(TargetCoreClockrateHz / (1)),
};

extern uint32_t APB1ClockRateHz;
extern uint32_t APB2ClockRateHz;
extern uint32_t AHBClockRateHz;
extern uint32_t PLLClockRateHz;

int current_power = 0;
bool enable_usart2 = false;

//Function Prototypes
void HardFault_Handler(void);
void mc_analog_init(void);
void mc_serial_init(void);
void mc_forward(float);
void mc_reverse(float);
void mc_brake(float);
void mc_stop(void);
void runMC(void);
void read_motor_controller(void);
void canTxUpdate(void);
void run_user_commands(void);

q_handle_t q_tx_can;
q_handle_t q_rx_can;

int main (void)
{
    //Data struct init
    qConstruct(&q_tx_can, sizeof(CanMsgTypeDef_t));
    qConstruct(&q_rx_can, sizeof(CanMsgTypeDef_t));
    initCANParse(&q_rx_can);

    /* HAL Initilization */
    if(0 != PHAL_configureClockRates(&clock_config))
    {
        HardFault_Handler();
    }
    if(!PHAL_initGPIO(gpio_config, sizeof(gpio_config)/sizeof(GPIOInitConfig_t)))
    {
        HardFault_Handler();
    }
    PHAL_initUSART(&huart1, APB2ClockRateHz);

    
    if (enable_usart2) {
        huart1.instance = USART2;
    }
    

    /* Task Creation */
    schedInit(APB2ClockRateHz);
    taskCreate(runMC, 1);
    taskCreate(canTxUpdate, 5);
    taskCreate(canRxUpdate, RX_UPDATE_PERIOD);
    taskCreate(run_user_commands, 1);

    schedStart();

    return 0;
} /* int main() */

/*
 *  Runs state machine that processes the timing and execution of commands
 *  to the motor controller.
 */

void runMC(void) {
    static uint16_t time;
    
    switch (time) {
        case (0):
        {
            mc_serial_init();
            //mc_forward(10.0);
            mc_brake(30.0);
          //mc_analog_init;
          //How to recieve the data messages
          //can_data.torque_request.front_left

            break;
        }
        
        case (1000):
        {

           mc_stop();

            break;
        }
        
    }

    // Reads data from the motor controller every second **!(time % 15)**
    if (time % 1000 == 0) {
     //   read_motor_controller();
    }

    if (++time == 10001) { 
        time = 0;
    }
} /* runMC() */

void run_user_commands(void) {
    uint16_t power = can_data.torque_request.front_left;
    if (power < 0) {
        mc_brake(power);
    }
    else if (power > 0) {
        mc_forward(power);
    }
    else {
        mc_stop();
    }
}

void HardFault_Handler()
{
    PHAL_writeGPIO(LED_GREEN_GPIO_Port, LED_GREEN_Pin, 0);
    while(1)
    {
        __asm__("nop");
    }
} /* HardFault_Handler() */

/*
 *  Initializes the motor controller in analog mode.
 */

void mc_analog_init(){
	uint16_t init[] = {'p'};
	PHAL_usartTxBl(&huart1, init, 1);
	return;
} /* mc_analog_init() */

/*
 *  Initializes the motor controller in serial mode,
 *  meaning that it can accept digital inputs.
 */

void mc_serial_init(){
	uint16_t init[] = {'s'};
	PHAL_usartTxBl(&huart1, init, 1);
	return;
} /* mc_serial_init() */

/*
 *  Spins the motor forward in serial mode to the given
 *  power.
 */

void mc_forward(float power){
	if(power > 100 || power < 0){
		return;
	}
	int input_power = (int)(power * 10);
	uint16_t init[] = {'f'};
	uint16_t increase[] = {'g'};
	uint16_t decrease[] = {'l'};

	//puts the mc in forward mode
	PHAL_usartTxBl(&huart1, init, 1);

	//checks if the current power is less than the input power and increases accordingly
	if (current_power < input_power){
		for(int i = current_power; i < input_power; i++){
				PHAL_usartTxBl(&huart1, increase, 1); //increase speed by 0.1%
		}
	}
	//checks if the current power is greater than the input power and decreases accordingly
	else if (current_power > input_power){
		for(int i = current_power; i > input_power; i--){
				PHAL_usartTxBl(&huart1, decrease, 1);  //increase speed by 0.1%
		}
	}
	//sets the current power equal to the input power
	current_power = input_power;
	return;
} /* mc_forward() */

/*
 *  Spins the motor in reverse to the given power.
 */

void mc_reverse(float power){

	if(power > 100 || power < 0 || current_power != 0){
		return;
	}

	int input_power = (int)(power * 10);
	uint16_t init[] = {'r'};
	uint16_t increase[] = {'g'};
	uint16_t decrease[] = {'l'};

	//puts the mc in reverse mode
    PHAL_usartTxBl(&huart1, init, 1); 
	//checks if the current power is less than the input power and increases accordingly
	if (current_power < input_power){
		for(int i = current_power; i < input_power; i++){
			PHAL_usartTxBl(&huart1, increase, 1); ;  //increase speed by 0.1%
		}
	}
	//checks if the current power is greater than the input power and decreases accordingly
	else if (current_power > input_power){
		for(int i = current_power; i > input_power; i--){
			PHAL_usartTxBl(&huart1, decrease, 1); ;  //increase speed by 0.1%
		}
	}
	//sets the current power equal to the input power
	current_power = input_power;
	return;
} /* mc_reverse() */

/*
 *  If the motor is currently spinning, this function
 *  sends the command to stop the motor.
 */
void mc_stop(){
	uint16_t stop[] = {'0'};
	PHAL_usartTxBl(&huart1, stop, 1);
	current_power = 0;
	return;
} /* mc_stop() */

//**Send a positive power
void mc_brake(float power) {
    if(power > 100 || power < 0 || current_power != 0){
		return;
	}

    int input_power = (int)(power * 10);
    uint16_t init[] = {'b'};
	uint16_t increase[] = {'g'};
	uint16_t decrease[] = {'l'};

    PHAL_usartTxBl(&huart1, init, 1); 
	//checks if the current power is less than the input power and increases accordingly
	if (current_power < input_power){
		for(int i = current_power; i < input_power; i++){
			PHAL_usartTxBl(&huart1, increase, 1); ;  //increase speed by 0.1%
		}
	}
	//checks if the current power is greater than the input power and decreases accordingly
	else if (current_power > input_power){
		for(int i = current_power; i > input_power; i--){
			PHAL_usartTxBl(&huart1, decrease, 1); ;  //increase speed by 0.1%
		}
	}
	//sets the current power equal to the input power
	current_power = input_power;
	return;
}

/*
 *  Reads the data being sent from the motor controller
 */

void read_motor_controller() { //Index 26 is U, 37 is the values for I
    for (int i = 0; i < 76; i++) {
        data[i] = '\0';
        translated[i] = 'a';
    }
    PHAL_usartRxBl(&huart1, data, 77);
    for (int i = 0; i < 76; i++) {
        translated[i] = data[i];
    }
    for (int i = 0; i < 76; i++) {
        if (data[i] == 32) {
            data[i] = '0';
        }
    }
    float voltage = ((data[30] - '0')* 100) + ((data[31] - '0')* 10) + (data[32] - '0') + ((data[34] - '0') / 10);
    if ((voltage < (CELL_MAX_V * 80)) && (voltage > (CELL_MIN_V * 80))) {
        proper_voltage = true;
    }
    else {
        proper_voltage = false;
    }
    phase_current = ((data[39] - '0') * 100) + ((data[40] - '0') * 10) + (data[41] - '0') + ((data[43] - '0') / 10);
    if ((voltage * phase_current) > 60000) {
        is_over_powered = false;
    }
    controller_temp = ((data[61] - '0') * 100) + ((data[62] - '0') * 10) + (data[63] - '0');
    mot_temp = ((data[71] - '0')* 100) + ((data[72] - '0') * 10) + (data[73] - '0');

    int con_slope;
    int mot_slope;
    if (!(prev_con_temp == 0)) {
        con_slope = controller_temp - prev_con_temp;

    }
    if (!(prev_mot_temp == 0)) {
        mot_slope = mot_temp - prev_mot_temp;
    }


    processed_data.is_over_powered = is_over_powered;
    processed_data.proper_voltage = proper_voltage;
    processed_data.motor_temp = mot_temp;
    processed_data.controller_temp = controller_temp;
    processed_data.phase_current = phase_current;
    
    prev_mot_temp = mot_temp;

    return;
} /* read_motor_controller() */

// *** Compulsory CAN Tx/Rx callbacks ***
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

        rx.Data[0] = (uint8_t) (CAN1->sFIFOMailBox[0].RDLR >> 0) & 0xFF;
        rx.Data[1] = (uint8_t) (CAN1->sFIFOMailBox[0].RDLR >> 8) & 0xFF;
        rx.Data[2] = (uint8_t) (CAN1->sFIFOMailBox[0].RDLR >> 16) & 0xFF;
        rx.Data[3] = (uint8_t) (CAN1->sFIFOMailBox[0].RDLR >> 24) & 0xFF;
        rx.Data[4] = (uint8_t) (CAN1->sFIFOMailBox[0].RDHR >> 0) & 0xFF;
        rx.Data[5] = (uint8_t) (CAN1->sFIFOMailBox[0].RDHR >> 8) & 0xFF;
        rx.Data[6] = (uint8_t) (CAN1->sFIFOMailBox[0].RDHR >> 16) & 0xFF;
        rx.Data[7] = (uint8_t) (CAN1->sFIFOMailBox[0].RDHR >> 24) & 0xFF;

        canProcessRxIRQs(&rx);

        CAN1->RF0R     |= (CAN_RF0R_RFOM0); 

        qSendToBack(&q_rx_can, &rx); // Add to queue (qSendToBack is interrupt safe)
    }
}