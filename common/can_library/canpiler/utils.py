"""
utils.py

Author: Irving Wang (irvingw@purdue.edu)
"""

import json
import subprocess
from pathlib import Path
from jinja2 import Environment, FileSystemLoader, select_autoescape

BASE_DIR = Path(__file__).parent.parent
SCHEMA_DIR = BASE_DIR / 'schema'
CONFIG_DIR = BASE_DIR / 'configs'
NODE_CONFIG_DIR = CONFIG_DIR / 'nodes'
EXTERNAL_NODE_CONFIG_DIR = CONFIG_DIR / 'external_nodes'
GENERATED_DIR = BASE_DIR / 'generated'
DBC_DIR = BASE_DIR / 'dbc'
BUS_CONFIG_PATH = CONFIG_DIR / 'system' / 'bus_configs.json'
COMMON_TYPES_CONFIG_PATH = CONFIG_DIR / 'system' / 'common_types.json'
FAULT_CONFIG_PATH = CONFIG_DIR / 'system' / 'faults.json'

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

def to_macro_name(name: str) -> str:
    return name.upper()

def get_git_hash():
    """
    Returns the short git hash of the current commit
    """
    try:
        result = subprocess.run(['git', 'rev-parse', '--short=7', 'HEAD'], 
                                capture_output=True, text=True, check=True)
        return result.stdout.strip()
    except (subprocess.CalledProcessError, FileNotFoundError):
        return "unknown"

def get_jinja_env():
    template_dir = Path(__file__).parent / 'templates'
    env = Environment(
        loader=FileSystemLoader(str(template_dir)),
        autoescape=select_autoescape(),
        trim_blocks=True,
        lstrip_blocks=True,
        keep_trailing_newline=True
    )
    
    # Custom Filters
    env.filters['to_c_hex'] = lambda v: f"0x{v:X}" if v is not None else "0"
    
    def format_float(val: float) -> str:
        """Format float to .6g and ensure it looks like a float literal in C"""
        s = f"{val:.6g}"
        if '.' not in s and 'e' not in s:
            s = f"{val:.1f}"
        return s + "f"
    
    env.filters['format_float'] = format_float
    
    return env

def render_template(env, template_name, output_path, **context):
    template = env.get_template(template_name)
    content = template.render(**context)
    with open(output_path, 'w') as f:
        f.write(content)
