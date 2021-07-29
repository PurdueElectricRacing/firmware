#!/usr/bin/python3

# Wrapper for command line tools to build, clean, and debug firmware modules
from optparse import OptionParser
import os
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

parser.add_option("--no_test", 
    dest="no_test",
    action="store_true", default=False,
    help="don't run unit tests"
)

parser.add_option("-j", "--jobs", 
    dest="jobs",
    type="int",
    action="store", default=4,
    help="number of parallel build jobs to run (more is faster)"
)

parser.add_option("-v", "--verbose", 
    dest="verbose",
    action="store_true", default=False,
    help="verbose build commnad output"
)

(options, args) = parser.parse_args()


BUILD_TYPE = "Release" if options.release else "Debug"
TARGET = options.target if options.target else "all"
JOBS = options.jobs
VERBOSE = "--verbose" if options.verbose else ""
RUN_TESTS = not options.no_test # TODO: This


# Always clean if we specify
if options.clean:
    subprocess.run(f"rm -rf {BUILD_DIR} {OUT_DIR}", shell=True)
    print("Build and output directories clean.")

# Build the target if specified or we did not clean
if options.target or not options.clean:
    CMAKE_OPTIONS = [
        f"-S {SOURCE_DIR}",
        f"-B {BUILD_DIR}",
        "-G Ninja",
        f"-DCMAKE_BUILD_TYPE={BUILD_TYPE}",
    ]

    NINJA_OPTIONS = [
        f"{VERBOSE}", 
        f"-j {JOBS}", 
        f"-C {BUILD_DIR}",
        f"{TARGET}",
    ]
    subprocess.run(f"cmake {' '.join(CMAKE_OPTIONS)}", shell=True, check=True)
    ninja_build = subprocess.run(f"ninja {' '.join(NINJA_OPTIONS)}", shell=True)

    if ninja_build.returncode != 0:
        subprocess.run(f"ninja -C {BUILD_DIR} help", shell=True) 
        print(f"\tERROR: Target `{TARGET}` not found. See above for a valid list of components.")



