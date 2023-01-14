""" generator.py: Main Code Generation for Embedded, DBC, etc. """

import math
import json
import os
from os import path
from jsonschema import validate
from jsonschema.exceptions import ValidationError
import gen_embedded_can
import gen_embedded_daq
import gen_dbc
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

data_type_length = {'uint8_t':8, 'uint16_t':16, 'uint32_t':32, 'uint64_t':64,
                    'int8_t':8, 'int16_t':16, 'int32_t':32, 'int64_t':64,
                    'float':32}
data_lengths = [8, 16, 32, 64]

def gen_bit_length(sig):
    """ Calculates and updates bit length for signal based on data type and length """
    bit_length = data_type_length[sig['type']]
    if('length' in sig):
        if ('uint' in sig['type']):
            if (sig['length'] > bit_length):
                log_error(f"Signal {sig['sig_name']} length too large for defined data type")
                quit(1)
            else:
                bit_length = sig['length']
        else:
            log_error(f"Don't define length for types other than unsigned integers, signal: {sig['sig_name']}")
            quit(1)
    sig['length'] = bit_length
    return bit_length

def generate_ids(can_config):
    """ Combine hlp, pgn, and ssa for each message and add 'id' key"""
    for bus in can_config['busses']:
        for node in bus['nodes']:
            ssa = node['node_ssa']
            for msg in node['tx']:
                id = 0
                if 'msg_id_override' in msg:
                    id = int(msg['msg_id_override'], 0)
                elif 'msg_hlp' in msg and 'msg_pgn' in msg:
                    hlp = msg['msg_hlp']
                    pgn = msg['msg_pgn']
                    # hlp (3) + pgn (20) + ssa (6) bits
                    id = ((((hlp & 0b111) << 20) | (pgn & 0xFFFFF)) << 6) | (ssa & 0b111111)
                else:
                    log_error(f"Message {msg['msg_name']} needs either msg_hlp and msg_pgn defined or msg_id_override")
                    quit(1)
                if id < 0 or id > 0x1FFFFFFF:
                    log_error(f"Message {msg['msg_name']}'s can id is too large: {hex(id)}, max is 0x1FFFFFFF")
                    quit(1)
                # print(msg['msg_name'] + " id: "+ hex(id))
                msg['id'] = id
    return can_config

def generate_dlcs(can_config):
    """ Add up / generate signal lengths and add 'dlc' key to each message """
    for bus in can_config['busses']:
        for node in bus['nodes']:
            for msg in node['tx']:
                msg_length = 0
                for sig in msg['signals']:
                    msg_length += gen_bit_length(sig)
                msg['dlc'] =  math.ceil(msg_length / 8.0)
                # print(msg['msg_name'] + " dlc: "+ str(msg['dlc']))
                if msg['dlc'] > 8:
                    log_error("DLC too long for " + msg['msg_name'])
                    quit(1)
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
                log_error(f"Found identical node names within a bus: {node['node_name']}")
                quit(1)
            else:
                node_names.append(node['node_name'])
            if node['node_ssa'] in node_ssas:
                log_error(f"Found identical node ssas for {node['node_name']} of ssa: {node['node_ssa']}")
                quit(1)
            else:
                node_ssas.append(node['node_ssa'])
            for msg in node['tx']:
                if msg['msg_name'] in message_names:
                    log_error(f"Found multiple definitions for {msg['msg_name']}")
                    quit(1)
                else:
                    message_names.append(msg['msg_name'])
                if msg['id'] in message_ids:
                    log_error(f"Found identical message ids for {msg['msg_name']} with id {hex(msg['id'])}")
                    quit(1)
                else:
                    message_ids.append(msg['id'])

def generateFLmessages(can_config, fault_config):
    """Generates messages in can config dictionaries for the fault library"""
    namearr = []
    for node in fault_config['modules']:
        try:
            namearr.append((str)(node['can_name']).lower())
        except KeyError:
            node['can_name'] = node['node_name']
            namearr.append((str)(node['can_name']).lower())
        except:
            log_error("An error occured in configuring CAN names for faults")
            quit(1)
    i = 0
    for bus in can_config['busses']:
        for node in bus['nodes']:
            if (str)(node['node_name']).lower() in namearr:
                namearr.remove((str)(node['node_name']).lower())
                node['tx'].append({'msg_name': 'fault_sync_' + (str)(node['node_name']).lower(), 'msg_desc': 'Fault status message', 'signals': [{'sig_name': 'idx', 'type': 'uint16_t', 'length': 16}, {'sig_name': 'latched', 'type': 'uint8_t', 'length': 1}], 'msg_period': 0, 'msg_hlp': 0, 'msg_pgn': (9000 + i)})
                for f_node in fault_config['modules']:
                    if (str)(f_node['can_name']).lower() != (str)(node['node_name']).lower():
                        node['rx'].append({'msg_name': 'fault_sync_' + (str)(f_node['can_name']).lower(), 'callback': True})
                node['rx'].append({"msg_name": "set_fault", "callback": True})
                node['rx'].append({"msg_name": "return_fault_control", "callback": True})
                i += 1


def check_repeat_daq_variables(daq_config):
    """ Checks for repeated variable names or eeprom labels on a per node basis """
    for bus in daq_config['busses']:
        for node in bus['nodes']:
            var_names = []
            eeprom_lbls = []
            if len(node['variables']) > 32:
                log_error(f"Node {node['node_name']} has too many daq variables (max=32)")
                quit(1)
            for var in node['variables']:
                if(var['var_name'] in var_names):
                    log_error(f"Repeated variable name: {var['var_name']} in node {node['node_name']}")
                    quit(1)
                else:
                    var_names.append(var['var_name'])
                if("eeprom" in var):
                    if(var['eeprom']['label'] in eeprom_lbls):
                        log_error(f"Repeated eeprom label: {var['eeprom']['label']} in node {node['node_name']}")
                        quit(1)
                    else:
                        eeprom_lbls.append(var['eeprom']['label'])

def label_junction_nodes(can_config):
    """ Finds junction nodes, ensures can peripherals defined, and adds is_junction """
    node_names = []
    for bus in can_config['busses']:
        for node in bus['nodes']:
            if node['node_name'] in node_names:
                if ('DAQ' == node['node_name']): continue
                # junction found (assumes check repeat defs already ran and passed)
                print(f"Junction node found: {node['node_name']}")
                node['is_junction'] = True
                # check peripheral
                if 'can_peripheral' not in node:
                    log_error(f"ERROR: can peripheral not defined for junction node {node['node_name']}")
                    quit(1)
                # label the matching nodes on the other busses as a junction
                for bus2 in can_config['busses']:
                    for node2 in bus2['nodes']:
                        if node2['node_name'] == node['node_name']:
                            node2['is_junction'] = True
                            # check peripheral
                            if 'can_peripheral' not in node2:
                                log_error(f"ERROR: can peripheral not defined for junction node {node2['node_name']}")
                                quit(1)
            node_names.append(node['node_name'])

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

def find_node_paths(node_names, source_dir, c_dir, h_dir):
    """
    searches through the head_dir for the c and h files
    with a "NODE_NAME" definition matching one in node_names

    @param node_names   list of node names to search for
    @param source_dir   directory to search for nodes in
    @param c_dir        directory within node of source file
    @param h_dir        directory within node of header file

    @return a dictionary of [h_path, c_path] for each node name
    """

    node_paths = {}
    for folder in os.listdir(source_dir):
        #print("Searching for nodes in "+str(source_dir/ folder/c_dir) + " directory")

        c_path = source_dir/ folder /c_dir
        h_path = source_dir/folder/h_dir

        if path.exists(h_path):
            with open(h_path) as h_file:
                for line in h_file.readlines():
                    if "NODE_NAME" in line:
                        a = line.index("\"")
                        b = line.index("\"", a+1)
                        name = line[a+1:b]
                        if name in node_names:
                            # print("Match found for " + name)
                            if path.exists(c_path):
                                node_paths[name] = [h_path, c_path]
                            else:
                                log_warning("C file not found for " + name +" at "+c_path)
                        break

        else:
            log_warning("Header not found for "+ str(folder) + " at " + str(h_path))
    print(f"Node matches found: {list(node_paths.keys())}")
    return node_paths

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

def check_json_config(config, schema_path):
    """ loads config from json and validates with existing can config dictionary"""
    schema = json.load(open(schema_path))

    # compare with schema
    try:
        validate(config, schema)
    except ValidationError as e:
        log_error("Invalid JSON after adding fault messages!")
        print(e)
        quit(1)

    return config

def generate_all():

    gen_config = json.load(open(GENERATOR_CONFIG_JSON_PATH))
    relative_dir = Path(os.path.dirname(__file__))

    can_config_path = Path(os.path.abspath(relative_dir / gen_config['can_json_config_path']))
    can_schema_path = Path(os.path.abspath(relative_dir / gen_config['can_json_schema_path']))
    daq_config_path = Path(os.path.abspath(relative_dir / gen_config['daq_json_config_path']))
    daq_schema_path = Path(os.path.abspath(relative_dir / gen_config['daq_json_schema_path']))
    fault_config_path = Path(os.path.abspath(relative_dir / gen_config['fault_json_config_path']))
    fault_schema_path = Path(os.path.abspath(relative_dir / gen_config['fault_json_schema_path']))

    firmware_source_dir = Path(os.path.abspath(relative_dir / gen_config['source_directory']))

    print(firmware_source_dir)

    can_config = load_json_config(can_config_path, can_schema_path)
    daq_config = load_json_config(daq_config_path, daq_schema_path)
    fault_config = load_json_config(fault_config_path, fault_schema_path)

    generateFLmessages(can_config, fault_config)

    check_json_config(can_config, can_schema_path)

    check_repeat_daq_variables(daq_config)
    gen_embedded_daq.generate_daq_can_msgs(daq_config, can_config)

    # perform error checking for CAN config
    generate_ids(can_config)
    generate_dlcs(can_config)
    check_repeat_defs(can_config)
    label_junction_nodes(can_config)

    gen_embedded_can.gen_embedded_can(can_config, firmware_source_dir, gen_config['node_parse_c_dir'], gen_config['node_parse_h_dir'])
    gen_embedded_daq.gen_embedded_daq(daq_config, firmware_source_dir, gen_config['node_daq_c_dir'], gen_config['node_daq_h_dir'])
    gen_dbc.gen_dbc(can_config, relative_dir / gen_config['dbc_output_path'])

    output_bus_load(can_config)


if __name__ == "__main__":
    generate_all()