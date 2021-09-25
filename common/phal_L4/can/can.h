/**
 * @file can.h
 * @author Adam Busch (busch8@purdue.edu)
 * @brief 
 * @version 0.1
 * @date 2021-03-20
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef _PHAL_CAN_H
#define _PHAL_CAN_H

#include "stm32l432xx.h"
#include <stdbool.h>

#define PHAL_CAN_TX_TIMEOUT   (1000U)
#define PHAL_CAN_INIT_TIMEOUT (1000U)

typedef struct
{
  uint16_t StdId; /*!< Specifies the standard identifier. */
  uint32_t ExtId; /*!< Specifies the extended identifier. */
  uint32_t IDE; /*!< Specifies the type of identifier for the message that will be transmitted.  */
  uint32_t DLC; /*!< Specifies the length of the frame that will be transmitted. */
  uint8_t Data[8]; /*!< Contains the data to be transmitted. */
} CanMsgTypeDef_t;

/**
 * @brief Initilize CAN peripheral to 500k. 
 * 
 * @param test_mode Initilize CAN peripheral for self test mode
 * 
 * @return true Peripheral sucessfully initalized
 * @return false Peripheral stalled during initilization
 */
bool PHAL_initCAN(bool test_mode);

bool PHAL_deinitCAN();

/**
 * @brief Find an empty TX mailbox and transmit a CAN message if one is found.
 * Function will block until sucessful transmission of message until a specified timeout.
 * 
 * @param can CAN peripheral to transmit with
 * @param msgId Message ID
 * @return true Sucessful TX of message.
 * @return false Unable to find empty message or transmit took too long.
 */
bool PHAL_txCANMessage(CanMsgTypeDef_t* msg);

#endif // _PHAL_CAN_H