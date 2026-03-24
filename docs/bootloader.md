# Bootloader Usage Guide

This guide covers **how to use** the PER bootloader in day-to-day development:

- what to build
- what artifact to flash
- how to perform CAN updates

For implementation details (memory map, command handlers, CRC internals), see:

- `source/bootloader/README.md`
- `common/bootloader/README.md`

---

## 1) Quick build matrix

### Build bare application firmware (no bootloader artifacts)

```bash
python3 per_build.py
```

### Build app + bootloader support artifacts

```bash
python3 per_build.py -b
```

This generates, per selected board:

- app binary + CRC (`<board>.bin`, `<board>.crc`)
- bootloader binary (`bootloader_<NODE>.bin`)
- combined image (`<board>_combined.bin`)

### Limit to specific board targets

```bash
python3 per_build.py -t "main_module dashboard"
python3 per_build.py -b -t "main_module"
```

`driveline` can be used as an alias for both:

- `front_driveline`
- `rear_driveline`

### Package for daqapp2 flashing

```bash
python3 per_build.py -p
python3 per_build.py -b -p
python3 per_build.py -b -p -t "main_module"
```

Creates `output/firmware_<git-hash-or-tag>.tar.gz` containing selected board artifacts
and `manifest.json`.

---

## 2) Which artifact should I flash?

### A) Full-device raw flash (factory/recovery)

Use:

- `<board>_combined.bin`

This image contains:

1. bootloader region
2. boot metadata (`crc`, `app start`, `app size`)
3. application payload

So the bootloader can validate and jump to app immediately after reset.

### B) CAN bootloader update (normal OTA/service flow)

Use:

- `<board>.bin` as update payload
- `<board>.crc` for final CRC command

The bootloader writes metadata during `BLCMD_CRC` / `BLCMD_CRC_BACKUP` handling.

---

## 3) Typical CAN update sequence

1. Ensure target is in bootloader mode
   - app-triggered reset path calls `Bootloader_ResetForFirmwareDownload()`
   - or power-on and enter update window
2. Send `BLCMD_START` with image size (bytes, 4-byte aligned)
3. Stream `bl_<node>_data` frames (`index`, `data`)
4. Send CRC command:
   - `BLCMD_CRC` for primary slot
   - `BLCMD_CRC_BACKUP` for backup slot
5. Wait for `bl_<node>_resp` status
6. Send `BLCMD_JUMP` or reset target

---

## 4) Message contracts and IDs

Canonical canpiler config sources:

- host command/data TX definitions:
  - `common/can_library/configs/external_nodes/BOOTLOADER_HOST.json`
- bootloader response TX definitions:
  - `common/can_library/configs/nodes/BL_<NODE>.json`

Payload conventions:

- `bl_<node>_cmd`
  - `cmd` (`uint8_t`): `BLCMD_*`
  - `data` (`uint32_t`): command argument
- `bl_<node>_data`
  - `index` (`uint16_t`): 32-bit word index
  - `data` (`uint32_t`): firmware word payload
- `bl_<node>_resp`
  - `status` (`uint8_t`): `BLSTAT_*`
  - `detail` (`uint32_t`): CRC, size, or error detail

---

## 5) Common checks / troubleshooting

- **No combined file generated for a board**
  - That board may not have a mapped bootloader target (expected for some boards).
- **Bootloader rejects CRC (`BLSTAT_INVALID_CRC`)**
  - Ensure host CRC algorithm matches STM32 CRC behavior (word-wise, poly `0x04C11DB7`, init `0xFFFFFFFF`).
- **Target stays in bootloader after raw flash**
  - Use `<board>_combined.bin` (not app-only image) for full-device flashing.
- **Unknown target passed to `-t`**
  - Run `python3 per_build.py --list`.

---

## 6) Regenerate CAN outputs after config changes

```bash
python3 common/can_library/canpiler/build.py
```

This refreshes generated headers/sources and DBC files used by firmware and host tooling.
