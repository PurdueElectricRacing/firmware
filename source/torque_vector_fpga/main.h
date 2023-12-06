#ifndef MAIN_H_
#define MAIN_H_

#include "common/faults/fault_nodes.h"

#define FAULT_NODE_NAME NODE_TV

// QuadSPI CS
#define QUADSPI_CS_FLASH_GPIO_Port (GPIOA)
#define QUADSPI_CS_FLASH_Pin       (2)
#define QUADSPI_CS_FPGA_GPIO_Port (GPIOA)
#define QUADSPI_CS_FPGA_Pin       (1)
// QuadSPI CLK
#define QUADSPI_CLK_GPIO_Port (GPIOA)
#define QUADSPI_CLK_Pin       (3)
// QuadSPI I/O Ports
#define QUADSPI_IO0_GPIO_Port (GPIOB)
#define QUADSPI_IO0_Pin       (2)
#define QUADSPI_IO1_GPIO_Port (GPIOB)
#define QUADSPI_IO1_Pin       (1)
#define QUADSPI_IO2_GPIO_Port (GPIOA)
#define QUADSPI_IO2_Pin       (7)
#define QUADSPI_IO3_GPIO_Port (GPIOA)
#define QUADSPI_IO3_Pin       (6)

// FPGA Configuration Reset
#define FPGA_CFG_RST_GPIO_Port (GPIOA)
#define FPGA_CFG_RST_Pin       (4)

// I2C Bus
#define I2C_SCL_GPIO_Port (GPIOB)
#define I2C_SCL_Pin (6)
#define I2C_SDA_GPIO_Port (GPIOB)
#define I2C_SDA_Pin (7)
#define I2C_WRITE_CONTROL_GPIO_Port (GPIOA)
#define I2C_WRITE_CONTROL_Pin (6)

// Status LEDs
#define ERROR_LED_GPIO_Port (GPIOA)
#define ERROR_LED_Pin       (15)
#define CONN_LED_GPIO_Port (GPIOB)
#define CONN_LED_Pin       (4)
#define HEARTBEAT_LED_GPIO_Port (GPIOB)
#define HEARTBEAT_LED_Pin       (5)

#endif