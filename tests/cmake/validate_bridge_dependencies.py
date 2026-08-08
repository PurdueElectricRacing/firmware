#!/usr/bin/env python3
"""Verify that the test bridge uses DAQApp's direct Cargo dependencies."""

import argparse
from pathlib import Path
import tomllib


def normalized_dependency(specification: object) -> object:
    if isinstance(specification, str):
        return specification.removeprefix("=")
    if isinstance(specification, dict):
        normalized = dict(specification)
        if isinstance(normalized.get("version"), str):
            normalized["version"] = normalized["version"].removeprefix("=")
        return normalized
    return specification


def locked_packages(path: Path) -> dict[str, set[tuple[str, str]]]:
    with path.open("rb") as file:
        lock = tomllib.load(file)
    packages: dict[str, set[tuple[str, str]]] = {}
    for package in lock["package"]:
        packages.setdefault(package["name"], set()).add(
            (package["version"], package.get("source", ""))
        )
    return packages


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--daqapp", type=Path, required=True)
    parser.add_argument("--bridge", type=Path, required=True)
    args = parser.parse_args()

    with (args.daqapp / "Cargo.toml").open("rb") as file:
        daq_manifest = tomllib.load(file)
    with (args.bridge / "Cargo.toml").open("rb") as file:
        bridge_manifest = tomllib.load(file)

    daq_dependencies = daq_manifest["dependencies"]
    bridge_dependencies = bridge_manifest["dependencies"]
    for name, bridge_specification in bridge_dependencies.items():
        daq_specification = daq_dependencies.get(name)
        if normalized_dependency(bridge_specification) != normalized_dependency(daq_specification):
            raise RuntimeError(
                f"DAQApp bridge dependency {name!r} drifted: "
                f"bridge={bridge_specification!r}, daqapp={daq_specification!r}"
            )

    daq_locked = locked_packages(args.daqapp / "Cargo.lock")
    bridge_locked = locked_packages(args.bridge / "Cargo.lock")
    for name in bridge_dependencies:
        if not bridge_locked.get(name, set()).issubset(daq_locked.get(name, set())):
            raise RuntimeError(f"DAQApp bridge lockfile dependency {name!r} drifted from DAQApp")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
