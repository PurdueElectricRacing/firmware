import json
from pathlib import Path

BASE_DIR = Path(__file__).parent
SCHEMA_DIR = BASE_DIR / 'schema'
CONFIG_DIR = BASE_DIR / 'configs'
NODE_CONFIG_DIR = CONFIG_DIR / 'nodes'
EXTERNAL_NODE_CONFIG_DIR = CONFIG_DIR / 'external_nodes'
BUS_CONFIG_PATH = CONFIG_DIR / 'system' / 'bus_configs.json'
COMMON_TYPES_CONFIG_PATH = CONFIG_DIR / 'system' / 'common_types.json'

MAX_DATA_SIZE = 64
CTYPE_SIZES = {
    "uint8_t": 8, "int8_t": 8,
    "uint16_t": 16, "int16_t": 16,
    "uint32_t": 32, "int32_t": 32,
    "uint64_t": 64, "int64_t": 64,
    "float": 32, "double": 64,
    "bool": 1
}

class bcolors:
    HEADER = '\033[95m'
    BLUE = '\033[94m'
    CYAN = '\033[96m'
    GREEN = '\033[92m'
    ORANGE = '\033[93m'
    RED = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

def print_as_error(message):
    print(f"{bcolors.RED}[ERROR] {message}{bcolors.ENDC}")

def print_as_warning(message):
    print(f"{bcolors.ORANGE}[WARNING] {message}{bcolors.ENDC}")

def print_as_ok(message):
    print(f"[OK] {message}")

def print_as_success(message):
    print(f"{bcolors.GREEN}[SUCCESS] {message}{bcolors.ENDC}")

def load_json(filepath):
    """Load JSON file"""
    with open(filepath) as f:
        return json.load(f)
