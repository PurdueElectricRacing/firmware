""" generator.py: Fault code Generation (faults.c and faults.h). """
import sys
import math
import json
import os
import gen_faults
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

def log_heading(phrase):
    print(f"{bcolors.BOLD}{bcolors.UNDERLINE}{phrase}{bcolors.ENDC}")

priority_dict = {'warning':0, 'error':1, 'fatal':2}


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


def insert_lines(source: list, start, stop, new_lines):
    """
    Insert lines between start and stop lines, writes over pre-existing data
    @param source    source lines to edit
    @param start     phrase contained in line to begin generation after
    @param stop      phrase contained in line after generation section
    @param new_lines list of lines to place between start and stop

    @return          source lines with the modification
    """

    curr_idx = 0
    start_idx = 0
    stop_idx = 0
    for line in source:
        if start in line:
            start_idx = curr_idx
        elif stop in line:
            stop_idx = curr_idx
            break
        curr_idx += 1

    if stop_idx <= start_idx or stop_idx == 0 or start_idx ==0:
        log_error("Insert lines failed for start "+start+" and stop "+stop)
        log_error("Check to make sure the start and stop phrases are correct")
        quit(1)

    # remove existing lines
    del source[start_idx+1:stop_idx]

    # add new lines
    for idx, nl in enumerate(new_lines):
        source.insert(start_idx + 1 + idx, nl)

    return source

def check_message_len(fault_config):
    """
    Make sure no fault message exceeds the maximum message length
    @param fault_config    Fault JSON dictionary

    @return          bool based on validity of fault JSON nodes+messages
    """

    #There are too many nodes (Only 4 bits allowed)
    if len(fault_config['modules']) > 16:
        log_error(f"Max number of nodes is 16. There are {len(fault_config['modules'])} nodes currently defined in fault_config.json. \nReduce the number of nodes you have defined")
        quit(1)
    else:
        print(f"Total number of nodes: {len(fault_config['modules'])}")
    length = 0
    #Check message length
    for node in fault_config['modules']:
        length += len(node['faults'])
        for fault in node['faults']:
            if len(fault['lcd_message']) >= 75:
                log_error(f"The message for fault \"{fault['fault_name']}\" in \"{node['node_name']}\"is too long. Max limit is 75 characters")
                quit(1)
    #Make sure total number of messages isn't too large
    if length >= 4090:
        log_error(f"Max number of faults is 4090. There are {length} faults currently defined in fault_config.json. \nReduce the number of faults you have defined")
        quit(1)
    else:
        print(f"Total number of faults: {length}")
    return True

def check_names(fault_config):
    """
    Make sure fault/node names are valid
    @param fault_config    Fault JSON dictionary

    @return          bool based on validity of fault JSON names
    """
    node_names = []
    fault_names = []
    for node in fault_config['modules']:
        #Validity of node names
        if node['node_name'] in node_names:
            log_error(f"Found multiple nodes with the name \"{node['node_name']}\". Duplicate names are not allowed")
            quit(1)
        else:
            node_names.append(node['node_name'])
        #Validity of fault names
        for fault in node['faults']:
            if fault['fault_name'] in fault_names:
                log_error(f"Found multiple faults with the name \"{fault['fault_name']}\". The duplicate was found in \"{node['node_name']}\" \n Duplicate names are not allowed")
                quit(1)
            else:
                fault_names.append(fault['fault_name'])


def create_ids(fault_config):
    """
    Ad an 'id' parameter to the JSON
    @param fault_config    Fault JSON dictionary

    @return          none
    """
    num = 0
    idx = 0
    for node in fault_config['modules']:
        for fault in node['faults']:
            #id : Owner (MCU) = 4 bits, Index in fault array = 12 bits
            id = ((num << 12) | (idx & 0x0fff))
            fault['id'] =  id
            idx += 1
        num += 1

def process_priorities(fault_config):
    """
    Add interpreted 'priority' value to fault dictionary
    @param fault_config    Fault JSON dictionary

    @return          none
    """
    for node in fault_config['modules']:
        for fault in node['faults']:
            if fault['priority'] == "warning":
                fault['pri_interp'] = 0
            elif fault['priority'] == "error":
                fault['pri_interp'] = 1
            else:
                fault['pri_interp'] = 2

def process_nodes(fault_config):
    """
    Replace node names with the corresponding name in can, named 'can_name'
    @param fault_config    Fault JSON dictionary

    @return          none
    """
    id = 0
    for node in fault_config['modules']:
        node['name_interp'] = id
        id += 1
    for node in fault_config['modules']:
        try:
            node['can_name']
        except KeyError:
            node['can_name'] = node['node_name']
        except:
            log_error("An error occured in configuring CAN names for faults")
            quit(1)




def generate_all():


    gen_config = json.load(open(GENERATOR_CONFIG_JSON_PATH))
    relative_dir = Path(os.path.dirname(__file__))

    fault_config_path = Path(os.path.abspath(relative_dir / gen_config['fault_json_config_path']))
    fault_schema_path = Path(os.path.abspath(relative_dir / gen_config['fault_json_schema_path']))
    fault_c_path = Path(os.path.abspath(relative_dir / gen_config['fault_c_file']))
    fault_h_path = Path(os.path.abspath(relative_dir / gen_config['fault_h_file']))
    node_path = Path(os.path.abspath(relative_dir / gen_config['node_file']))

    log_heading("Defined Paths:")

    print(fault_config_path)
    print(fault_schema_path)
    print(fault_c_path)
    print(fault_h_path)
    print(node_path)

    log_heading("Checking validity of fault configuration:")

    fault_config = load_json_config(fault_config_path, fault_schema_path)

    # arg = check_args(fault_config)
    check_message_len(fault_config)
    check_names(fault_config)

    log_success("Fault configuration is valid!")

    log_heading(f"Generating fault code:")

    create_ids(fault_config)
    process_priorities(fault_config)
    process_nodes(fault_config)

    gen_faults.gen_faults(fault_config, fault_c_path, fault_h_path, node_path)
    log_success(f"Fault Code Successfully Generated!")







if __name__ == "__main__":
    generate_all()