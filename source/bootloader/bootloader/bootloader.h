/**
 * @file bootloader.h
 * @brief Core bootloader API
 */

#ifndef _BOOTLOADER_H_
#define _BOOTLOADER_H_

#include <stdint.h>
#include <stdbool.h>
#include "common/bootloader/bootloader_common.h"

extern volatile uint32_t bootloader_ms;

/* Check firmware CRC and boot if valid */
void BL_checkAndBoot(void);

/* Process incoming bootloader command */
void BL_processCommand(uint8_t cmd, uint32_t data);

/* Send status message to host */
void BL_sendStatusMessage(uint8_t status, uint32_t data);

/* Check if flash operation is in progress */
bool BL_flashStarted(void);

/* CAN polling function */
void BL_CANPoll(void);

/* Initialize CAN for bootloader */
void BL_CANInit(void);

#endif /* _BOOTLOADER_H_ */
