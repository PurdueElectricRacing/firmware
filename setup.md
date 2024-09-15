1. Package Manager - A tool to help you easily install the other tools.

   - Windows users should install [MSYS2](https://www.msys2.org/): `winget install MSYS2.MSYS2`.
   - MacOS users should install [Homebrew](https://brew.sh/).

2. Add your package manager's `bin/` directory to your environment path variable to make your tools accessible. Note that your install location may vary.

   - Windows: Add `C:\msys64\usr\bin`. Check this [link](https://stackoverflow.com/questions/5733220/how-do-i-add-the-mingw-bin-directory-to-my-system-path) for help with the process.
   - MacOS: Add `/opt/homebrew/bin`. Check this [link](https://stackoverflow.com/questions/35677031/adding-homebrew-to-path) for help with the process.

3. Install [Git](https://git-scm.com/downloads): Tool for managing source code and uploading to GitHub.

   - Windows: `pacman -S git`.
   - MacOS: this should be preinstalled for you. To double-check this, type `git --version` into your terminal. Otherwise, install with `brew install git`.

4. Clone this repository with the following command:

```bash
git clone https://github.com/PurdueElectricRacing/firmware.git
```

5. Install [Visual Studio Code](https://code.visualstudio.com/): Text editor with extensions for helping build the firmware components using CMake.

6. Initialize the git submodules in this project with the following command to download the source for the various git submodules: `git submodule update --init --recursive`.

   - This command needs to be run in the base folder of the cloned repository and may take a few minutes to complete.

7. Install the required python packages with the following command: `pip install -r requirements.txt`.

   - Newer MacOS systems come with python3, you can try using `pip3` instead of `pip`.

8. Install [arm-none-eabi-gcc](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads): Compiler specific to ARM based targets.

   - Windows: `pacman -S mingw-w64-x86_64-arm-none-eabi-gcc`.
   - MacOS: `brew install --cask gcc-arm-embedded`.
   <!-- [Windows](https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.07/gcc-arm-none-eabi-10.3-2021.07win32/gcc-arm-none-eabi-10.3-2021.07-win32.exe)
      - Note: You must manually add this to your path. To do so, open the start menu and select "edit the system environment variables". From here, copy the full file path of your arm-none-eabi-gcc executable into the PATH environment variable (C:\Program Files (x86)\GNU Arm Embedded Toolchain\10 2021.07\bin). Your filepath may not look exactly the same, but it should look similar to this. -->

9. Install [OpenOCD v0.11.0-3](https://github.com/xpack-dev-tools/openocd-xpack/releases/tag/v0.11.0-3/): Open Source On-Chip Debugger used to help GDB debug your code on a STM32 processor.

   - Windows: `pacman -S mingw-w64-x86_64-openocd`.
   <!-- It is extremely important that you install this version of openocd or else you might run into issues with debugging
   Installation Instructions [here](https://xpack.github.io/openocd/install/). Again, use v0.11.0-3 as linked above.-->

   - MacOS: `brew install openocd`.
   <!--If you are on MacOS, you must install the latest version of OpenOcd (v12), or you will run into issues while debugging STM32F7 microcontrollers. To install, simply run -->

10. Install [CMake](https://cmake.org/install/): Build system generator. This takes care of making all of the build files needed to compile the project.

    - Windows: `pacman -S mingw-w64-x86_64-cmake`.
    - MacOS: `brew install cmake`.
    <!-- On some MacOS versions, CMake will install as a GUI only, follow the `Tools > Install Command Line Tools` tip inside CMake to fix this. -->

11. Install [Ninja](https://ninja-build.org/): Small & fast build system used by CMake.

    - Windows: `pacman -S mingw-w64-x86_64-ninja`.
    - MacOS: `brew install ninja`.

12. Install [STLink Drivers](https://www.st.com/en/development-tools/stsw-link009.html) Windows drivers for STM32 debugging probe.
    - Windows: `pacman -S mingw-w64-x86_64-stlink`.
    - MacOS, use `brew install stlink`.
