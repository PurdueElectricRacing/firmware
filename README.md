# PER Vehicle Firmware ⚡️

[![CircleCI](https://circleci.com/gh/PurdueElectricRacing/firmware/tree/master.svg?style=svg)](https://circleci.com/gh/PurdueElectricRacing/firmware/tree/master)
![GitHub commit activity](https://img.shields.io/github/commit-activity/m/PurdueElectricRacing/firmware?style=flat-square)

A monorepo of all firmware projects, build tools, and scripts driving the PER vehicle.


## Directory Structure
- `common/` - Common libraries shared across the codebase
- `source/` - Source code for each vehicle PCB
- `.vscode/` - VSCode configuration directory


## Getting Started

To compile software for the PER vehicle, make sure your system is set up by following the steps in [setup.md](setup.md) if you haven’t already.

> [!NOTE]
> ![setup.md](setup.md) is here!


## Building Firmware

Firmware is built using a python-based build system. The python script `per_build.py` handles CMake configuration and ninja build steps automatically.

To build the firmware, run:
```bash
python3 per_build.py
```

You can view available build targets and options with:
```bash
python3 per_build.py --help
```

## Hardware Debugging 

In VS Code, go to **View → Run and Debug**, select the appropriate MCU target from the dropdown, then press the green ▶️ arrow to flash and live-debug the firmware.

Once everything is ![set up](setup.md), you can build the firmware by pressing:

```
Ctrl + Shift + B on Windows/Linux
Cmd + Shift + B on macOS
```

This triggers the default build task configured in .vscode/tasks.json, which runs the firmware build process automatically.

Make sure you're in the root of the `firmware` repo (code .) before triggering the build.
