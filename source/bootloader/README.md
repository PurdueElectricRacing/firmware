# Bootloader Technical Reference (`source/bootloader`)

This directory contains the firmware bootloader implementation used by PER boards.
It is intended as an implementation-level reference (architecture, memory contract,
command processing, and build behavior).

For operational usage (how to build/flash/update), see `docs/bootloader.md`.

## 1. Components in this directory

- `main.c`
  - Bare-metal entry point (no FreeRTOS).
  - Initializes clocks/GPIO/SysTick/CAN.
  - Runs a CAN command window (`BOOTLOADER_INITIAL_TIMEOUT = 3000 ms`).
  - Attempts application boot via CRC metadata validation.
  - Falls back to infinite polling loop if no valid app is present.
- `bootloader/bootloader.c`
  - Core state machine and flash operations.
  - Handles `BLCMD_*` command processing.
  - Stages incoming firmware in a buffer region.
  - Verifies CRC and copies image into app/backup region.
  - Writes boot metadata (`crc`, `addr`, `size`) used by next boot.
- `node_defs.h`
  - Per-node CAN peripheral/pin config and message IDs.
  - Maps compile-time `APP_ID` to generated canpiler headers (`BL_<NODE>.h`).

## 2. Shared contract with application code

Shared definitions live in `common/bootloader/bootloader_common.{h,c}`.

Important symbols:

- Commands: `BLCMD_START`, `BLCMD_CRC`, `BLCMD_CRC_BACKUP`, `BLCMD_JUMP`, `BLCMD_RST`
- Status: `BLSTAT_VALID`, `BLSTAT_INVALID`, `BLSTAT_INVALID_CRC`, `BLSTAT_UNKNOWN_CMD`
- Errors: `BLERROR_*`
- Flash layout constants: `BL_ADDRESS_*`, `MAX_FIRMWARE_SIZE`
- Metadata offsets:
  - `BL_CRC_OFFSET_CRC = 0x00`
  - `BL_CRC_OFFSET_ADDR = 0x04`
  - `BL_CRC_OFFSET_SIZE = 0x08`

## 3. Flash map used by bootloader

### STM32G4

- `0x08000000 - 0x08003FFF`: bootloader image
- `0x08004000 - 0x08007FFF`: metadata (CRC block)
- `0x08008000 - ...`: primary application (`BL_ADDRESS_APP`)
- `0x08030000 - ...`: staging buffer (`BL_ADDRESS_BUFFER`)
- `0x08058000 - ...`: backup app slot (`BL_ADDRESS_BACKUP`)

### STM32F4

- `0x08000000 - 0x08003FFF`: bootloader image
- `0x08004000 - 0x08007FFF`: metadata (CRC block)
- `0x08008000 - ...`: primary application (`BL_ADDRESS_APP`)
- `0x08040000 - ...`: staging buffer (`BL_ADDRESS_BUFFER`)
- `0x08080000 - ...`: backup app slot (`BL_ADDRESS_BACKUP`)

Application linker scripts (`*_FLASH_APP.ld`) place the app at `0x08008000`.
Bootloader linker scripts (`*_FLASH_BL.ld`) reserve the first 32 KB.

## 4. Boot and update behavior

### Boot path (`BL_checkAndBoot`)

On boot, metadata at `BL_ADDRESS_CRC` is read:

1. `crc_stored`
2. `addr`
3. `size`

Boot continues only if:

- `addr` is one of `{BL_ADDRESS_APP, BL_ADDRESS_BACKUP}`
- `size` is nonzero, 4-byte aligned, and within `MAX_FIRMWARE_SIZE`
- `addr + size` remains in device flash bounds
- computed CRC over `[addr, addr+size)` matches `crc_stored`

If valid, bootloader jumps to app vector table (`VTOR = addr`, MSP loaded from app image).

### Update path (command flow)

1. `BLCMD_START(size)`
   - validates size
   - erases staging buffer
   - unlocks write path
2. `bl_<node>_data(index, word)` frames
   - data stored in staging buffer
   - G4 path uses 64-bit aligned writes
3. `BLCMD_CRC(crc)` or `BLCMD_CRC_BACKUP(crc)`
   - computes CRC over staged image
   - if valid: copies staged image to destination (`APP` or `BACKUP`)
   - writes metadata block (`crc`, `dst_addr`, `size`) to `BL_ADDRESS_CRC`
4. `BLCMD_JUMP`
   - attempts immediate boot using metadata

## 5. CRC specifics

Both firmware and host tooling are expected to use STM32-compatible CRC-32:

- polynomial: `0x04C11DB7`
- init: `0xFFFFFFFF`
- data fed as 32-bit words

The bootloader uses `PHAL_CRC32_Calculate(...)`.

## 6. CAN contract source of truth

Message schemas and IDs are generated from canpiler configs:

- host TX definitions:
  - `common/can_library/configs/external_nodes/BOOTLOADER_HOST.json`
- device TX response definitions:
  - `common/can_library/configs/nodes/BL_<NODE>.json`

Generated headers consumed by this module live in:

- `common/can_library/generated/BL_<NODE>.h`

## 7. Build outputs and target mapping

`source/bootloader/CMakeLists.txt` builds one target per node:

- G4: `bootloader_MAIN_MODULE`, `bootloader_DASHBOARD`, `bootloader_A_BOX`,
  `bootloader_TORQUE_VECTOR`, `bootloader_DRIVELINE`, `bootloader_G4_TESTING`
- F4: `bootloader_PDU`

When building with bootloader mode enabled (`-DBOOTLOADER_BUILD=ON`), output artifact
names are prefixed with `BL_` by postbuild rules.

---

For developer/operator instructions (build commands, combined binaries, update flow),
use `docs/bootloader.md`.
