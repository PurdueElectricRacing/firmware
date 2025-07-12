/**
 * @file process.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-02-28
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef _PROCESS_H
#define _PROCESS_H

#include "can_parse.h"
#include "common/bootloader/bootloader_common.h"
#include "inttypes.h"
#include "node_defs.h"
#include "stdbool.h"

typedef enum {
    BLSTAT_VALID = 0,
    BLSTAT_INVALID = 1,
    BLSTAT_INVALID_CRC = 2,
    BLSTAT_UNKNOWN_CMD = 3,
} BLStatus_t;

typedef enum {
    BLERROR_CRC_FAIL = 0,
    BLERROR_LOCKED = 1,
    BLERROR_LOW_ADDR = 2,
    BLERROR_ADDR_BOUND = 3,
    BLERROR_FLASH = 4,
    BLERROR_SIZE = 5,
} BLError_t;

void BL_checkAndBoot(void);

void BL_processCommand(BLCmd_t cmd, uint32_t data);

void BL_sendStatusMessage(uint8_t cmd, uint32_t data);

bool BL_flashStarted(void);

#endif
