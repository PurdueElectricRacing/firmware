# PER Firmware Setup Instructions

# 1. Tools

The PER firmware development environment relies on several tools -- such as `cmake`, `ninja`, and `stlink` -- to build, debug, flash, and deploy code to the vehicle. To streamline setup, we (highly) recommend using a package manager compatible with your operating system. Package managers help manage dependencies and ensure tools are correctly installed and updated. Below are supported package manager setups for each OS (macOS, Windows, Linux):


## MacOS Tools Setup
1. [Homebrew](https://brew.sh/): macOS package manager. Open a terminal (e.g., iTerm) and paste and run the installation command provided on the homebrew install page.
	- After installing Homebrew, make sure to add /opt/homebrew/bin to your system’s PATH environment variable. The exact command should be printed at the end of the homebrew install.
	- After installation, ensure brew is installed by running:
	`brew --version`

2. Run the following commands in your terminal to install the other tools (you can copy and paste them all at once).
```bash
brew install git cmake ninja openocd stlink python3
brew install --cask gcc-arm-embedded
```

3. Download VSCode from the website: https://code.visualstudio.com/docs/setup/mac

## Windows Tools Setup
1. [Chocolatey](https://chocolatey.org/install#install-step2): Windows package manager Paste and run the installation command provided on the Chocolatey install page.
	- Open the Start Menu, scroll to W, and locate Windows PowerShell. Right-click on PowerShell and select "Run as Administrator".
	- After installation, confirm choco is installed by running:
	`choco --version`
2. In your administrator powershell, paste the following commands (right click to paste in powershell terminal).

```bash
choco install git cmake ninja python3 gcc-arm-embedded openocd 
```
- Enter (`A`) on the first prompt to select 'Yes to All'

> [!NOTE]
> Make sure to run these in PowerShell as Administrator!

3. STLink drivers need to be manually installed from [here](https://www.st.com/en/development-tools/stsw-link009.html).

4. Download VSCode from the website: https://code.visualstudio.com/download


## Linux Tools Setup
1. You probably already know what you're doing, so here are the commands for `apt` (Ubuntu, Debian, Pop!, etc.):
```bash
sudo apt update && sudo apt upgrade
sudo apt install git cmake python3 python3-pip ninja-build gcc-arm-none-eabi openocd stlink-tools
```
2. Visual Studio Code requires some special attention, install from [here](https://code.visualstudio.com/docs/setup/linux).


# 2. VSCode Setup (All OS)

VS Code is the recommended editor for firmware development.

Install the following VSCode extensions for debugging, inspection, and code editing:

```
Cortex-Debug
Peripheral Viewer
MemoryView
C/C++
RTOS Views
Black Formatter
Code Spell Check
Python
Python Debug
Pylance
```


# 3. Repository Setup (All OS)

Follow these steps to download the PER codebase and get started on development:

1. Clone the repository with submodules. Open your terminal, and clone this repository with the following command:
```bash
git clone --recurse-submodules https://github.com/PurdueElectricRacing/firmware.git
```
2. Install required Python packages:
```bash
cd firmware
pip3 install -r requirements.txt
```
3. Launch Visual Studio Code:
```bash
code .
```
4. Try running a build by doing `CTRL/CMD + Shift + B` in your VSCode window

```
Ctrl + Shift + B on Windows/Linux
Cmd + Shift + B on macOS
```

All done!
