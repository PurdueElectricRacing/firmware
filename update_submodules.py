#!/usr/bin/env python3

import os
import sys
import subprocess
import shutil
from pathlib import Path

# define submodules, family names
submodules = [
    ("common/STM32CubeF4", "F4"),
    ("common/STM32CubeF7", "F7"),
    ("common/STM32CubeG4", "G4"),
    ("common/STM32CubeL4", "L4"),
]

# define the directories we want to keep
sparse_checkout_settings = """
Drivers/CMSIS/
!Drivers/CMSIS/docs/

Middlewares/Third_Party/FreeRTOS/Source/
!Middlewares/Third_Party/FreeRTOS/Source/Demo/

Drivers/CMSIS/RTOS2/
Drivers/CMSIS/RTOS/
"""

def run_command(cmd, cwd=None, check=True):
    """Run a shell command and return the result"""
    try:
        return subprocess.run(cmd, shell=True, cwd=cwd, check=check, capture_output=True, text=True)
    except subprocess.CalledProcessError as e:
        print(f"Error running command: {cmd}, {e.stderr}")
        if check:
            sys.exit(1)
        return e
    
def deinit_submodule(submodule_path):
    if Path(submodule_path).exists():
        print("Deinitializing existing submodule")
        run_command(f"git submodule deinit -f {submodule_path}", check=False)
        shutil.rmtree(submodule_path, ignore_errors=True)

def setup_submodule(submodule_path, family):
    """Setup a sparse checkout submodule"""
    print(f"Setting up {submodule_path}")

    run_command(f"git submodule init {submodule_path}")
    run_command(f"git submodule update {submodule_path}")

    start_dir = os.getcwd() # save root dir

    os.chdir(submodule_path)
    
    try:
        run_command("git config core.sparseCheckout true")
        
        result = run_command("git rev-parse --git-dir")
        git_dir = result.stdout.strip()
        
        info_dir = Path(git_dir) / "info"
        info_dir.mkdir(parents=True, exist_ok=True)
        
        sparse_checkout_file = info_dir / "sparse-checkout"
        sparse_checkout_file.write_text(sparse_checkout_settings)
        
        run_command("git checkout HEAD")
        
    finally:
        os.chdir(start_dir) # reset to root dir
    
    print(f"{submodule_path} initialized")


def g4_princess_treatment():
    """G4 needs special init cuz it's submodule is weird"""
    print("Initializing G4 nested submodules...")
    start_dir = os.getcwd()
    os.chdir("common/STM32CubeG4")
    
    try:
        run_command("git submodule update --init Drivers/CMSIS/Device/ST/STM32G4xx")
    finally:
        os.chdir(start_dir)

    print("G4 nested submodules initialized")

def main():
    if not (Path("common").exists()):
        print("Error: Run this script from the firmware root directory")
        sys.exit(1)
    
    for (submodule_path, family) in submodules:
        deinit_submodule(submodule_path)
        setup_submodule(submodule_path, family)
    
    g4_princess_treatment() # idk why its special
    
    print("")
    print("STM32Cube submodules initialized")

if __name__ == "__main__":
    main()
