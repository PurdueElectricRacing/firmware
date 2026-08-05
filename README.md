# Purdue Electric Racing Vehicle Software

This repository contains the software used to operate and support Purdue
Electric Racing vehicles.

## Projects

- [firmware](firmware/README.md) contains the embedded firmware, shared
  libraries, build tools, and hardware documentation.
- [daqapp](daqapp/) contains the Rust desktop application for live vehicle
  data acquisition, visualization, and hardware-in-the-loop testing.

## Getting Started

Each project keeps its own build system and should be run from its directory:

```bash
cd firmware
python3 per_build.py
```

```bash
cd daqapp
cargo build --all-targets
```
