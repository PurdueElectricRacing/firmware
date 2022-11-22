""" generator.py: Fault code Generation (faults.c and faults.h). """
import sys
import math
import json
import os
from os import path
from jsonschema import validate
from jsonschema.exceptions import ValidationError
from pathlib import Path

# Generator configuration relative to project directory
GENERATOR_CONFIG_JSON_PATH = os.path.join(os.path.dirname(__file__), 'gen_config.json')

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

priority_dict = {'info':0, 'warning':1, 'critical':2}

def load_json_config(config_path, schema_path):
    """ loads config from json and validates with schema """
    config = json.load(open(config_path))
    schema = json.load(open(schema_path))

    # compare with schema
    try:
        validate(config, schema)
    except ValidationError as e:
        log_error("Invalid JSON!")
        print(e)
        quit(1)

    return config
# arg = -1
# try:
#     arg = sys.argv[1]
# except:
#     print("This has failed")
# print(f"Attempting to generate code for {arg}")
def generate_all():
    gen_config = json.load(open(GENERATOR_CONFIG_JSON_PATH))
    relative_dir = Path(os.path.dirname(__file__))

    fault_config_path = Path(os.path.abspath(relative_dir / gen_config['fault_json_config_path']))
    fault_schema_path = Path(os.path.abspath(relative_dir / gen_config['fault_json_schema_path']))

    firmware_common_dir = Path(os.path.abspath(relative_dir / gen_config['common_directory']))

    print(firmware_common_dir)

    print(fault_config_path)
    print(fault_schema_path)

    fault_config = load_json_config(fault_config_path, fault_schema_path)

if __name__ == "__main__":
    generate_all()