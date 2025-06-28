## Tool Overview
The PER development environment requires the following tools to build, debug, and deploy firmware to the vehicle:
- [Git](https://git-scm.com/downloads): Version control tool for managing source code and pushing to GitHub.
- [Visual Studio Code](https://code.visualstudio.com/): Text editor with extensions. Notably, we use the following extensions:
	- [Cortex-Debug](https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug): Provides debugging support for ARM Cortex-M MCUs, with live watch.
	- [clangd](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd) (Optional, Recommended): Provides advanced C/C++ code completion and analysis. 
- [CMake](https://cmake.org/install/): Generates the build files needed to compile the project.
- [Ninja](https://ninja-build.org/): Small & fast build system used by CMake.
- [ARM Embedded Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads): A collection of tools  (compiler, linker, debugger, etc.) used to develop software for ARM Cortex-M MCUs.
- [OpenOCD](https://openocd.org/): Interface for debugging STM32 code with GDB.
- [STLink Drivers](https://www.st.com/en/development-tools/stsw-link009.html): drivers for STM32 debugging probe.
- [Python3](https://www.python.org/downloads/): Scripting language we use to do code generation.

> [!NOTE]
> The rest of the setup guide will use a package manager — a tool to help you easily install the other tools. Although optional, it is highly recommended.

## MacOS Tool Setup
1. [Homebrew](https://brew.sh/): package manager.
	- Add `/opt/homebrew/bin` to your environment path variable to make your tools accessible. Check this [link](https://stackoverflow.com/questions/35677031/adding-homebrew-to-path) for help with the process. Your install location may vary.
2. Run the following commands in your terminal to install the other tools (you can copy and paste them all at once).
```bash
brew install git
brew install --cask visual-studio-code
brew install cmake
brew install ninja
brew install --cask gcc-arm-embedded
brew install openocd
brew install stlink
brew install python3
```
> [!NOTE]
> Git may come preinstalled with MacOS, you can double check by running `git --version`.

## Windows Tool Setup
1. [Chocolatey](https://chocolatey.org/install#install-step2): package manager.
2. Run the following commands in your terminal to install the other tools (you can copy and paste them all at once).
```bash
choco install git
choco install vscode
choco install cmake
choco install ninja
choco install gcc-arm-embedded
choco install openocd
choco install python
```
> [!NOTE]
> Make sure to run these in PowerShell as Administrator! 
3. STLink drivers need to be manually installed from [here](https://www.st.com/en/development-tools/stsw-link009.html).

## Linux Tool Setup
1. You probably already know what you're doing, so here are the commands for `apt` (Ubuntu, Debian, Pop!, etc.):
```bash
sudo apt update
sudo apt install git
sudo apt install cmake
sudo apt install ninja-build
sudo apt install gcc-arm-none-eabi
sudo apt install openocd
sudo apt install stlink-tools
sudo apt install python3 python3-pip
```
2. Visual Studio Code requires some special attention, install from [here](https://code.visualstudio.com/docs/setup/linux).

## Repository Setup (All OS)
1. Clone this repository with the following command:
```bash
git clone --recurse-submodules https://github.com/PurdueElectricRacing/firmware.git
```
2. Install the required python packages using the following commands:
```bash
cd firmware
pip3 install -r requirements.txt
```
3. Open Visual Studio Code using the following command:
```bash
code .
```
4. Try running a build by doing `CTRL/CMD + Shift + B` in your VSCode window

All done!