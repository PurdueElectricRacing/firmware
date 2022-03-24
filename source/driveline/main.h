#ifndef MAIN_H_
#define MAIN_H_

#include <stdbool.h>

//STM32L432KCU6
// Suspension POTAMPL
#define POT_AMPL_GPIO_Port (GPIOA)
#define POT_AMPL_Pin (0) //left 
#define POT_AMPR_GPIO_Port (GPIOA)
#define POT_AMPR_Pin (1) //right

// USART_R_TX + USTART_R_RX
#define USART_R_TX_GPIO_Port (GPIOA)
#define USART_R_TX_Pin (2)
#define USART_R_RX_GPIO_Port(GPIOA)
#define USART_R_RX_pin (3)

// WSpeedL
#define WSPEEDL_GPIO_Port (GPIOA)
#define WSPEEDL_Pin (8)
#define WHEELSPEEDL_AF (1)

//USART_L_TX + USART_L_RX
#define USART_L_TX_GPIO_Port (GPIOA)
#define USART_L_TX_Pin (9)
#define USART_L_RX_GPIO_Port (GPIOA)
#define USART_L_RX_Pin (10)
#define USART_L (USART2)

// CAN_RX + CAN_TX
#define CAN_RX_GPIO_Port (GPIOA)
#define CAN_RX_Pin (11)
#define CAN_TX_GPIO_Port (GPIOA)
#define CAN_TX_Pin (12)

// SWDIO + SWCLK
#define SWDIO_GPIO_Port (GPIOA)
#define SWDIO_Pin (13)
#define SWCLK_GPIO_Port (GPIOA)
#define SWCLK_Pin (14)

// WSpeedR
#define WSPEEDR_GPIO_Port (GPIOA)
#define WSPEEDR_Pin (15)
#define WHEELSPEEDR_AF (1)

// ERR_LED + CONN_LED
#define ERR_LED_GPIO_Port (GPIOB)
#define ERR_LED_Pin (3)
#define CONN_LED_GPIO_Port (GPIOB)
#define CONN_LED_Pin (3)

// HEARTBEAT
#define HEARTBEAT_GPIO_Port (GPIOB)
#define HEARTBEAT_Pin (5)

// EEPROM
#define WC_GPIO_Port (GPIOA)
#define WC_Pin (7)
#define SCL_GPIO_Port (GPIOB)
#define SCL_Pin (6)
#define SDA_GPIO_Port (GPIOB)
#define SDA_Pin (7)

// Status LEDs
#define LED_GREEN_Pin (3)
#define LED_GREEN_GPIO_Port (GPIOB)
#define LED_RED_Pin (1)
#define LED_RED_GPIO_Port (GPIOB)
#define LED_BLUE_Pin (7)
#define LED_BLUE_GPIO_Port (GPIOB)

// TIM Pins
#define TIM1_GPIO_Port (GPIOA)
#define TIM1_Pin (8)
#define TIM1_AF (1)

#define TIM2_GPIO_Port (GPIOA)
#define TIM2_Pin (0)
#define TIM2_AF (1)

#define TIM16_GPIO_Port (GPIOA)
#define TIM16_Pin (6)
#define TIM16_AF (14)

// Motor Controller Constants:
float CELL_MAX_V = 4.2; //May be increased to 4.25 in the future
float CELL_MIN_V = 2.5;

//Motor Controller reading results
bool is_over_powered;
int controller_temp;
int mot_temp;

typedef struct motor_data {
    bool proper_voltage;
    float phase_current;
    bool is_over_powered;
    float controller_temp;
    float motor_temp;
    //These are integers, and the period of time per measurement is 1 second
    int motor_temp_slope;
    int con_temp_slope;
} motor_data_t;

#endif

