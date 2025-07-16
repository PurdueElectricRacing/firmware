#!/usr/bin/python3

# Wrapper for command line tools to build, clean, and debug firmware modules
from optparse import OptionParser
import pathlib
import subprocess
import tarfile
import zlib

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
    dest="targets",
    type="string",
    action="store",
    help="comma seperated list of firmware targets to build. Defaults to `all`"
)

parser.add_option("-c", "--clean",
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

parser.add_option("-b", "--bootloader",
    dest="bootloader",
    action="store_true", default=False,
    help="build bootloader components"
)

parser.add_option("-v", "--verbose",
    dest="verbose",
    action="store_true", default=False,
    help="verbose build commnad output"
)

parser.add_option("-p", "--package",
    dest="package",
    action="store_true", default=False,
    help="package build output into tarball with CRCs, suffixed by Git hash"
)

(options, args) = parser.parse_args()


BUILD_TYPE = "Release" if options.release else "Debug"
VERBOSE = "--verbose" if options.verbose else ""
RUN_TESTS = not options.no_test # TODO: This

# Auto-append .elf to each target unless already present
if options.targets:
    TARGETS = [t if t.endswith(".elf") else f"{t}.elf" for t in options.targets.split(",")]
else:
    TARGETS = ["all"]

# Always clean if we specify
if options.clean or options.package:
    subprocess.run(["cmake", "-E", "rm", "-rf", str(BUILD_DIR), str(OUT_DIR)])
    print("Build and output directories clean.")

# Build the target if specified or we did not clean
if options.targets or not options.clean:
    CMAKE_OPTIONS = [
        "-S", str(SOURCE_DIR),
        "-B", str(BUILD_DIR),
        "-G", "Ninja",
        f"-DCMAKE_BUILD_TYPE={BUILD_TYPE}",
        f"-DBOOTLOADER_BUILD={'ON' if options.bootloader else 'OFF'}",
    ]

    NINJA_OPTIONS = [
        "-C", str(BUILD_DIR),
    ] + TARGETS
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

# --package logic
def get_git_hash_or_tag():
    try:
        # Check if current commit has a tag
        tag = subprocess.check_output(
            ["git", "describe", "--tags", "--exact-match"],
            stderr=subprocess.DEVNULL
        ).strip().decode()
        return tag
    except subprocess.CalledProcessError:
        # No tag on this commit, fallback to short hash
        return subprocess.check_output(
            ["git", "rev-parse", "--short", "HEAD"]
        ).strip().decode()


def add_crc_to_files():
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    for elf in OUT_DIR.glob("*/*.elf"):
        with open(elf, "rb") as f:
            data = f.read()
            crc = format(zlib.crc32(data) & 0xFFFFFFFF, '08X')
        crc_file = elf.with_suffix(".crc")
        with open(crc_file, "w") as cf:
            cf.write(crc + "\n")
        log_success(f"CRC written for {elf.name}: {crc}")

def create_tarball():
    git_hash = get_git_hash_or_tag()
    tarball_name = OUT_DIR / f"firmware_{git_hash}.tar.gz"

    with tarfile.open(tarball_name, "w:gz") as tar:
        for f in OUT_DIR.glob("*/*"):
            if f.suffix in [".elf", ".crc"]:
                tar.add(f, arcname=f.name)

    log_success(f"Tarball created: {tarball_name}")
    return tarball_name

# Package output if requested
if options.package:
    log_success("Packaging firmware...")
    add_crc_to_files()
    create_tarball()
