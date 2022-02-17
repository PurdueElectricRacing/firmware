#ifndef _BMS_H_
#define _BMS_H_

// Includes
#ifdef STM32L496xx
#include "stm32l496xx.h"
#elif STM32L432xx
#include "stm32l432xx.h"
#else
#error "Please define a STM32 arch"
#endif

#include "common_defs.h"
#include "main.h"
#include "afe.h"

// Generic Defines
#define CELL_MAX    24
#define TEMP_MAX    CELL_MAX / 2

// Structures
typedef struct {
    // Cells are indexed from bottom of stack to top of stack
    uint16_t chan_volts_raw[CELL_MAX];      // Raw 14 bit ADC value for each cell's voltage
    float    chan_volts_conv[CELL_MAX];     // Converted voltage values
    uint16_t mod_volts_raw;                 // Raw 14 bit ADC value for module
    float    mod_volts_conv;                // Converted voltage value
    float    est_cap[CELL_MAX];             // Current estimated cell capacity in W*hr
    float    est_cap_max[CELL_MAX];         // Current estimated maximum cell capacity in W*hr
    float    est_SOC[CELL_MAX];             // Current estimated cell state of charge in %
    float    est_SOH[CELL_MAX];             // Current estimated cell state of health in %
    float    pack_current;                  // Current being pulled from the pack
    float    balance_current[CELL_MAX];     // Current from balance circuit for each cell

    uint32_t balance_flags;                 // Cell overcharge flag
    uint32_t balance_mask;                  // Cell balancing flag masks

    uint16_t chan_temps[TEMP_MAX];          // Converted temperature values
} cells_t;

typedef struct {
    uint32_t error;                         // Error flags
    uint8_t  afe_con;                       // AFE connection flag
    cells_t  cells;                         // Cell information
    
    SPI_InitConfig_t* spi;                  // SPI handle
} bms_t;

extern bms_t bms;                           // Global BMS structure

// Prototypes
void bmsStatus(void);
void initBMS(SPI_InitConfig_t* hspi);

#endif