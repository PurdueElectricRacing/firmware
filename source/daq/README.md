# Data Acquisition Board

This directory contains the firmware source code for the Data Acquisition (DAQ) board, responsible for data collection, storage, and streaming.

## Directory Structure
- `main.c / main.h` - Main entry point for DAQ firmware, responsible for initialization and thread management.
- `daq_hub/` - Central hub defining behavior for SD card writing, Ethernet communication, SPI transactions, threads, etc.
- `spmc/` - Custom lockless Single Producer Multiple Consumer queue implementation for high throughput data buffering between CAN IRQs and consumer threads (SD card writing, Ethernet streaming).
- `fatfs/` - FatFs filesystem module for FAT/exFAT support on embedded systems; enables SD card writing
- `sdio/` - SDIO peripheral drivers for high-speed SD card communication
- `w5500/` - WIZnet TCP/IP chip drivers and application protocols (see [w5500/README.md](./w5500/README.md))