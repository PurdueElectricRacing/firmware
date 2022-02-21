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

#include "common/psched/psched.h"
#include "common_defs.h"
#include "main.h"
#include "afe.h"
#include "temp.h"

// Generic Defines
#define CELL_MAX    12
#define TEMP_MAX    20
#define TEMP_MAX_C  600

#define BMS_ACCUM
// #define BMS_LV

// Structures
typedef struct {
    // Cells are indexed from bottom of stack to top of stack
    uint16_t chan_volts_raw[CELL_MAX];      // Raw 14 bit ADC value for each cell's voltage
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

    uint16_t chan_temps_raw[TEMP_MAX];      // Raw temperature values
    uint16_t chan_temps_conv[TEMP_MAX];     // Converted temperature values
} cells_t;

typedef struct {
    uint16_t charge_curr;                   // Charge current in A * 10
    uint16_t charge_volts;                  // Charge voltage in V * 10
    uint16_t charge_power;                  // Charge power in W
    uint8_t  charge_mode;                   // 0: voltage and current, 1: voltage and power
    uint8_t  charging;                      // Marks if charging has started
} charge_t;

typedef struct {
    uint16_t soc_max;
    uint16_t temp_max;
    uint16_t v_max;
} p_lim_t;

typedef struct {
    // Error flags:
    // 
    // [0] -> AFE connection error
    // [1] -> Cell overvoltage
    // [2] -> Cell undervoltage
    // [3] -> Temp connection error
    // [4] -> Cell temp critical
    // [5] -> Cell temp derivative critical
    uint32_t error;                         // Error flags

    // Sleep flags:
    //
    // [0] -> AFE not ready to sleep
    // [1] -> Temps not ready to sleep
    // [2] -> Comms not ready to sleep
    //
    // Note: BMS cannot sleep with an error
    uint8_t  no_sleep;                      // Flags for each portion letting us sleep
    uint8_t  sleep_req;                     // Marks a request for sleep entry

    uint8_t  afe_con;                       // AFE connection flag
    uint8_t  bms_charge;                    // Charge flag
    uint8_t  cell_count;                    // Number of cells
    uint8_t  temp_count;                    // Number of thermistors on a board
    uint8_t  temp_master;                   // Marks this PCB as a thermistor driver
    uint16_t master_p_lim;                  // Final power limit sent out
    uint16_t current_out;                   // Current output to MC (x100)
    uint16_t voltage_out;                   // Voltage output to MC (x100)
    uint32_t power_out;                     // Power output to MC (x10000)
    cells_t  cells;                         // Cell information
    p_lim_t  p_lim;                         // Power limits

    SPI_InitConfig_t* spi;                  // SPI handle
} bms_t;

extern bms_t bms;                           // Global BMS structure

// Prototypes
void bmsStatus(void);
void initBMS(SPI_InitConfig_t* hspi);
void setPLim(void);
void calcMisc(void);
void checkSleep(void);

#endif