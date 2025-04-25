#ifndef __BOOTLOADER_H__
#define __BOOTLOADER_H__

#include "inttypes.h"
#include "stdbool.h"
#include "can_parse.h"
#include "node_defs.h"
#include "common/bootloader/bootloader.h"

#define BL_METADATA_PING_MAGIC 0xFEE2DEAD

void BL_checkAndBoot(bool initial);
bool BL_flashStarted(void);

#endif // __BOOTLOADER_H__
