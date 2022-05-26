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
#include "can/can_parse.h"
#include "node_defs.h"

// Generic Defines
#define CELL_MAX                12
#define TEMP_MAX                20
#define TEMP_MAX_C              600
#define MIN_CHG_CURR            (-1)
#define CELL_CHARGE_IMPLAUS     0.003f
#define MAX_DELTA               100U
#define BAL_DUTY                0.82f
#define BAL_RES                 30.0f

// #define BMS_ACCUM
#ifndef BMS_ACCUM
#define BMS_LV
#endif

#define NO_BALANCE

// Enumerations
typedef enum {
    MODE_IDLE,
    MODE_DISCHARGE,
    MODE_CHARGE
} bms_mode_t;

typedef enum {
    E_AFE_CONN,
    E_OV,
    E_UV,
    E_OW,
    E_TEMP_CONN,
    E_TEMP,
    E_TEMP_DT,
    E_EEPROM,
    E_I2C_CONN,
    E_CAN_TX,
    // Must come last!
    E_CNT
} bms_error_t;

// Structures
typedef struct {
    // Cells are indexed from bottom of stack to top of stack
    uint16_t chan_volts_raw[CELL_MAX];      // Raw 14 bit ADC value for each cell's voltage
    uint16_t pu[CELL_MAX];                  // Raw 14 bit ADC value during pull-up section of OW
    uint16_t pd[CELL_MAX];                  // Raw 14 bit ADC value during pull-down section of OW
    uint16_t mod_volts_raw;                 // Raw 14 bit ADC value for module
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
    // Error flags (blinking light codes):
    // 
    // 1  -> AFE connection error
    // 2  -> Cell overvoltage
    // 3  -> Cell undervoltage
    // 4  -> Cell open-wire
    // 5  -> Temp connection error
    // 6  -> Cell temp critical
    // 7  -> Cell temp derivative critical
    // 8  -> EEPROM not initialized
    // 9  -> I2C communication/connection error
    // 10 -> CAN TX error
    uint32_t   error;                       // Error flags
    uint16_t   ow;                          // Open wire flag
    uint16_t   ov;                          // Over-voltage flag
    uint16_t   uv;                          // Under-voltage flag

    // Sleep flags:
    //
    // [0] -> AFE not ready to sleep
    // [1] -> Temps not ready to sleep
    // [2] -> Comms not ready to sleep
    //
    // Note: BMS cannot sleep with an error
    uint8_t    no_sleep;                    // Flags for each portion letting us sleep
    uint8_t    sleep_req;                   // Marks a request for sleep entry

    uint8_t    afe_con;                     // AFE connection flag
    uint8_t    veh_con;                     // Vehicle connection flag
    uint8_t    bms_charge;                  // Charge flag
    uint8_t    cell_count;                  // Number of cells
    uint8_t    curr_sense_conn;             // Current sensor on CAN bus
    uint8_t    temp_count;                  // Number of thermistors on a board
    uint8_t    temp_master;                 // Marks this PCB as a thermistor driver
    uint16_t   master_p_lim;                // Final power limit sent out
    int16_t    current_out;                 // Current output to MC (x100)
    uint16_t   voltage_out;                 // Voltage output to MC (x100)
    int32_t    power_out;                   // Power output to MC (x10000)
    float      die_temp;                    // Internal die temperature in deg C
    float      afe_vdd;                     // AFE VDD voltage in V
    float      afe_ref;                     // AFE ref voltage in V
    cells_t    cells;                       // Cell information
    p_lim_t    p_lim;                       // Power limits
    bms_mode_t op_mode;                     // Operating mode

    SPI_InitConfig_t* spi;                  // SPI handle
} bms_t;

extern bms_t      bms;                      // Global BMS structure
extern uint8_t    error_ff;                 // Error fast-forward across memset
extern q_handle_t q_tx_can;                 // TX CAN queue
extern q_handle_t q_rx_can;                 // RX CAN queue

// Prototypes
void bmsStatus(void);
void initBMS(SPI_InitConfig_t* hspi);
void txCAN(void);
void setPLim(void);
void calcMisc(void);
void checkLVStatus(void);
void checkSleep(void);
void canTxUpdate(void);

#endif