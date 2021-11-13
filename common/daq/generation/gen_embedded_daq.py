""" gen_embedded_daq.py: Adds can msg defs for daq protocol and generates the required embedded code """

import generator

#
# GENERATION STRINGS
#
gen_auto_var_ct_start   = "BEGIN AUTO VAR COUNT"
gen_auto_var_ct_stop    = "END AUTO VAR COUNT"
gen_auto_var_ids_start  = "BEGIN AUTO VAR IDs"
gen_auto_var_ids_stop   = "END AUTO VAR IDs"
gen_auto_var_defs_start = "BEGIN AUTO VAR DEFS"
gen_auto_var_defs_stop  = "END AUTO VAR DEFS"

def generate_daq_can_msgs(daq_config, can_config):
    """ generates message definitions for daq commands and responses """

    for daq_bus in daq_config['busses']:

        # find corresponding can config for the bus
        can_bus_config = {}
        config_found = False 
        for can_bus in can_config['busses']:
            if can_bus['bus_name'] == daq_bus['bus_name']:
                print(f"can config for bus {can_bus['bus_name']} found")
                can_bus_config = can_bus
                config_found = True
                break
        if not config_found:
            generator.log_error(f"CAN config for bus {daq_bus['bus_name']} not found")
            quit(1)
        
        # create daq node
        daq_node = {"node_name":"DAQ",
                    "node_ssa":daq_bus['daq_ssa'],
                    "tx":[], "rx":[]}

        for daq_node_config in daq_bus['nodes']:

            # get node's ssa
            ssa = -1
            for can_node in can_bus['nodes']:
                if can_node['node_name'] == daq_node_config['node_name']:
                    print(f"match for can node {daq_node['node_name']} found")
                    ssa = can_node['node_ssa']
                    # configure daq rx message
                    can_node['rx'].append({"msg_name":f"daq_command_{daq_node_config['node_name']}",
                                           "callback":True, "irq":False, "arg_type":"header"})
                    
                    # configure node tx message
                    response_msg = {"msg_name":f"daq_response_{daq_node_config['node_name']}",
                                    "msg_desc":f"daq response from node {daq_node_config['node_name']}",
                                    "signals":[{"sig_name":"daq_response","type":"uint64_t","length":64}],
                                    "msg_period":0, "msg_hlp":5, "msg_pgn":daq_bus['daq_rx_pgn']}
                    can_node['tx'].append(response_msg)
                    break
            if ssa == -1:
                generator.log_error(f"CAN node name not found: {daq_node_config['node_name']}")
                quit(1)

            # configure daq node tx message defs
            command_msg = {"msg_name":f"daq_command_{daq_node_config['node_name']}",
                           "msg_desc":f"daq command for node {daq_node_config['node_name']}",
                           "signals":[{"sig_name":"daq_command","type":"uint64_t","length":64}],
                           "msg_period":0, "msg_hlp":5, "msg_pgn":ssa}
            daq_node['tx'].append(command_msg)

            # configure daq node rx messages
            daq_node['rx'].append(f"daq_response_{daq_node['node_name']}")


        can_bus_config['nodes'].append(daq_node)

def configure_node(node_config, node_paths):
    """ 
    Generates code for c and h files within a node
    @param  node_config     json config for the specific node
    @param  node_paths      paths to [h file, c file] for that node
    """

    print(f"Configuring DAQ for Node " + node_config['node_name'])

    #
    # Configure header file -----------
    #
    with open(node_paths[0], "r") as h_file:
        h_lines = h_file.readlines()

    # define NUM_VARS
    num_vars = len(node_config['variables'])
    generator.insert_lines(h_lines, gen_auto_var_ct_start, gen_auto_var_ct_stop, [f"#define NUM_VARS {num_vars}\n"])

    # define variable IDs
    var_id_lines = [f"#define DAQ_ID_{var['var_name'].upper()} {idx}\n" for idx, var in enumerate(node_config['variables'])]
    generator.insert_lines(h_lines, gen_auto_var_ids_start, gen_auto_var_ids_stop, var_id_lines)

    # Write changes to header file
    with open(node_paths[0], "w") as h_file:
        h_file.writelines(h_lines)

    #
    # Configure source file -----------
    #
    with open(node_paths[1], "r") as c_file:
        c_lines = c_file.readlines()

    # define variable definitions
    var_defs = ["daq_variable_t tracked_vars[NUM_VARS] = {\n"]
    for var in node_config['variables']:
        line = f"    {{.is_read_only={int(var['read_only'])}, .bit_length={var['bit_length']}, "
        if "eeprom" in var:
            line += f".eeprom_enabled=1, .eeprom_label=\"{var['eeprom']['label']}\", .eeprom_version={var['eeprom']['version']}"
        else:
            line += f".eeprom_enabled=0"
        line += "},\n"
        var_defs.append(line)
    var_defs.append("};\n")
    generator.insert_lines(c_lines, gen_auto_var_defs_start, gen_auto_var_defs_stop, var_defs)

    # Write changes to source file
    with open(node_paths[1], "w") as c_file:
        c_file.writelines(c_lines)

def configure_bus(bus, source_dir, c_dir, h_dir):
    """ Generates daq code for nodes on a bus """

    node_names = [node['node_name'] for node in bus['nodes']]
    node_paths = generator.find_node_paths(node_names, source_dir, c_dir, h_dir)

    matched_nodes = [node for node in bus['nodes'] if node['node_name'] in node_paths.keys()]

    for node in matched_nodes:
        configure_node(node, node_paths[node['node_name']])


def gen_embedded_daq(daq_conf, source_dir, c_dir, h_dir):
    """ Generate daq code """

    for bus in daq_conf['busses']:
        configure_bus(bus, source_dir, c_dir, h_dir)
    
    generator.log_success("Embedded DAQ Code Generated")