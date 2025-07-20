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

## Install Extensions
Open the Extensions view (Ctrl+Shift+X or Cmd+Shift+X) and install the [PER extension pack](https://marketplace.visualstudio.com/items?itemName=irvingywang.per-pack) by searching the extension marketplace for "PER-Pack".
> [!WARNING]
> PER-Pack is required! Dont skip this step.

## Turn on Autosave
Go to `File -> Autosave` and click check to turn on autosave.


# 3. Repository Setup (All OS)

Follow these steps to download the PER codebase and get started on development:


## Open a new terminal
1. Open a new terminal. Do *not* run as Administrator. Just use your regular user account. 

For Windows, open PowerShell (not as Administrator).

You should land in your home directory:
```bash
C:\Users\{username}\
```

For MacOS:
```bash
/Users/{username}
```

You can confirm with:
```bash
pwd
```

## Clone PER repo
2. Once you're in your home directory, clone the PER firmware repository with the following command:
```bash
git clone https://github.com/PurdueElectricRacing/firmware.git
```

3. Enter the firmware repository:
```bash
cd firmware
```

## Setup & Build
4. Install the required Python packages:
```bash
pip3 install -r requirements.txt
```

5. Launch Visual Studio Code:
```bash
code .
```
6. Try running a build by doing `CTRL/CMD + Shift + B` in your VSCode window

```
Ctrl + Shift + B on Windows/Linux
Cmd + Shift + B on macOS
```

All done!
