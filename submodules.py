#!/usr/bin/env python3

import subprocess
import shutil
from pathlib import Path

SUBMODULES = [
    "common/STM32CubeF4",
    "common/STM32CubeF7", 
    "common/STM32CubeG4",
    "common/STM32CubeL4"
]

sparse_checkout_settings = """
Drivers/CMSIS/
!Drivers/CMSIS/docs/
Middlewares/Third_Party/FreeRTOS/Source/
!Middlewares/Third_Party/FreeRTOS/Source/Demo/
Drivers/CMSIS/RTOS2/
Drivers/CMSIS/RTOS/
"""

def run(cmd, check=True):
    subprocess.run(cmd, shell=True, check=check)

def main():
    if not Path("common").exists():
        print("Error: Run this script from the firmware root directory")
        exit(1)

    # Deinitialize existing submodules
    for submodule in SUBMODULES:
        if Path(submodule).exists():
            run(f"git submodule deinit -f {submodule}", check=False)
            shutil.rmtree(submodule, ignore_errors=True)

    run("git submodule update --init --filter=blob:none")
    
    # setup each submodule with sparse checkout
    for submodule in SUBMODULES:
        print(f"Setting up {submodule}")
        run(f"git -C {submodule} config core.sparseCheckout true")

        result = subprocess.run(f"git -C {submodule} rev-parse --git-dir", shell=True, capture_output=True, text=True, check=True)
        git_dir = Path(submodule) / result.stdout.strip()
        
        sparse_file = git_dir / "info" / "sparse-checkout"
        sparse_file.parent.mkdir(parents=True, exist_ok=True)
        sparse_file.write_text(sparse_checkout_settings)
        
        run(f"git -C {submodule} read-tree -m -u HEAD")
    
    # G4 special case
    run("git -C common/STM32CubeG4 submodule update --init Drivers/CMSIS/Device/ST/STM32G4xx")
    
    print("Done")

if __name__ == "__main__":
    main()