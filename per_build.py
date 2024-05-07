#!/usr/bin/python3

# Wrapper for command line tools to build, clean, and debug firmware modules
from optparse import OptionParser
import pathlib
import subprocess
import os

# Logging helper functions
class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

def log_error(phrase):
    print(f"{bcolors.FAIL}ERROR: {phrase}{bcolors.ENDC}")

def log_warning(phrase):
    print(f"{bcolors.WARNING}WARNING: {phrase}{bcolors.ENDC}")

def log_success(phrase):
    print(f"{bcolors.OKGREEN}{phrase}{bcolors.ENDC}")


# Get build directory path
CWD = pathlib.Path.cwd()
BUILD_DIR = CWD/"build"
SOURCE_DIR = CWD
OUT_DIR = CWD/"output"

# Setup cli arguments
parser = OptionParser()

parser.add_option("-t", "--target", 
    dest="target",
    type="string",
    action="store",
    help="firmware target to build. Defaults to `all`"
)

parser.add_option("--clean",
    dest="clean",
    action="store_true", default=False,
    help="remove build artifacts"
)

parser.add_option("--release",
    dest="release",
    action="store_true", default=False,
    help="build for release (optimized)"
)

parser.add_option("--no-test", 
    dest="no_test",
    action="store_true", default=False,
    help="don't run unit tests"
)

parser.add_option("-v", "--verbose", 
    dest="verbose",
    action="store_true", default=False,
    help="verbose build commnad output"
)

(options, args) = parser.parse_args()


BUILD_TYPE = "Release" if options.release else "Debug"
TARGET = options.target if options.target else "all"
VERBOSE = "--verbose" if options.verbose else ""
RUN_TESTS = not options.no_test # TODO: This

# Combine HEX files into a single firmware.hex
# NOTE - No additional memory address offsets are needed since each node has its own memory space
def generate_firmware_hex():
    print("Generating firmware.hex")
    # List of nodes to combine, these must match the directory names in the output directory
    NODES = ["main_module", "dashboard", "pdu", "a_box", "daq", "torque_vector"]
    combined_hex_data = []
    firmware_output_dir = OUT_DIR/"firmware"

    # Create the firmware output directory if it does not exist
    try: 
        os.mkdir(firmware_output_dir)
    except FileExistsError:
        pass

    try:
        # Read each hex file and append to the combined hex data
        for node in NODES:
            hex_file = f"{OUT_DIR}/{node}/BL_{node}.hex"
            with open(hex_file, "r") as hex_file:
                hex_data = hex_file.read()
                combined_hex_data.append(hex_data)
                combined_hex_data.append("\n") # Add new line between hex files
        
        # Write the combined hex data to the firmware.hex file
        with open(firmware_output_dir/"firmware.hex", "w") as firmware_hex:
            firmware_hex.writelines(combined_hex_data)
        log_success("Sucessfully generated firmware.hex")
    except FileNotFoundError as e:
        log_error("Unable to find hex files for all nodes.")
        exit()

# Always clean if we specify
if options.clean:
    subprocess.run(["cmake", "-E", "rm", "-rf", str(BUILD_DIR), str(OUT_DIR)])
    print("Build and output directories clean.")

# Build the target if specified or we did not clean
if options.target or not options.clean:
    CMAKE_OPTIONS = [
        "-S", str(SOURCE_DIR),
        "-B", str(BUILD_DIR),
        "-G", "Ninja",
        f"-DCMAKE_BUILD_TYPE={BUILD_TYPE}",
    ]

    NINJA_OPTIONS = [
        "-C", str(BUILD_DIR),
        TARGET,
    ]
    NINJA_COMMAND = ["ninja"] + NINJA_OPTIONS 

    try:
        subprocess.run(["cmake"] + CMAKE_OPTIONS, check=True)
    except subprocess.CalledProcessError as e:
        log_error("Unable to configure CMake, see the CMake output above.")
        exit()

    log_success("Sucessfully generated build files.")
    print(f"Running Build command {' '.join(NINJA_COMMAND)}")

    try:
        ninja_build = subprocess.run(NINJA_COMMAND)
    except subprocess.CalledProcessError as e:
        log_error("Unable to configure compile sources, see the Ninja output above.")
        exit()

    if ninja_build.returncode != 0:
        log_error("Unable to generate targets.")
    else:
        log_success("Sucessfully built targets.")
        generate_firmware_hex()
