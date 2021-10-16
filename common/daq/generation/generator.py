""" generator.py: Main Code Generation for Embedded, DBC, etc. """

import math
import json
from jsonschema import validate
from jsonschema.exceptions import ValidationError
import gen_embedded_can
import gen_dbc

# Generator configuration relative to project directory
GENERATOR_CONFIG_JSON_PATH = './common/daq/generation/gen_config.json'

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

def generate_ids(can_config):
    """ Combine hlp, pgn, and ssa for each message and add 'id' key"""
    for bus in can_config['busses']:
        for node in bus['nodes']:
            ssa = node['node_ssa']
            for msg in node['tx']:
                hlp = msg['msg_hlp']
                pgn = msg['msg_pgn']

                # hlp (3) + pgn (20) + ssa (6) bits
                id = ((((hlp & 0b111) << 20) | (pgn & 0xFFFFF)) << 6) | (ssa & 0b111111)
                # print(msg['msg_name'] + " id: "+ hex(id))
                msg['id'] = id
    return can_config

def generate_dlcs(can_config):
    """ Add up signal lengths and add 'dlc' key to each message """
    for bus in can_config['busses']:
        for node in bus['nodes']:
            for msg in node['tx']:
                msg_length = 0
                for sig in msg['signals']:
                    msg_length += sig['length']
                msg['dlc'] =  math.ceil(msg_length / 8.0)
                # print(msg['msg_name'] + " dlc: "+ str(msg['dlc']))
                if msg['dlc'] > 8:
                    log_error("DLC too long for " + msg['msg_name'])
                    quit()
    return can_config

def check_repeat_defs(can_config):
    """ Checks for repeated message definitions or ids"""
    message_names = []
    for bus in can_config['busses']:
        message_ids = []
        node_ssas = []
        node_names = []
        for node in bus['nodes']:
            if node['node_name'] in node_names:
                log_error(f"Found identical node names: {node['node_name']}")
                quit()
            else:
                node_names.append(node['node_name'])
            if node['node_ssa'] in node_ssas:
                log_error(f"Found identical node ssas for {node['node_name']} of ssa: {node['node_ssa']}")
                quit()
            else:
                node_ssas.append(node['node_ssa'])
            for msg in node['tx']:
                if msg['msg_name'] in message_names:
                    log_error(f"Found multiple definitions for {msg['msg_name']}")
                    quit()
                else:
                    message_names.append(msg['msg_name'])
                if msg['id'] in message_ids:
                    log_error(f"Found identical message ids for {msg['msg_name']} with id {hex(msg['id'])}")
                    quit()
                else:
                    message_ids.append(msg['id'])

def output_bus_load(can_config):
    """ calculates bus load based on message periods and sizes """
    overhead_per_msg = 64 + 18 # frame + possible stuffing
    baudrate = 500000
    bit_time = 1.0 / baudrate
    for bus in can_config['busses']:
        total_load = 0
        for node in bus['nodes']:
            for msg in node['tx']:
                if msg['msg_period'] != 0:
                    load = (msg['dlc'] * 8 + overhead_per_msg) * bit_time / (msg['msg_period']/1000)
                    total_load += load
                    print(f"{msg['msg_name']}: {round(load*100,3)}%")
        print(f"Total load for bus {bus['bus_name']}: {round(total_load*100,3)}% (calculated with only periodic messages)")


def load_message_config(config_path, schema_path):
    """ loads message definitions from json and validates with schema """
    config = json.load(open(config_path))
    can_schema = json.load(open(schema_path))

    # compare with schema
    try:
        validate(config, can_schema)
    except ValidationError as e:
        log_error("Invalid JSON!")
        print(e)
        quit()

    generate_ids(config)
    generate_dlcs(config)
    check_repeat_defs(config)

    return config

def generate_all():

    gen_config = json.load(open(GENERATOR_CONFIG_JSON_PATH))
    config = load_message_config(gen_config['can_json_config_path'], gen_config['can_json_schema_path'])

    gen_embedded_can.gen_embedded_can(config, gen_config['source_directory'], gen_config['node_parse_c_dir'], gen_config['node_parse_h_dir'])
    gen_dbc.gen_dbc(config, gen_config['dbc_output_path'])
    output_bus_load(config)


if __name__ == "__main__":
    generate_all()