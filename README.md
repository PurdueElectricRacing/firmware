# PER Component Firmware Projects

[![CircleCI](https://circleci.com/gh/PurdueElectricRacing/firmware/tree/master.svg?style=svg)](https://circleci.com/gh/PurdueElectricRacing/firmware/tree/master)
![GitHub commit activity](https://img.shields.io/github/commit-activity/m/PurdueElectricRacing/firmware?style=flat-square)

A mega-repository full of all firmware projects, build tools, and dependencies to create firmware modules for the car.

## Directory Structure

- `/cmake` - CMake helper files for compiling common modules
- `/common` - Common firmware modules shared across the codebase
- `/source` - Firmware source code for specific MCUs on the car
- `/output` - Generated output files from compiling
- `/build` - CMake work directory (if CMake gives you errors, some can be solved by deleting this directory and trying again)
- `/.circleci` - Automated cloud build process configuration
- `/.vscode` - Visual Studio Code configuration directory

## Getting Started

Before you can compile software for PER vehicle, here are some steps you need to take to configure your system. Detailed tool install and setup instructions can be found [here](setup.md).

## Building Firmware Components

### Python build script

```
python3 per_build.py
```

> This command is a thin wrapper around CMake to build all of the components which will be placed in a newly created `output` folder. Running `python3 per_build.py --help` will give you more options for building components.

## Debugging on Hardware

In order to begin flashing, executing, or debugging your code, you need to connect to a STM32 device using an STLink. You need to install the latest STLink drivers as mentionted in the getting started section. VSCode has a ["Cortex-Debug" extension](https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug) that we use for connecting to the STM32 devices.

The "Run and Debug" window will allow for you to upload code to any firmware component which has a configuration in the `.vscode/launch.json` file.

## Build Bootloader Components

Because there are many bootloader components to build and they only need to be re-built every now and again, building the bootloaders is disabled by default. In order to enable building the bootloaders, you need to edit the CMake cache to set the "BOOTLOADER*BUILD" option to "ON". This can be done inside VSCode using the "Edit CMake Cahce (UI)" command. This needs to be disabled if you want to debug just your application code, as the applications will be built using the bootloader linker script. Notice that a new filename is used for the bootloader .hex and .elf files with the prefix "BL*".

## CircleCI Integration

Each pull request into the master branch will be automatically built using [CircleCI](https://app.circleci.com/pipelines/github/PurdueElectricRacing/firmware?filter=all). This build needs to pass in order for the pull request to be merged. It is important to keep the build system and docker image up to date. Future work can be put in to add software unit tests and have those block merges as well!

This is an attempt at making sure that all code is able to build when pushed to the master branch.

In order to update the docker image being used by CircleCI to build the firmware components you must make the necessacary changes to the Dockerfile so the firmware is able to build completley. The docker image is hosted by DockerHub, you will need to create an account there before you can push a new image.
After the changes have been made to the Dockerfile, build the docker image with a tag by running

```bash
docker image build . -t <docker hub username>/per_firmware:latest
```

You can then push that to docker hub with

```bash
docker push <docker hub username>/per_firmware:latest
```

Make sure that image tag is being referenced in the `.circleci/config.yaml`:

```bash
    docker:
      - image: <docker hub username>/per_firmware:latest
```
