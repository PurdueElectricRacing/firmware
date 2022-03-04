#ifndef _FAULTS_H_
#define _FAULTS_H_

// Includes
#include "stm32l432xx.h"
#include "common/phal_L4/can/can.h"
#include "common/psched/psched.h"

#include "faults_glob.h"
#include "stdbool.h"

typedef void (*err_callback)(void);

// Defines
#define F_COUNT             0
#define F_TEST_L_TIME       10
#define F_TEST_UL_TIME      5
#define F_TEST_WIND         true

// Structures
typedef struct {
    bool     active;
    uint16_t time_curr;
} fault_t;

typedef struct {
    // User modifiable
    err_callback callback;

    // Internal only
    bool         __callback_set;
    fault_t      faults[F_COUNT];
} fault_core_t;

// Prototypes
void faultInit(err_callback callback);
void faultsBg(void);
bool faultCheck(size_t idx);
void __assert(bool truth);
__weak void __recover(void* mem_addr);

__weak void testCB(void);

// Variables
bool         f_wind[F_COUNT]     = {F_TEST_WIND};
uint32_t     f_l_time[F_COUNT]   = {F_TEST_L_TIME};
uint32_t     f_ul_time[F_COUNT]  = {F_TEST_UL_TIME};
err_callback f_callback[F_COUNT] = {testCB};

#endif