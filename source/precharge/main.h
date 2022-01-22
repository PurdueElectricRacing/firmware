#ifndef MAIN_H_
#define MAIN_H_

// Current Sense
#define I_SENSE_CH1_GPIO_Port (GPIOA)
#define I_SENSE_CH1_Pin (0)
#define I_SENSE_CH2_GPIO_Port (GPIOA)
#define I_SENSE_CH2_Pin (1)

// BMS Status
#define BMS_STATUS_GPIO_Port (GPIOA)
#define BMS_STATUS_Pin (3)

// IMD Data
#define IMD_HS_PWM_GPIO_Port (GPIOB)
#define IMD_HS_PWM_Pin (6)
#define IMD_LS_PWM_GPIO_Port (GPIOB)
#define IMD_LS_PWM_Pin (7)
#define IMD_STATUS_GPIO_Port (GPIOB)
#define IMD_STATUS_Pin (8)

// Status LEDs
#define HEARTBEAT_LED_GPIO_Port (GPIOE)
#define HEARTBEAT_LED_Pin       (13)
#define CONN_LED_GPIO_Port      (GPIOE)
#define CONN_LED_Pin            (14)
#define ERROR_LED_GPIO_Port     (GPIOE)
#define ERROR_LED_Pin           (15)


#endif