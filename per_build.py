#!/usr/bin/env python3
"""Build one or more projects and test suites in the PER monorepo."""

import argparse
from pathlib import Path
import subprocess
import sys


ROOT = Path(__file__).resolve().parent
TEST_BUILD = ROOT / "firmware" / "build" / "host-tests"
PROJECT_TARGETS = ["firmware", "daqapp", "tests"]


def test_commands(target_args: list[str]) -> list[tuple[list[str], Path]]:
    parser = argparse.ArgumentParser(prog="per_build.py tests", description="Build and run host tests.")
    parser.add_argument("layer", nargs="?", choices=("all", "unit"), default="all")
    parser.add_argument("--sanitizers", action="store_true", help="enable AddressSanitizer and UBSan")
    args = parser.parse_args(target_args)

    configure = [
        "cmake",
        "-S",
        str(ROOT / "tests"),
        "-B",
        str(TEST_BUILD),
        f"-DPER_TEST_LAYER={args.layer}",
        f"-DPER_TEST_SANITIZERS={'ON' if args.sanitizers else 'OFF'}",
    ]
    build = ["cmake", "--build", str(TEST_BUILD)]
    run_tests = ["ctest", "--test-dir", str(TEST_BUILD), "--output-on-failure"]
    return [(configure, ROOT), (build, ROOT), (run_tests, ROOT)]


def commands_for(target: str, target_args: list[str]) -> list[tuple[list[str], Path]]:
    if target == "firmware":
        return [([sys.executable, "firmware_build.py", *target_args], ROOT / "firmware")]
    if target == "daqapp":
        return [(["cargo", "build", *target_args], ROOT / "daqapp")]
    if target == "tests":
        return test_commands(target_args)
    raise ValueError(f"Unknown project target: {target}")


def run_target(target: str, target_args: list[str]) -> None:
    print("\nBuilding project:", target)
    print("=" * 80)
    for command, working_directory in commands_for(target, target_args):
        print(f"$ (cd {working_directory} && {' '.join(command)})", flush=True)
        subprocess.run(command, cwd=working_directory, check=True)


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Build projects and host tests in the PER monorepo.",
        epilog=(
            "Examples:\n"
            "  python3 per_build.py                      # build every project\n"
            "  python3 per_build.py firmware --package   # pass options to firmware_build.py\n"
            "  python3 per_build.py daqapp --all-targets # pass options to cargo build\n"
            "  python3 per_build.py tests unit           # run the host unit tests\n"
            "  python3 per_build.py tests --sanitizers   # run host tests with sanitizers"
        ),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "target",
        nargs="?",
        choices=(*PROJECT_TARGETS, "all"),
        default="all",
        help="project or test target to build (default: all)",
    )
    parser.add_argument(
        "target_args",
        nargs=argparse.REMAINDER,
        help="arguments forwarded to the selected project or test runner",
    )
    args = parser.parse_args()

    targets = PROJECT_TARGETS if args.target == "all" else (args.target,)
    if args.target == "all" and args.target_args:
        parser.error("arguments can only be forwarded when a single target is selected")

    try:
        for target in targets:
            run_target(target, args.target_args)
    except subprocess.CalledProcessError as error:
        return error.returncode
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
