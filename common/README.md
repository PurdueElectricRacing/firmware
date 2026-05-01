# Common Modules
Libraries, drivers, generic data structures, and other utilities.

## Core platform layers
- `phal` Shared PHAL interface layer and common headers.
- `phal_F4_F7` PHAL implementations for STM32F4/F7 targets.
- `phal_G4` PHAL implementations for STM32G4 targets.
- `freertos` FreeRTOS configuration and wrapper functions.

## Device / protocol drivers
- `amk` AMK motor controller helpers.
- `bmi088` BMI088 IMU driver.
- `nextion` Nextion display communication helpers.
- `sdio` SDIO helpers.
- `ublox` u-blox GNSS parsing and configuration helpers.
- `izze_imu` IZZE IMU message definitions/helpers.

## System services
- `heartbeat` Board heartbeat / liveness utilities.
- `watchdog` Watchdog abstraction and helpers.
- `syscalls` Syscall shims used by embedded runtime/newlib.

## Utility libraries
- `common_defs` Shared compile-time/runtime definitions.
- `utils` General-purpose math and macro helpers (`min/max/clamp/rescale`, etc.).
- `strbuf` Fixed-size string buffer utilities.
- `lerp_lut` Lookup-table interpolation utilities.
- `bangbang` Bang-bang control helper.
