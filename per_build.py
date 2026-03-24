#!/usr/bin/python3

# Wrapper for command line tools to build, clean, and debug firmware modules
from optparse import OptionParser
import pathlib
import subprocess
import sys
import tarfile
import struct
import json
import io


# Logging helper functions
class bcolors:
    HEADER = "\033[95m"
    OKBLUE = "\033[94m"
    OKCYAN = "\033[96m"
    OKGREEN = "\033[92m"
    WARNING = "\033[93m"
    FAIL = "\033[91m"
    ENDC = "\033[0m"
    BOLD = "\033[1m"
    UNDERLINE = "\033[4m"


def log_error(phrase):
    print(f"{bcolors.FAIL}ERROR: {phrase}{bcolors.ENDC}")


def log_warning(phrase):
    print(f"{bcolors.WARNING}WARNING: {phrase}{bcolors.ENDC}")


def log_success(phrase):
    print(f"{bcolors.OKGREEN}{phrase}{bcolors.ENDC}")


BOARD_TARGETS = [
    "main_module",
    "a_box",
    "torque_vector",
    "dashboard",
    "pdu",
    "daq",
    "front_driveline",
    "rear_driveline",
]

TARGET_ALIASES = {
    "front_driveline": "driveline",
    "rear_driveline": "driveline",
}

BOOTLOADER_TARGETS = [
    "bootloader_MAIN_MODULE",
    "bootloader_DASHBOARD",
    "bootloader_A_BOX",
    "bootloader_TORQUE_VECTOR",
    "bootloader_DRIVELINE",
    "bootloader_G4_TESTING",
    "bootloader_PDU",
]

# Map board names to bootloader names and CAN node names
BOARD_TO_BOOTLOADER = {
    "main_module": {
        "bootloader": "bootloader_MAIN_MODULE",
        "bl_node": "BL_MAIN_MODULE",
        "can_node": "MAIN_MODULE",
        "arch": "G4",
    },
    "dashboard": {
        "bootloader": "bootloader_DASHBOARD",
        "bl_node": "BL_DASHBOARD",
        "can_node": "DASHBOARD",
        "arch": "G4",
    },
    "a_box": {
        "bootloader": "bootloader_A_BOX",
        "bl_node": "BL_A_BOX",
        "can_node": "A_BOX",
        "arch": "G4",
    },
    "torque_vector": {
        "bootloader": "bootloader_TORQUE_VECTOR",
        "bl_node": "BL_TORQUE_VECTOR",
        "can_node": "TORQUE_VECTOR",
        "arch": "G4",
    },
    "driveline": {
        "bootloader": "bootloader_DRIVELINE",
        "bl_node": "BL_DRIVELINE",
        "can_node": "DRIVELINE",
        "arch": "G4",
    },
    "front_driveline": {
        "bootloader": "bootloader_DRIVELINE",
        "bl_node": "BL_DRIVELINE",
        "can_node": "DRIVELINE",
        "arch": "G4",
    },
    "rear_driveline": {
        "bootloader": "bootloader_DRIVELINE",
        "bl_node": "BL_DRIVELINE",
        "can_node": "DRIVELINE",
        "arch": "G4",
    },
    "pdu": {
        "bootloader": "bootloader_PDU",
        "bl_node": "BL_PDU",
        "can_node": "PDU",
        "arch": "F4",
    },
    "g4_testing": {
        "bootloader": "bootloader_G4_TESTING",
        "bl_node": "BL_G4_TESTING",
        "can_node": "G4_TESTING",
        "arch": "G4",
    },
}

# Flash layout constants
BL_SIZE = 16 * 1024  # 16KB bootloader
METADATA_SIZE = 16 * 1024  # 16KB metadata
APP_START = 0x08008000  # App starts after bootloader + metadata

# Get build directory path
CWD = pathlib.Path.cwd()
BUILD_DIR = CWD / "build"
SOURCE_DIR = CWD
OUT_DIR = CWD / "output"
CAN_GEN_DIR = SOURCE_DIR / "common" / "can_library" / "generated"

# Setup cli arguments
parser = OptionParser()

parser.add_option(
    "-t",
    "--target",
    type="string",
    help="Space-separated list of board targets to build",
)

parser.add_option(
    "-l",
    "--list",
    action="store_true",
    default=False,
    help="List boards targets available to build",
)

parser.add_option(
    "-b",
    "--bootloader",
    dest="bootloader",
    action="store_true",
    default=False,
    help="build bootloader components",
)

parser.add_option(
    "-v",
    "--verbose",
    dest="verbose",
    action="store_true",
    default=False,
    help="verbose build command output",
)

parser.add_option(
    "-p",
    "--package",
    dest="package",
    action="store_true",
    default=False,
    help="package selected build output for daqapp2 flashing into tarball",
)


def print_available_targets():
    boards = sorted(set(BOARD_TARGETS + ["driveline"]))
    print("Available board targets to build:")
    for board in boards:
        print(f"\t{board}")


(options, args) = parser.parse_args()
if options.list:
    # User ran `-t` with no argument: print available targets
    print_available_targets()
    sys.exit(0)

VERBOSE = "--verbose" if options.verbose else ""


def cmake_module_for_target(target):
    if target in BOOTLOADER_TARGETS or target == "bootloader":
        return "bootloader"
    return TARGET_ALIASES.get(target, target)


def resolve_requested_boards(raw_target_arg):
    """Return an ordered, de-duplicated list of board targets to build."""
    if not raw_target_arg:
        return list(BOARD_TARGETS)

    requested_boards = []
    invalid_targets = []

    for token in raw_target_arg.split():
        expanded = None
        if token == "driveline":
            expanded = ["front_driveline", "rear_driveline"]
        elif token in BOARD_TARGETS:
            expanded = [token]

        if expanded is None:
            invalid_targets.append(token)
            continue

        for board in expanded:
            if board not in requested_boards:
                requested_boards.append(board)

    if invalid_targets:
        log_error(f"Unknown board target(s): {' '.join(invalid_targets)}")
        print_available_targets()
        sys.exit(1)

    return requested_boards


selected_boards = resolve_requested_boards(options.target)

cmake_module_targets = []
for board in selected_boards:
    module = cmake_module_for_target(board)
    if module not in cmake_module_targets:
        cmake_module_targets.append(module)

selected_bootloader_targets = []
if options.bootloader:
    if "bootloader" not in cmake_module_targets:
        cmake_module_targets.append("bootloader")

    for board in selected_boards:
        board_info = BOARD_TO_BOOTLOADER.get(board)
        if not board_info:
            continue
        bl_target = board_info["bootloader"]
        if bl_target not in selected_bootloader_targets:
            selected_bootloader_targets.append(bl_target)

cmake_modules_str = ";".join(cmake_module_targets)
ninja_targets = [f"{board}.elf" for board in selected_boards] + [
    f"{bootloader}.elf" for bootloader in selected_bootloader_targets
]

# Always clean for a fresh build environment
subprocess.run(
    ["cmake", "-E", "rm", "-rf", str(BUILD_DIR), str(OUT_DIR), str(CAN_GEN_DIR)]
)
print("Build, output, and generated CAN directories clean.")

# Configure and Build
CMAKE_OPTIONS = [
    "-S",
    str(SOURCE_DIR),
    "-B",
    str(BUILD_DIR),
    "-G",
    "Ninja",
    f"-DBOOTLOADER_BUILD={'ON' if options.bootloader else 'OFF'}",
    f"-DMODULES={cmake_modules_str}",
]

NINJA_OPTIONS = ["-C", str(BUILD_DIR)] + ninja_targets
NINJA_COMMAND = ["ninja"] + NINJA_OPTIONS

try:
    subprocess.run(["cmake"] + CMAKE_OPTIONS, check=True)
except subprocess.CalledProcessError as e:
    log_error("Unable to configure CMake, see the CMake output above.")
    sys.exit(1)

log_success("Sucessfully generated build files.")
print(f"Running Build command {' '.join(NINJA_COMMAND)}")

try:
    ninja_build = subprocess.run(NINJA_COMMAND)
except subprocess.CalledProcessError as e:
    log_error("Unable to configure compile sources, see the Ninja output above.")
    sys.exit(1)

if ninja_build.returncode != 0:
    log_error("Unable to generate targets.")
    sys.exit(1)
else:
    log_success("Sucessfully built targets.")


# Post-build helpers
def get_git_hash_or_tag():
    try:
        tag = (
            subprocess.check_output(
                ["git", "describe", "--tags", "--exact-match"],
                stderr=subprocess.DEVNULL,
            )
            .strip()
            .decode()
        )
        return tag
    except subprocess.CalledProcessError:
        return (
            subprocess.check_output(["git", "rev-parse", "--short", "HEAD"])
            .strip()
            .decode()
        )


def crc32_stm32(data):
    """
    Calculate CRC32 matching STM32 hardware CRC peripheral.
    Uses Ethernet polynomial 0x04C11DB7, initial value 0xFFFFFFFF.
    This matches the bootloader's PHAL_CRC32_Calculate function.
    """
    crc = 0xFFFFFFFF
    for i in range(0, len(data), 4):
        word_bytes = data[i : i + 4]
        if len(word_bytes) < 4:
            word_bytes = word_bytes + b"\x00" * (4 - len(word_bytes))
        word = struct.unpack("<I", word_bytes)[0]
        crc ^= word
        for _ in range(32):
            if crc & 0x80000000:
                crc = ((crc << 1) ^ 0x04C11DB7) & 0xFFFFFFFF
            else:
                crc = (crc << 1) & 0xFFFFFFFF
    return crc


def create_combined_binary(bootloader_bin, app_bin):
    """
    Create a combined binary with bootloader + metadata + app.
    Layout:
      0x00000 - 0x03FFF  Bootloader (16 KB)
      0x04000 - 0x07FFF  Metadata   (16 KB)
      0x08000 - ...      Application

    Metadata words at BL_ADDRESS_CRC:
      [0x00] CRC32 (STM32 peripheral compatible)
      [0x04] Application start address
      [0x08] Application size in bytes (4-byte aligned)
    """
    if len(bootloader_bin) > BL_SIZE:
        raise ValueError(
            f"Bootloader binary size {len(bootloader_bin)} exceeds reserved {BL_SIZE} bytes"
        )

    # Bootloader validates size as 4-byte aligned and computes CRC over 32-bit words.
    # For direct-flashed combined images, ensure app payload is padded accordingly.
    app_size_aligned = (len(app_bin) + 3) & ~0x3
    if app_size_aligned != len(app_bin):
        app_bin = app_bin + (b"\xff" * (app_size_aligned - len(app_bin)))

    app_crc = crc32_stm32(app_bin)

    metadata = bytearray(b"\xff" * METADATA_SIZE)
    metadata[0:4] = struct.pack("<I", app_crc)
    metadata[4:8] = struct.pack("<I", APP_START)
    metadata[8:12] = struct.pack("<I", app_size_aligned)

    combined = bytearray()
    combined.extend(bootloader_bin)
    if len(combined) < BL_SIZE:
        combined.extend(b"\xff" * (BL_SIZE - len(combined)))
    combined.extend(metadata)
    combined.extend(app_bin)
    return bytes(combined)


def ensure_bin_artifact(target_dir, stem, required=False):
    """Return a .bin path for target stem, generating it if needed."""
    bin_path = target_dir / f"{stem}.bin"
    if bin_path.exists():
        return bin_path

    elf_path = target_dir / f"{stem}.elf"
    hex_path = target_dir / f"{stem}.hex"

    # Some bootloader-oriented builds emit BL_<stem>.hex/.elf.
    if not hex_path.exists() and not elf_path.exists():
        bl_stem = f"BL_{stem}"
        alt_elf = target_dir / f"{bl_stem}.elf"
        alt_hex = target_dir / f"{bl_stem}.hex"
        if alt_hex.exists() or alt_elf.exists():
            elf_path = alt_elf
            hex_path = alt_hex

    if hex_path.exists():
        try:
            hex_records = {}
            upper_linear = 0
            with open(hex_path, "r") as hf:
                for raw_line in hf:
                    line = raw_line.strip()
                    if not line:
                        continue
                    if not line.startswith(":"):
                        raise ValueError("Invalid HEX record")

                    rec = bytes.fromhex(line[1:])
                    length = rec[0]
                    addr = (rec[1] << 8) | rec[2]
                    rec_type = rec[3]
                    data = rec[4 : 4 + length]

                    if rec_type == 0x00:
                        abs_addr = upper_linear + addr
                        for i, b in enumerate(data):
                            target_addr = abs_addr + i
                            if 0x08000000 <= target_addr <= 0x080FFFFF:
                                hex_records[target_addr] = b
                    elif rec_type == 0x01:
                        break
                    elif rec_type == 0x04:
                        upper_linear = ((data[0] << 8) | data[1]) << 16

            if not hex_records:
                raise ValueError("No data records in HEX")

            min_addr = min(hex_records.keys())
            max_addr = max(hex_records.keys())
            blob = bytearray(b"\xff" * (max_addr - min_addr + 1))
            for addr, val in hex_records.items():
                blob[addr - min_addr] = val

            with open(bin_path, "wb") as bf:
                bf.write(blob)

            log_success(f"Generated {bin_path.name} from {hex_path.name}")
            return bin_path
        except Exception as e:
            log_warning(
                f"Failed to convert {hex_path.name} to .bin ({e}); falling back to ELF"
            )
    else:
        log_warning(f"Missing {hex_path.name}; trying ELF to generate .bin")

    if elf_path.exists():
        try:
            subprocess.run(
                ["arm-none-eabi-objcopy", "-O", "binary", str(elf_path), str(bin_path)],
                check=True,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.PIPE,
            )
            log_success(f"Generated {bin_path.name} from {elf_path.name}")
            return bin_path
        except subprocess.CalledProcessError as e:
            err = e.stderr.decode(errors="replace").strip() if e.stderr else "unknown"
            log_warning(f"Failed to convert {elf_path.name} to .bin ({err})")
    else:
        log_warning(f"Missing {elf_path.name}; cannot use ELF fallback")

    msg = f"Unable to produce {bin_path.name}: no usable HEX/ELF source in {target_dir}"
    if required:
        log_error(msg)
    else:
        log_warning(msg)

    return None


def compute_crcs_and_manifest(selected_boards):
    """Compute CRC for selected app .bin files and return a manifest dict."""
    if not OUT_DIR.exists():
        log_error(f"Output directory does not exist: {OUT_DIR}")
        sys.exit(1)

    manifest = {}

    for board in selected_boards:
        board_dir = OUT_DIR / board
        bin_path = ensure_bin_artifact(board_dir, board)
        if bin_path is None or not bin_path.exists():
            continue

        with open(bin_path, "rb") as f:
            data = f.read()

        crc = crc32_stm32(data)
        crc_hex = format(crc, "08X")

        crc_file = bin_path.with_suffix(".crc")
        with open(crc_file, "w") as cf:
            cf.write(crc_hex + "\n")
        log_success(f"CRC written for {bin_path.name}: {crc_hex}")

        board_info = BOARD_TO_BOOTLOADER.get(board, {})
        manifest[board] = {
            "bin": f"{board}.bin",
            "crc": crc_hex,
            "size": len(data),
            "bootloader": board_info.get("bootloader", ""),
            "bl_node": board_info.get("bl_node", ""),
            "can_node": board_info.get("can_node", ""),
            "arch": board_info.get("arch", ""),
        }

    return manifest


def create_combined_binaries(selected_boards, selected_bootloader_targets):
    """
    For every board that has BOTH a bootloader .bin and an app .bin,
    produce <board>_combined.bin  (bootloader + metadata + golden app).
    """
    if not OUT_DIR.exists():
        log_error(f"Output directory does not exist: {OUT_DIR}")
        return set()

    generated_for_boards = set()

    for board in selected_boards:
        board_info = BOARD_TO_BOOTLOADER.get(board)
        if not board_info:
            continue

        bl_name = board_info["bootloader"]
        if bl_name not in selected_bootloader_targets:
            continue

        bl_path = ensure_bin_artifact(
            OUT_DIR / bl_name,
            bl_name,
            required=True,
        )
        app_path = ensure_bin_artifact(OUT_DIR / board, board)

        if bl_path is None or not bl_path.exists():
            # Bootloader was selected but not available.
            continue
        if app_path is None or not app_path.exists():
            log_warning(f"App binary not found for {board}: {app_path}")
            continue

        with open(bl_path, "rb") as f:
            bl_bin = f.read()
        with open(app_path, "rb") as f:
            app_bin = f.read()

        try:
            combined = create_combined_binary(bl_bin, app_bin)
        except ValueError as e:
            log_error(f"Cannot create combined binary for {board}: {e}")
            sys.exit(1)

        out_path = OUT_DIR / board / f"{board}_combined.bin"
        with open(out_path, "wb") as f:
            f.write(combined)

        generated_for_boards.add(board)

        log_success(
            f"Combined binary: {out_path.name}  "
            f"(bootloader {len(bl_bin)}B + app {len(app_bin)}B = {len(combined)}B)"
        )

    return generated_for_boards


def create_tarball(manifest, selected_boards, include_combined=False):
    git_hash = get_git_hash_or_tag()
    tarball_name = OUT_DIR / f"firmware_{git_hash}.tar.gz"

    with tarfile.open(tarball_name, "w:gz") as tar:
        manifest_json = json.dumps(manifest, indent=2)
        manifest_bytes = manifest_json.encode("utf-8")
        info = tarfile.TarInfo(name="manifest.json")
        info.size = len(manifest_bytes)
        tar.addfile(info, fileobj=io.BytesIO(manifest_bytes))
        log_success("Added manifest.json to tarball")

        for board in selected_boards:
            bin_path = ensure_bin_artifact(OUT_DIR / board, board)
            crc_path = OUT_DIR / board / f"{board}.crc"
            combined_path = OUT_DIR / board / f"{board}_combined.bin"

            if bin_path is not None and bin_path.exists():
                tar.add(bin_path, arcname=f"{board}.bin")
                log_success(f"Added {bin_path.name} to tarball")
            if crc_path.exists():
                tar.add(crc_path, arcname=f"{board}.crc")
                log_success(f"Added {crc_path.name} to tarball")
            if include_combined and combined_path.exists():
                tar.add(combined_path, arcname=f"{board}_combined.bin")
                log_success(f"Added {combined_path.name} to tarball")

    log_success(f"Tarball created: {tarball_name}")
    return tarball_name


# Post-build actions
log_success("Post-build: computing CRCs for selected board targets …")
manifest = compute_crcs_and_manifest(selected_boards)

generated_combined_boards = set()
if options.bootloader and selected_bootloader_targets:
    log_success(
        "Generating combined binaries for selected bootloader targets: "
        + ", ".join(sorted(selected_bootloader_targets))
    )
    generated_combined_boards = create_combined_binaries(
        selected_boards, set(selected_bootloader_targets)
    )
elif options.bootloader:
    log_warning(
        "Bootloader support requested, but no matching bootloader targets were resolved."
    )
else:
    log_success(
        "Bootloader support not requested; skipping combined binary generation."
    )

for board in generated_combined_boards:
    if board in manifest:
        manifest[board]["combined_bin"] = f"{board}_combined.bin"

if options.package:
    log_success("Packaging firmware into tarball for daqapp2 flashing …")
    create_tarball(
        manifest,
        selected_boards,
        include_combined=options.bootloader,
    )
