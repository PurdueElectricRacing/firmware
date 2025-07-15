#!/usr/bin/env python3

import subprocess
import shutil
from pathlib import Path

submodules = [
    ("external/STM32CubeF4", "Drivers/CMSIS/Device/ST/STM32F4xx"),
    ("external/STM32CubeF7", "Drivers/CMSIS/Device/ST/STM32F7xx"), 
    ("external/STM32CubeG4", "Drivers/CMSIS/Device/ST/STM32G4xx"),
    ("external/STM32CubeL4", "Drivers/CMSIS/Device/ST/STM32L4xx"),
]

# Edit these settings to add or remove files from the sparse checkout
sparse_checkout_settings = """
# CMSIS headers, register maps, RTOS2
Drivers/CMSIS/Device/
Drivers/CMSIS/RTOS2/
Drivers/CMSIS/Core/Include/

Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/
Middlewares/Third_Party/FreeRTOS/Source/include/
Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/
Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/
Middlewares/Third_Party/FreeRTOS/Source/*.c
"""

def run(cmd, check=True):
    subprocess.run(cmd, shell=True, check=check)

def main():
    if not Path("external").exists():
        print("Error: Run this script from the firmware root directory")
        exit(1)

    # deinit existing submodules
    for (family_module, device_module) in submodules:
        if Path(family_module).exists():
            run(f"git submodule deinit -f {family_module}", check=False)
            shutil.rmtree(family_module, ignore_errors=True)

    run("git submodule update --init --filter=blob:none")
    
    # setup each submodule with sparse checkout
    for (family_module, device_module) in submodules:
        print(f"Setting up {family_module}")
        run(f"git -C {family_module} config core.sparseCheckout true")

        result = subprocess.run(f"git -C {family_module} rev-parse --git-dir", shell=True, capture_output=True, text=True, check=True)
        git_dir = Path(family_module) / result.stdout.strip()
        
        sparse_file = git_dir / "info" / "sparse-checkout"
        sparse_file.parent.mkdir(parents=True, exist_ok=True)
        sparse_file.write_text(sparse_checkout_settings)
        
        run(f"git -C {family_module} read-tree -m -u HEAD")

    # checkout device modules
    for (family_module, device_module) in submodules:
        if Path(family_module).exists():
            print(f"Setting up cmsis-device submodule in {family_module}")
            run(f"git -C {family_module} submodule update --init {device_module}")

    print("All submodules initialized")

if __name__ == "__main__":
    main()