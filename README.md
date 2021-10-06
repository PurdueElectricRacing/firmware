# PER Component Firmware Projects
A mega-repository full of all firmware projects, build tools, and dependencies to create firmware modules for the car.

## Directory Structure
 ```
 cmake - CMake helper files for compiling common modules
 common - Common firmware modules shared across the codebase
 source - Firmware comonents for descrete MCUs
 ```
## Getting Started
1. Initialize the git submodules in this project with the command `git submodule update --init --recursive` to download the source for the various git submodules.
2. Install the required python packages with the command `pip install -r requirements.txt`.

### Option A) Local Package Install
Install these packages:
1. [arm-none-eabi-gcc](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads) Compiler specific to ARM based targets.
   1. [Windows](https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.07/gcc-arm-none-eabi-10.3-2021.07win32/gcc-arm-none-eabi-10.3-2021.07-win32.exe)
   2. [Mac](https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.07/gcc-arm-none-eabi-10.3-2021.07-mac-10.14.6-sha1.pkg)
   3. Linux - Find it yourself
2. [CMake](https://cmake.org/install/) Build system generator. This takes care of making all of the build files needed to compile the project.
3. [Ninja](https://ninja-build.org/) Small & fast build system used by CMake

### Option B) Docker Install
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

### Building firmware components
```
python3 per_build.py
``` 
> This command is a thin wrapper around CMake to build all of the components which will be placed in a newly created `output` folder. Running `python3 per_build.py --help` will give you more options for building components.