## Tool Overview
The PER development environment requires the following tools to build, debug, and deploy firmware to the vehicle:
- [Git](https://git-scm.com/downloads): Tool for managing source code and uploading to GitHub.
- [Visual Studio Code](https://code.visualstudio.com/): Text editor with extensions for helping build the firmware components. Notably, we use the following extensions:
	- [Cortex-Debug](https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug): Provides debugging support for ARM Cortex-M microcontrollers, including live watch capability.
	- [clangd](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd): Provides advanced C/C++ code completion and analysis. (Optional, Recommended)
- [CMake](https://cmake.org/install/): Build system generator. This takes care of making all of the build files needed to compile the project.
- [Ninja](https://ninja-build.org/): Small & fast build system used by CMake.
- [OpenOCD](https://openocd.org/): Open Source On-Chip Debugger used to help GDB debug your code on a STM32 processor.
- [STLink Drivers](https://www.st.com/en/development-tools/stsw-link009.html) drivers for STM32 debugging probe.

> [!NOTE]
> The rest of the setup guide will use a package manager — a tool to help you easily install the other tools. Although optional, it is highly recommended.

## MacOS Setup
1. Package Manager: install [Homebrew](https://brew.sh/).
	- Add `/opt/homebrew/bin` to your environment path variable to make your tools accessible. Check this [link](https://stackoverflow.com/questions/35677031/adding-homebrew-to-path) for help with the process. Note that your install location may vary.
2. [Git](https://git-scm.com/downloads):  This should be preinstalled for you. To double-check this, type `git --version` into your terminal. Otherwise, install with `brew install git`.