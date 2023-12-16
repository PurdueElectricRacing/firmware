/**
 * @file bmm150.h
 * @author your name (cmcgalli@purdue.edu)
 * @brief
 * @version 0.1
 * @date 2023-01-26
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef _BMM150_H_
#define _BMM150_H_

#include "stm32l4xx.h"
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include "common/phal_L4/gpio/gpio.h"
#include "common/phal_L4/spi/spi.h"
#include "SFS.h"

// Suspend mode is the default power mode of BMM150 after the chip is powered
// The device can switch into Active mode from Sleep mode by setting OpMode bits (register 0x4C).

/*
DATAX_LSB (0x42) contains 5-bit LSB part [4:0] of the 13 bit output data of the X-channel.
DATAX_MSB (0x43) contains 8-bit MSB part [12:5] of the 13 bit output data of the X-channel.
DATAY_LSB (0x44) contains 5-bit LSB part [4:0] of the 13 bit output data of the Y-channel.
DATAY_MSB (0x45) contains 8-bit MSB part [12:5] of the 13 bit output data of the Y-channel.
The width of the Z-axis magnetic field data is 15 bit word stored in two’s complement.
DATAZ_LSB (0x46) contains 7-bit LSB part [6:0] of the 15 bit output data of the Z-channel.
DATAZ_MSB (0x47) contains 8-bit MSB part [14:7] of the 15 bit output data of the Z-channel.
*/
// All signed register values are in two´s complement representation.

/*
The “Data ready status” bit (register 0x48 bit0) is set “1” when the data registers have been
updated but the data was not yet read out over digital interface. Data ready is cleared (set “0”)
directly after completed read out of any of the data registers and subsequent stop condition (I²C)
or lifting of CSB (SPI).
*/

// Register 0x4C <1:0>
// Bits 1 and 2
typedef enum
{
    OPMode_NORMAL = 0b00,
    OPMode_FORCED = 0b01,
    OPMode_SLEEP = 0b11,
} BMM150_OPMode_t;

// Register 0x4C <2:0>
// Bits 3, 4, and 5
// output data rate
typedef enum
{
    BMM_ODR_10hz = 0b000,
    BMM_ODR_2hz = 0b001,
    BMM_ODR_6hz = 0b010,
    BMM_ODR_8hz = 0b011,
} BMM150_ODR_t;

typedef union
{
    struct
    {
        int16_t value : 13;
        int16_t excess : 3;
    };
    uint8_t raw_data[2];
} __attribute__((packed)) parsed_data_t;

typedef union
{
    struct
    {
        int16_t value : 15;
        int16_t excess : 1;
    };
    uint8_t raw_data_1;
    uint8_t raw_data_2;
} __attribute__((packed)) parsed_data_z_t;

typedef struct
{
    SPI_InitConfig_t *spi;
    GPIO_TypeDef *mag_csb_gpio_port;
    uint32_t mag_csb_pin;
} BMM150_Handle_t;

#define BMM150_CHIP_ID_ADDR (0x40)
#define BMM150_CHIP_ID (0x32)
#define BMM150_OP_MODE_ADDR (0x4C)
#define BMM150_MAG_X_LSB_ADDR (0x42)
#define BMM150_MAG_X_MSB_ADDR (0x43)

bool BMM150_readID(BMM150_Handle_t *bmm);
void BMM150_powerOnMag(BMM150_Handle_t *bmm);
bool BMM150_init(BMM150_Handle_t *bmm);
bool BMM150_readMag(BMM150_Handle_t *bmm, ExtU *rtU);
void BMM150_setActive(BMM150_Handle_t *bmm);
bool BMM150_selfTest(BMM150_Handle_t *bmm);
bool BMM150_selfTestAdvanced(BMM150_Handle_t *bmm);
#endif