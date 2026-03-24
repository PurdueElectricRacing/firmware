# Bootloader Common (`common/bootloader`)

Shared bootloader/application contract lives here.

## What this module provides

- `bootloader_common.h`
  - Common enums/constants for bootloader command and status protocol:
    - `BLCmd_t` (`BLCMD_START`, `BLCMD_CRC_BACKUP`, `BLCMD_CRC`, `BLCMD_JUMP`, `BLCMD_RST`)
    - `BLStatus_t`
    - `BLError_t`
  - Shared memory struct and magic value:
    - `BootloaderSharedMemory_t`
    - `BOOTLOADER_SHARED_MEMORY_MAGIC`
  - Flash layout constants (`BL_ADDRESS_*`, `MAX_FIRMWARE_SIZE`) for supported MCU families.
- `bootloader_common.c`
  - Defines `bootloader_shared_memory` in `.noinit` so it survives reset.
  - Implements reset helpers used by the application:
    - `Bootloader_ResetForFirmwareDownload()`
    - `Bootloader_ResetForWatchdog()`

## Shared memory contract

`bootloader_shared_memory` is used to pass reset intent across a software reset:

- `magic_word`: set to `BOOTLOADER_SHARED_MEMORY_MAGIC` when the app requests a special reset path
- `reset_reason`: reason enum consumed by bootloader/app startup logic
- `reset_count`: currently reset to `0` by helper functions

The application should call `Bootloader_ConfirmApplicationLaunch()` early in app startup to acknowledge a successful handoff.

## Build integration

`CMakeLists.txt` exposes this as the `BOOTLOADER_COMMON` interface library so both app and bootloader components share the same definitions and implementation.
