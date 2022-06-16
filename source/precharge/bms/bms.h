#ifndef _BMS_H_
#define _BMS_H_

#include "can_parse.h"

#define NUM_CELLS 80
#define BALANCE_DELTA_MINIMUM_V (0.01)  // Minimum delta that balance will be enabled
#define CHARGE_DELTA_MAXIMUM_V (1)      // Maximum delta that charge will be enabled
#define MAX_TEMP (300)

/**
 * @brief Initilize BMS structures
 * 
 */
void BMS_init();

uint8_t BMS_updateMask();

void tempPeriodic();
/**
 * @brief updateBMSError: Update and check a latched error flag for all BMS remote boards
 * 
 * @return uint16_t : Current error flags present
 */
uint16_t BMS_updateErrorFlags();

/**
 * @brief txBatteryStatus: Send CAN messages combined from the BMS modules
 * 
 */
void BMS_txBatteryStatus();

/**
 * @brief Task for monitoring hte ELCON charger and setting votlage/current limits
 * 
 */
void BMS_chargePeriodic();

#endif // _BMS_H_