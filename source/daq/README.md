# Data Acquisition Board

This directory contains the firmware source code for the Data Acquisition (DAQ) board, responsible for data collection, storage, and communication.

## Directory Structure

- **`buffer/`** - Files for creating and managing data buffers
- **`daq_hub/`** - Central hub defining behavior for SD card writing, Ethernet communication, SPI transactions, threads, etc.
- **`fatfs/`** - FatFs filesystem module for FAT/exFAT support on embedded systems; enables SD card writing
- **`ftp/`** - FTP server library files (see [ftp/README.md](./ftp/README.md))
- **`sdio/`** - SDIO peripheral drivers for high-speed SD card communication
- **`w5500/`** - WIZnet TCP/IP chip drivers and application protocols (see [w5500/README.md](./w5500/README.md))

## Main Entry Point

**`main.c/main.h`** - Initializes the board with:
- GPIO configuration
- Interrupt setup
- CAN communication initialization
- Buffer creation
- Thread spawning