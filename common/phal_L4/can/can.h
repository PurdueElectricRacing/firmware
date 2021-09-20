/**
 * @file hal_can_f4.h
 * @author Adam Busch (busch8@purdue.edu)
 * @brief 
 * @version 0.1
 * @date 2021-03-20
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef _PHAL_CAN
#define _PHAL_CAN

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
} CanMsgTypeDef;

bool PHAL_initCAN();
bool PHAL_deinitCAN();

bool PHAL_txCANMessage(CAN_TypeDef* can, CanMsgTypeDef* msg);

#endif // _PHAL_CAN