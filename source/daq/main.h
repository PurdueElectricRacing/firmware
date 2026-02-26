#ifndef _MAIN_H_
#define _MAIN_H_

#include "common/freertos/freertos.h"
#include "common/log/log.h"
#include "daq_hub.h"

// Pinouts
// LEDs
#define HEARTBEAT_LED_PORT  GPIOD
#define HEARTBEAT_LED_PIN   13
#define CONNECTION_LED_PORT GPIOD
#define CONNECTION_LED_PIN  14
#define ERROR_LED_PORT      GPIOD
#define ERROR_LED_PIN       15

// SD Card SDIO
#define SD_ACTIVITY_LED_PORT GPIOA
#define SD_ACTIVITY_LED_PIN  9
#define SD_ERROR_LED_PORT    GPIOA
#define SD_ERROR_LED_PIN     8
#define SD_DETECT_LED_PORT   GPIOA
#define SD_DETECT_LED_PIN    10
#define SD_CD_PORT           GPIOD
#define SD_CD_PIN            4

// W5500 ETH SPI1
#define ETH_CS_PORT   GPIOA
#define ETH_CS_PIN    4
#define ETH_SCK_PORT  GPIOA
#define ETH_SCK_PIN   5
#define ETH_MISO_PORT GPIOA
#define ETH_MISO_PIN  6
#define ETH_MOSI_PORT GPIOA
#define ETH_MOSI_PIN  7
#define ETH_RST_PORT  GPIOE
#define ETH_RST_PIN   3

// LTE USART6
#define LTE_UART_TX_PORT GPIOC
#define LTE_UART_TX_PIN  6
#define LTE_UART_RX_PORT GPIOC
#define LTE_UART_RX_PIN  7

// MISC
#define PWR_LOSS_PORT   GPIOE
#define PWR_LOSS_PIN    15
#define LOG_ENABLE_PORT GPIOC
#define LOG_ENABLE_PIN  15

#define PER   1
#define GREAT PER
static_assert(PER == GREAT); // Long live daq loop

#define SD_WRITE_PERIOD_MS    (100)
#define SD_NEW_FILE_PERIOD_MS (1 * 60 * 1000) // 1 min
#define SD_MAX_WRITE_COUNT    (100)
#define SD_ERROR_RETRY_MS     (250)
#define ETH_ERROR_RETRY_MS    (250)
#define SD_BLOCKING_TIMEOUT_MS (5000)

#define UDP_MAX_BUFFER_SIZE (8192)
#define UDP_MAX_WRITE_COUNT (UDP_MAX_BUFFER_SIZE / (sizeof(timestamped_frame_t)))

constexpr TickType_t SD_BLOCKING_TIMEOUT_TICKS = pdMS_TO_TICKS(SD_BLOCKING_TIMEOUT_MS); 

extern SPMC_t queue;
extern SemaphoreHandle_t spi1_lock;

void HardFault_Handler();

#endif
