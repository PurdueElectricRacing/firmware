#ifndef _BMS_H_
#define _BMS_H_

#include "can_parse.h"

#define NUM_CELLS 80

/**
 * @brief updateBMSError: Update and check a latched error flag for all BMS remote boards
 * 
 * @return uint16_t : Current error flags present
 */
uint16_t updateBMSErrorFlags();

/**
 * @brief txBatteryStatus: Send CAN messages combined from the BMS modules
 * 
 */
void txBatteryStatus();

#endif // _BMS_H_