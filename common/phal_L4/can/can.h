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

#include <stdbool.h>

#include "stm32l4xx.h"

#define PHAL_CAN_TX_TIMEOUT   (5000U) //!< in milliseconds
#define PHAL_CAN_INIT_TIMEOUT (5000U) //!< in milliseconds

/** Bit timing recovered from http://www.bittiming.can-wiki.info/ */
#define PHAL_CAN_16MHz_500k (0x001c0001)
#define PHAL_CAN_20MHz_500k (0x00050004)
#define PHAL_CAN_40MHz_500k (0x001c0004)
#define PHAL_CAN_80MHz_500k (0x001c0009)

#define PHAL_CAN_16MHz_250k (0x001c0003)

typedef struct
{
    CAN_TypeDef* Bus; //!< Specifies the bus.
    uint16_t StdId; //!< Specifies the standard identifier.
    uint32_t ExtId; //!< Specifies the extended identifier.
    uint32_t IDE; //!< Specifies the type of identifier for the message that will be transmitted.
    uint32_t DLC; //!< Specifies the length of the frame that will be transmitted.
    uint8_t Data[8]; //!< Contains the data to be transmitted. */
} CanMsgTypeDef_t;

/**
 * @brief Initilize CAN peripheral to 500k. 
 * 
 * @param bus The CAN peripheral to initialize (i.e. CAN1)
 * @param test_mode Initilize CAN peripheral for self test mode
 * @param bit_rate  Bit rate in bps (i.e. 500000)
 * 
 * @return true Peripheral sucessfully initalized
 * @return false Peripheral stalled during initilization
 */
bool PHAL_initCAN(CAN_TypeDef* bus, bool test_mode, uint32_t bit_rate);

/**
 * @brief De-initialize the CAN peripheral
 * 
 * @param bus The CAN peripheral to de-initialize (i.e. CAN1)
 * @return Perihperal succesfully de-initialized
 */
bool PHAL_deinitCAN(CAN_TypeDef* bus);

/**
 * @brief Find an empty TX mailbox and transmit a CAN message if one is found.
 * Function will block until sucessful transmission of message until a specified timeout.
 * 
 * @param can CAN peripheral to transmit with
 * @param msgId Message ID
 * @return true Sucessful TX of message.
 * @return false Unable to find empty message or transmit took too long.
 */
bool PHAL_txCANMessage(CanMsgTypeDef_t* msg, uint8_t txMbox);
bool PHAL_txMailboxFree(CAN_TypeDef* bus, uint8_t mbx);
void PHAL_txCANAbort(CAN_TypeDef* bus, uint8_t mbx);

#endif // _PHAL_CAN_H