# PER Component Firmware Projects
[![CircleCI](https://circleci.com/gh/PurdueElectricRacing/firmware/tree/master.svg?style=svg)](https://circleci.com/gh/PurdueElectricRacing/firmware/tree/master)


A mega-repository full of all firmware projects, build tools, and dependencies to create firmware modules for the car.

## Directory Structure

  - ./cmake - CMake helper files for compiling common modules
  - ./common - Common firmware modules shared across the codebase
  - ./source - Firmware comonents for descrete MCUs
  - ./output - Generated output files from compiling
  - ./build - CMake work directory (if CMake gives you errors, some can be solved by deleting this directory and trying again)

# Getting Started
1. Initialize the git submodules in this project with the command `git submodule update --init --recursive` to download the source for the various git submodules.
2. Install the required python packages with the command `pip install -r requirements.txt`.

## Option A) Local Package Install [PREFERRED]
Install these packages:
1. [arm-none-eabi-gcc](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads) Compiler specific to ARM based targets.
   - [Windows](https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.07/gcc-arm-none-eabi-10.3-2021.07win32/gcc-arm-none-eabi-10.3-2021.07-win32.exe)
   - [Mac](https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.07/gcc-arm-none-eabi-10.3-2021.07-mac-10.14.6-sha1.pkg)
   - Linux - Find it yourself
2. [CMake](https://cmake.org/install/) Build system generator. This takes care of making all of the build files needed to compile the project.
   - On some Mac OS versions, CMake will install as a GUI only, follow the `Tools > Install Command Line Tools` tip inside CMake to fix this
3. [Ninja](https://ninja-build.org/) Small & fast build system used by CMake
4. [Git](https://git-scm.com/downloads) Tool for managing source code and uploading to GitHub.
5. [Visual Studio Code](https://code.visualstudio.com/) Code editor with extensions for helping build the firmware components.
6. [STLink Drivers](https://www.st.com/en/development-tools/stsw-link009.html) Windows drivers for STM32 debugging probe
   - [Alternate open-source drivers for all platforms](https://github.com/stlink-org/stlink)

### Notes for MacOS Install
1. Adding a program to your path: `sudo vim /etc/paths`

## Option B) Docker Install
You can also use docker to install all of the packages inside a development container. This has a minor drawback of not being able to fully complete the VSCode IntelliSense database with your source files for auto completion and code navigation.

1. This project has a Dockerfile which will setup your build environment and setup the tools necessary to develop firmware. Once you install [Docker](https://docs.docker.com/get-docker/) you should be able to access command line tools by running `docker --help` for a list of available commands.

2. Build the image defined by the `Dockerfile` by running:
```
docker compose build develop
```
>This command uses the `docker-compose.yaml` file to build the image with the tag of `stm32_develop:latest`. 

4. Run the compiled image in a container:
```
docker compose run develop
```
> This will place you into an interactive command line inside a container defined by the `stm32_develop:latest` image. This is a minimal Ubuntu 20.04 distro and moves you to the `/per` directory. This directory is linked to the same directory in host machine which is defined in the `docker-compose.yaml` file.

## Building Firmware Components

### CMake Extension
VSCode has a recommended CMake extension. This extension is configured throught the bottom ribbon of VSCode where you can select the `GCC for arm-none-eabi` toolchain and specific build targets.

### Python build script

```
python3 per_build.py
``` 
> This command is a thin wrapper around CMake to build all of the components which will be placed in a newly created `output` folder. Running `python3 per_build.py --help` will give you more options for building components.

## Debugging on Hardware
In order to begin flashing, executing, or debugging your code, you need to connect to a STM32 device using an STLink. You need to install the latest STLink drivers as mentionted in the getting started section. VSCode has a "Cortex-Debug" extension that we use for connecting to the STM32 devices.

The "Run and Debug" window will allow for you to upload code to any firmware component which has a configuration in the `.vscode/launch.json` file.

## CircleCI Integration
Each pull request into the master branch will be automatically built using [CircleCI](https://app.circleci.com/pipelines/github/PurdueElectricRacing/firmware?filter=all). This build needs to pass in order for the pull request to be merged. It is important to keep the build system and docker image up to date. Future work can be put in to add software unit tests and have those block merges as well!

This is an attempt at making sure that all code is able to build when pushed to the master branch.

In order to update the docker image being used by CircleCI to build the firmware components you must make the necessacary changes to the Dockerfile so the firmware is able to build completley. The docker image is hosted by DockerHub, you will need to create an account there before you can push a new image.
After the changes have been made to the Dockerfile, build the docker image with a tag by running
```
docker image build . -t <docker hub username>/per_firmware:latest
```

You can then push that to docker hub with
```
docker push <docker hub username>/per_firmware:latest
```

Make sure that image tag is being referenced in the `.circleci/config.yaml`:
```
    docker:
      - image: <docker hub username>/per_firmware:latest
```