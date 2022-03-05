#ifndef _FAULTS_H_
#define _FAULTS_H_

// Includes
#include "stm32l432xx.h"
#include "common/phal_L4/can/can.h"
#include "common/psched/psched.h"
#include "common/common_defs/common_defs.h"

#include "stdbool.h"

typedef void (*err_callback)(void);

// Defines
// #define F_MASTER
#define F_SLAVE

// Cell overtemp
#define F_BMS_TEMP_L_TIME       0
#define F_BMS_TEMP_UL_TIME      100
#define F_BMS_TEMP_WIND         false
#define F_BMS_TEMP_CRIT         true

// Cell undervoltage
#define F_BMS_UV_L_TIME         0
#define F_BMS_UV_UL_TIME        10000
#define F_BMS_UV_WIND           false
#define F_BMS_UV_CRIT           true

// Cell overvoltage
#define F_BMS_OV_L_TIME         0
#define F_BMS_OV_UL_TIME        10000
#define F_BMS_OV_WIND           false
#define F_BMS_OV_CRIT           true

// AFE connection lost
#define F_BMS_CONN_L_TIME       1000
#define F_BMS_CONN_UL_TIME      0
#define F_BMS_CONN_WIND         false
#define F_BMS_CONN_CRIT         true

// AFE internal error
#define F_BMS_AFE_L_TIME        1000
#define F_BMS_AFE_UL_TIME       0
#define F_BMS_AFE_WIND          false
#define F_BMS_AFE_CRIT          true

// Thermistor communication error
#define F_BMS_I2C_L_TIME        1000
#define F_BMS_I2C_UL_TIME       0
#define F_BMS_I2C_WIND          false
#define F_BMS_I2C_CRIT          true

// Motor controller overtemp
#define F_MC_TEMP_L_TIME        3000
#define F_MC_TEMP_UL_TIME       3000
#define F_MC_TEMP_WIND          false
#define F_MC_TEMP_CRIT          true

// Motor controller low flow rate
#define F_MC_FLOW_L_TIME        3000
#define F_MC_FLOW_UL_TIME       3000
#define F_MC_FLOW_WIND          false
#define F_MC_FLOW_CRIT          true

// Battery low flow rate
#define F_BATT_FLOW_L_TIME      0
#define F_BATT_FLOW_UL_TIME     3000
#define F_BATT_FLOW_WIND        false
#define F_BATT_FLOW_CRIT        true

// APPS check failure
#define F_APPS_L_TIME           100
#define F_APPS_UL_TIME          0
#define F_APPS_WIND             false
#define F_APPS_CRIT             false

// BSPD trip
#define F_BSPD_L_TIME           100
#define F_BSPD_UL_TIME          0
#define F_BSPD_WIND             false
#define F_BSPD_CRIT             false

// Enumerations
enum {
    F_BMS_TEMP,
    F_BMS_UV,
    F_BMS_OV,
    F_BMS_CONN,
    F_BMS_AFE,
    F_BMS_I2C,
    F_MC_TEMP,
    F_MC_FLOW,
    F_BATT_FLOW,
    F_APPS,
    F_BSPD,
    
    // F_COUNT MUST COME LAST
    F_COUNT
}

// Variables
bool     f_wind[F_COUNT]    = {F_BMS_TEMP_WIND, F_BMS_UV_WIND, F_BMS_OV_WIND, F_BMS_CONN_WIND, F_BMS_AFE_WIND, F_BMS_I2C_WIND
                               F_MC_TEMP_WIND, F_MC_FLOW_WIND, F_BATT_FLOW_WIND, F_APPS_WIND, F_BSPD_WIND};
uint32_t f_l_time[F_COUNT]  = {F_BMS_TEMP_UL_TIME, F_BMS_UV_UL_TIME, F_BMS_OV_UL_TIME, F_BMS_CONN_UL_TIME, F_BMS_AFE_UL_TIME, F_BMS_I2C_UL_TIME
                               F_MC_TEMP_UL_TIME, F_MC_FLOW_UL_TIME, F_BATT_FLOW_UL_TIME, F_APPS_UL_TIME, F_BSPD_UL_TIME};
uint32_t f_ul_time[F_COUNT] = {F_BMS_TEMP_L_TIME, F_BMS_UV_L_TIME, F_BMS_OV_L_TIME, F_BMS_CONN_L_TIME, F_BMS_AFE_L_TIME, F_BMS_I2C_L_TIME
                               F_MC_TEMP_L_TIME, F_MC_FLOW_L_TIME, F_BATT_FLOW_L_TIME, F_APPS_L_TIME, F_BSPD_L_TIME};
bool     f_crit[F_COUNT]    = {F_BMS_TEMP_CRIT, F_BMS_UV_CRIT, F_BMS_OV_CRIT, F_BMS_CONN_CRIT, F_BMS_AFE_CRIT, F_BMS_I2C_CRIT
                               F_MC_TEMP_CRIT, F_MC_FLOW_CRIT, F_BATT_FLOW_CRIT, F_APPS_CRIT, F_BSPD_CRIT};

// Structures
typedef struct {
    bool     active;
    uint16_t time_curr;
} fault_t;

typedef struct {
    bool         master_caution;
    bool         master_error;
    fault_t      faults[F_COUNT];
} fault_core_t;

// Prototypes
void heartTask(void);
void faultBg(void);
bool faultCheck(size_t idx);
void __assert(bool truth);
__weak void __recover(void* mem_addr);

#endif