/**
 * @file main.h
 * @author Luke Oxley (lcoxley@purdue.edu)
 * @brief  Software for controlling Torque Vectoring PCB
            sensor acquisition
 * @version 0.1
 * @date 2022-12-08
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef _MAIN_H_
#define _MAIN_H_

#include "common/faults/fault_nodes.h"
#include "common/phal_F4_F7/can/can.h"

// STM32L471RET


#define FAULT_NODE_NAME NODE_TORQUE_VECTOR
// Status Indicators
#define ERR_LED_GPIO_Port (GPIOB)
#define ERR_LED_Pin (5)
#define CONN_LED_GPIO_Port (GPIOB)
#define CONN_LED_Pin (7)
#define CONN_LED_MS_THRESH (500)
#define HEARTBEAT_GPIO_Port (GPIOB)
#define HEARTBEAT_Pin (6)

// SPI IMU
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
#define SPI_CS_MAG_GPIO_Port (GPIOB)
#define SPI_CS_MAG_Pin (0)

// USART GPS
#define GPS_RX_GPIO_Port (GPIOC)
#define GPS_RX_Pin (5)
#define GPS_TX_GPIO_Port (GPIOC)
#define GPS_TX_Pin (4)

#define GPS_RESET_GPIO_Port (GPIOC)
#define GPS_RESET_Pin       (9)

// EEPROM
#define NAV_EEPROM_CS_GPIO_PORT (GPIOB)
#define NAV_EEPROM_CS_PIN (12)
#define NAV_WP_GPIO_PORT (GPIOB)
#define NAV_WP_PIN (13)

void canTxSendToBack(CanMsgTypeDef_t *msg);

// If you modify this struct, update the Python script accordingly
// The Python script must match this exact structure for correct data parsing.
// 
// Python struct format specifiers:
//   - 'f' -> float (4 bytes)
//   - 'i' -> int (4 bytes)
//   - 'h' -> short (2 bytes)
//   - 'b' -> byte (1 byte)
//   - 'H' -> unsigned short (2 bytes)
//   - 'I' -> unsigned int (4 bytes)
//
struct __attribute__((packed)) serial_tx {
    float ET_permit_buffer[5];
    float PT_permit_buffer[5];
    float VS_permit_buffer[5];
    float VT_permit_buffer[5];
    float VCU_mode;
    float IB_CF_buffer[10];
    float TH_CF;
    float ST_CF;
    float VB_CF;
    float WT_CF[2];
    float WM_CF[2];
    float GS_CF;
    float AV_CF[3];
    float IB_CF;
    float MT_CF;
    float CT_CF;
    float IT_CF;
    float MC_CF;
    float IC_CF;
    float BT_CF;
    float AG_CF[3];
    float TO_CF[2];
    float DB_CF;
    float PI_CF;
    float PP_CF;
    float zero_current_counter;
    float Batt_SOC;
    float Batt_Voc;
    float TO_ET[2];
    float TO_AB_MX;
    float TO_DR_MX;
    float TO_PT[2];
    float WM_VS[2];
    float VT_mode;
    float TO_VT[2];
    float TV_AV_ref;
    float TV_delta_torque;
    float TC_highs;
    float TC_lows;
    float sl;
};

// If you modify this struct, update the Python script accordingly
// The Python script must match this exact structure for correct data parsing.
// 
// Python struct format specifiers:
//   - 'f' -> float (4 bytes)
//   - 'i' -> int (4 bytes)
//   - 'h' -> short (2 bytes)
//   - 'b' -> byte (1 byte)
//   - 'H' -> unsigned short (2 bytes)
//   - 'I' -> unsigned int (4 bytes)

struct __attribute__((packed)) serial_rx {
    float TH_RAW;
    float ST_RAW;
    float VB_RAW;
    float WT_RAW[2];
    float WM_RAW[2];
    float GS_RAW;
    float AV_RAW[3];
    float IB_RAW;
    float MT_RAW;
    float CT_RAW;
    float IT_RAW;
    float MC_RAW;
    float IC_RAW;
    float BT_RAW;
    float AG_RAW[3];
    float TO_RAW[2];
    float DB_RAW;
    float PI_RAW;
    float PP_RAW;
    float CS_SFLAG;
    float TB_SFLAG;
    float SS_SFLAG;
    float WT_SFLAG;
    float IV_SFLAG;
    float BT_SFLAG;
    float IAC_SFLAG;
    float IAT_SFLAG;
    float IBC_SFLAG;
    float IBT_SFLAG;
    float SS_FFLAG;
    float AV_FFLAG;
    float GS_FFLAG;
    float VCU_PFLAG;
};

#endif