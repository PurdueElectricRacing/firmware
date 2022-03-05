#ifndef MAIN_H_
#define MAIN_H_

//STM32L432KCU6
// Suspension POTAMPL
#define POT_AMPL_LEFT_GPIO_Port (GPIOA)
#define POT_AMPL_LEFT_Pin (0) //left 
#define POT_AMPL_RIGHT_GPIO_Port (GPIOA)
#define POT_AMPL_RIGHT_Pin (1) //right

// USART_R_TX + USTART_R_RX
#define USART_R_TX_GPIO_Port (GPIOA)
#define USART_R_TX_Pin (2)
#define USART_R_RX_GPIO_Port(GPIOA)
#define USART_R_RX_pin (3)

// WC
#define WC_GPIO_Port (GPIOA)
#define WC_Pin (7)

// WSpeedL
#define WSPEEDL_GPIO_Port (GPIOA)
#define WSPEEDL_GPin (8)

//USART_L_TX + USART_L_RX
#define USART_L_TX_GPIO_Port (GPIOA)
#define USART_L_TX_Pin (9)
#define USART_L_RX_GPIO_Port (GPIOA)
#define USART_L_RX_Pin (10)

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

// ERR_LED + CONN_LED
#define ERR_LED_GPIO_Port (GPIOB)
#define ERR_LED_Pin (3)
#define CONN_LED_GPIO_Port (GPIOB)
#define CONN_LED_Pin (4)

// HEARTBEAT
#define HEARBEAT_GPIO_Port (GPIOB)
#define HEARTBEAT_Pin (5)

// SCL + SDA 
#define SCL_GPIO_Port (GPIOB)
#define SCL_Pin (6)
#define SDA_GPIO_Port (GPIOB)
#define SDA_Pin (7)


#endif