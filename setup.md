## Repository Clone
Clone this repository with the following command:
```bash
git clone -r https://github.com/PurdueElectricRacing/firmware.git
```
- Make sure you have git installed before running this command!

## Tool Overview
The PER development environment requires the following tools to build, debug, and deploy firmware to the vehicle:
- [Git](https://git-scm.com/downloads): Version control tool for managing source code and pushing to GitHub.
- [Visual Studio Code](https://code.visualstudio.com/): Text editor with extensions. Notably, we use the following extensions:
	- [Cortex-Debug](https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug): Provides debugging support for ARM Cortex-M MCUs, with live watch.
	- [clangd](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd) (Optional, Recommended): Provides advanced C/C++ code completion and analysis. 
- [CMake](https://cmake.org/install/): Generates the build files needed to compile the project.
- [Ninja](https://ninja-build.org/): Small & fast build system used by CMake.
- [ARM Embedded Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads): A collection of tools  (compiler, linker debugger, etc.) used to develop software for ARM Cortex-M MCUs.
- [OpenOCD](https://openocd.org/): Interface for debugging STM32 code with GDB.
- [STLink Drivers](https://www.st.com/en/development-tools/stsw-link009.html): drivers for STM32 debugging probe.
- [Python3](https://www.python.org/downloads/): Scripting language we use to do code generation.

> [!NOTE]
> The rest of the setup guide will use a package manager — a tool to help you easily install the other tools. Although optional, it is highly recommended.


## MacOS Setup
1. [Homebrew](https://brew.sh/): package manager.
	- Add `/opt/homebrew/bin` to your environment path variable to make your tools accessible. Check this [link](https://stackoverflow.com/questions/35677031/adding-homebrew-to-path) for help with the process. Note that your install location may vary.

Run the following commands in your terminal (you can do it all at once or one at a time).
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
