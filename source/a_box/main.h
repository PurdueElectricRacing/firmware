#ifndef MAIN_H_
#define MAIN_H_


#include "common/faults/fault_nodes.h"


#define FAULT_NODE_NAME NODE_A_BOX


// SPI Accel
#define SPI_SCLK_GPIO_Port (GPIOA)
#define SPI_SCLK_Pin (5)
#define SPI_MISO_GPIO_Port (GPIOA)
#define SPI_MISO_Pin (6)
#define SPI_MOSI_GPIO_Port (GPIOA)
#define SPI_MOSI_Pin (7)
#define SPI_CS_ACEL_GPIO_Port (GPIOA)
#define SPI_CS_ACEL_Pin (3)
#define SPI_CS_GYRO_GPIO_Port (GPIOA)
#define SPI_CS_GYRO_Pin (2)
#define SPI_CS_TMU_GPIO_Port (GPIOD)
#define SPI_CS_TMU_GPIO_Pin (15)


// Current Sense
#define I_SENSE_CH1_GPIO_Port (GPIOA) // ADC12_IN5
#define I_SENSE_CH1_Pin (0)
#define I_SENSE_CH2_GPIO_Port (GPIOA) // ADC12_IN6
#define I_SENSE_CH2_Pin (1)


// BMS Status
#define BMS_STATUS_GPIO_Port (GPIOD)
#define BMS_STATUS_Pin (1)
#define BMS_DISCHARGE_ENABLE_Port (GPIOD)
#define BMS_DISCHARGE_ENABLE_Pin (0)
#define BMS_CHARGE_ENABLE_Port (GPIOD)
#define BMS_CHARGE_ENABLE_Pin (3)
#define BMS_CHARGER_SAFETY_Port (GPIOD)
#define BMS_CHARGER_SAFETY_Pin (2)


// IMD Data
#define IMD_HS_PWM_GPIO_Port (GPIOB)
#define IMD_HS_PWM_Pin (6)
#define IMD_LS_PWM_GPIO_Port (GPIOB)
#define IMD_LS_PWM_Pin (7)
#define IMD_STATUS_GPIO_Port (GPIOB)
#define IMD_STATUS_Pin (5)


// Status LEDs
#define HEARTBEAT_LED_GPIO_Port (GPIOE)
#define HEARTBEAT_LED_Pin       (13)
#define CONN_LED_GPIO_Port      (GPIOE)
#define CONN_LED_Pin            (14)
#define ERROR_LED_GPIO_Port     (GPIOE)
#define ERROR_LED_Pin           (15)


//TMU Mux Input Pins
#define MUX_A_Port  (GPIOC)
#define MUX_A_Pin   (9)
#define MUX_B_Port  (GPIOC)
#define MUX_B_Pin   (8)
#define MUX_C_Port  (GPIOC)
#define MUX_C_Pin   (7)
#define MUX_D_Port  (GPIOC)
#define MUX_D_Pin   (6)

//TMU Mux Output Pins
#define MUX_1_OUT_Port (GPIOC)
#define MUX_1_OUT_Pin (0)
#define MUX_2_OUT_Port (GPIOC)
#define MUX_2_OUT_Pin (1)
#define MUX_3_OUT_Port (GPIOC)
#define MUX_3_OUT_Pin (2)
#define MUX_4_OUT_Port (GPIOC)
#define MUX_4_OUT_Pin (3)

// Board Temp
#define BOARD_TEMP_Port (GPIOB)
#define BOARD_TEMP_Pin (0)

// 5V Monitoring
#define VSENSE_5V_Port (GPIOA)
#define VSENSE_5V_Pin (7)

// ADC Channels
#define TMU_1_ADC_CHANNEL (10)
#define TMU_2_ADC_CHANNEL (11)
#define TMU_3_ADC_CHANNEL (12)
#define TMU_4_ADC_CHANNEL (13)

typedef struct 
{
    // Do not modify this struct unless
    // you modify the ADC DMA config
    // in main.c to match
    uint16_t tmu_1;
    uint16_t tmu_2;
    uint16_t tmu_3;
    uint16_t tmu_4;
}__attribute__((packed)) ADCReadings_t;
volatile extern ADCReadings_t adc_readings;

#endif
