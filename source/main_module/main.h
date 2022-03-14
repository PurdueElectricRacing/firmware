#ifndef _MAIN_H_
#define _MAIN_H_

//STM32L496VGT6

// Status Indicators
#define ERR_LED_GPIO_Port (GPIOE)
#define ERR_LED_Pin (9)
#define CONN_LED_GPIO_Port (GPIOE)
#define CONN_LED_Pin (10)
#define HEARTBEAT_GPIO_Port (GPIOE)
#define HEARTBEAT_Pin (11)
#define BRK_LIGHT_GPIO_Port (GPIOA)
#define BRK_LIGHT_Pin (5)
#define BUZZER_GPIO_Port (GPIOA)
#define BUZZER_Pin (6)
#define UNDERGLOW_GPIO_Port (GPIOA)
#define UNDERGLOW_Pin (7)

#define SDC_CTRL_GPIO_Port (GPIOA)
#define SDC_CTRL_Pin (10)

// Drivetrain
#define DT_THERM_1_GPIO_Port (GPIOA)
#define DT_THERM_1_Pin (1)
#define DT_THERM_2_GPIO_Port (GPIOA)
#define DT_THERM_2_Pin (0)
#define DT_PUMP_CTRL_GPIO_Port (GPIOE)
#define DT_PUMP_CTRL_Pin (13)
#define DT_PUMP_FLOW_ADJ_GPIO_Port (GPIOA) // N/A for current pump
#define DT_PUMP_FLOW_ADJ_Pin (4)           // N/A for current pump
#define DT_FLOW_RATE_PWM_GPIO_Port (GPIOC)
#define DT_FLOW_RATE_PWM_Pin (1)
#define DT_RAD_FAN_CTRL_GPIO_Port (GPIOE)
#define DT_RAD_FAN_CTRL_Pin (14)

// Battery (HV)
#define BAT_THERM_OUT_GPIO_Port (GPIOA)
#define BAT_THERM_OUT_Pin (2)
#define BAT_THERM_IN_GPIO_Port (GPIOA)
#define BAT_THERM_IN_Pin (3)
#define BAT_PUMP_CTRL_GPIO_Port (GPIOE)
#define BAT_PUMP_CTRL_Pin (15)
#define BAT_PUMP_FLOW_ADJ_GPIO_Port (GPIOC)
#define BAT_PUMP_FLOW_ADJ_Pin (3)
#define BAT_FLOW_RATE_PWM_GPIO_Port (GPIOC)
#define BAT_FLOW_RATE_PWM_Pin (2)
#define BAT_RAD_FAN_CTRL_GPIO_Port (GPIOC)
#define BAT_RAD_FAN_CTRL_Pin (4)

#define I_SENSE_C1_GPIO_Port (GPIOC)
#define I_SENSE_C1_Pin (0)

// LV Bat
#define LIPO_BAT_STAT_GPIO_Port (GPIOA)
#define LIPO_BAT_STAT_Pin (15)
#define LV_I_SENSE_GPIO_Port (GPIOB)
#define LV_I_SENSE_Pin (0)

// TODO: what are these
#define SWDIO_GPIO_Port (GPIOA)
#define SWDIO_Pin (13)
#define SWCLK_GPIO_Port (GPIOA)
#define SWCKL_Pin (14)

// TODO: need these?
#define WC_GPIO_Port (GPIOB)
#define WC_Pin (5)
#define SCL_GPIO_Port (GPIOB)
#define SCL_Pin (6)
#define SDA_GPIO_Port (GPIOB)
#define SDA_Pin (7)

#define DBG_SCL_GPIO_Port (GPIOB)
#define DBG_SCL_Pin (10)
#define DBG_SDA_GPIO_Port (GPIOB)
#define DBG_SDA_Pin (11)

#define VCAN_TX_GPIO_Port (GPIOD)
#define VCAN_TX_Pin (0)
#define VCAN_RX_GPIO_Port (GPIOD)
#define VCAN_RX_Pin (1)

#endif