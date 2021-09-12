#!/usr/bin/python3

# Wrapper for command line tools to build, clean, and debug firmware modules
from optparse import OptionParser
import pathlib
import subprocess

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


# Always clean if we specify
if options.clean:
    subprocess.run(["cmake", "-E", "rm", "-rf", BUILD_DIR, OUT_DIR])
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

    subprocess.run(["cmake"] + CMAKE_OPTIONS, check=True)
    print(f"Running Build command {' '.join(NINJA_COMMAND)}")
    ninja_build = subprocess.run(NINJA_COMMAND)

    if ninja_build.returncode != 0:
        subprocess.run(["ninja", "-C", BUILD_DIR, "help"]) 
        print(f"\tERROR: Target `{TARGET}` not found. See above for a valid list of components.")
