""" gen_embedded_daq.py: Adds can msg defs for daq protocol and generates the required embedded code """

import generator
import gen_embedded_can

#
# GENERATION STRINGS
#
gen_auto_var_ct_start       = "BEGIN AUTO VAR COUNT"
gen_auto_var_ct_stop        = "END AUTO VAR COUNT"
gen_auto_var_ids_start      = "BEGIN AUTO VAR IDs"
gen_auto_var_ids_stop       = "END AUTO VAR IDs"

gen_auto_file_structs_start = "BEGIN AUTO FILE STRUCTS"
gen_auto_file_structs_stop  = "END AUTO FILE STRUCTS"

gen_auto_file_defaults_start = "BEGIN AUTO FILE DEFAULTS"
gen_auto_file_defaults_stop  = "END AUTO FILE DEFAULTS"

gen_auto_init_start = "BEGIN AUTO INIT"
gen_auto_init_stop  = "END AUTO INIT"

gen_auto_var_includes_start = "BEGIN AUTO VAR INCLUDES"
gen_auto_var_includes_stop  = "END AUTO VAR INCLUDES"
gen_auto_var_defs_start     = "BEGIN AUTO VAR DEFS"
gen_auto_var_defs_stop      = "END AUTO VAR DEFS"
gen_auto_callback_def_start = "BEGIN AUTO CALLBACK DEF"
gen_auto_callback_def_stop  = "END AUTO CALLBACK DEF"

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
                    can_node['rx'].append({"msg_name":f"daq_command_{daq_node_config['node_name'].upper()}",
                                           "callback":True, "irq":False, "arg_type":"header"})
                    
                    # configure node tx message
                    rsp_msg = {"msg_name":f"daq_response_{daq_node_config['node_name'].upper()}",
                                    "msg_desc":f"daq response from node {daq_node_config['node_name'].upper()}",
                                    "signals":[{"sig_name":"daq_response","type":"uint64_t","length":64}],
                                    "msg_period":0, "msg_hlp":5, "msg_pgn":daq_bus['daq_rx_pgn']}
                    can_node['tx'].append(rsp_msg)

                    # add to daq config
                    periph = gen_embedded_can.DEFAULT_PERIPHERAL
                    if 'can_periperal' in can_node: periph = can_node['can_peripheral']
                    daq_node_config['daq_rsp_msg_periph'] = periph

                    break
            if ssa == -1:
                generator.log_error(f"CAN node name not found: {daq_node_config['node_name']}")
                quit(1)

            # configure daq node tx message defs
            command_msg = {"msg_name":f"daq_command_{daq_node_config['node_name'].upper()}",
                           "msg_desc":f"daq command for node {daq_node_config['node_name'].upper()}",
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
    if 'files' in node_config:
        for file in node_config['files']:
            num_vars += len(file['contents'])
    generator.insert_lines(h_lines, gen_auto_var_ct_start, gen_auto_var_ct_stop, [f"#define NUM_VARS {num_vars}\n"])

    # define variable IDs
    var_id_lines = [f"#define DAQ_ID_{var['var_name'].upper()} {idx}\n" for idx, var in enumerate(node_config['variables'])]
    if 'files' in node_config:
        for file in node_config['files']:
            offset = len(var_id_lines)
            var_id_lines += [f"#define DAQ_ID_{var['var_name'].upper()} {idx + offset}\n" for idx, var in enumerate(file['contents'])]
    generator.insert_lines(h_lines, gen_auto_var_ids_start, gen_auto_var_ids_stop, var_id_lines)

    # generate file structures
    file_struct_lines = []
    if 'files' in node_config:
        for file in node_config['files']:
            file_struct_lines.append("typedef struct { \n")
            for file_var in file['contents']:
                file_struct_lines.append(f"    {file_var['type']}   {file_var['var_name']};\n")
            file_struct_lines.append(f"}} __attribute__((packed)) {file['name']}_t;\n\n")
            file_struct_lines.append(f"extern {file['name']}_t {file['name']};\n\n")
    generator.insert_lines(h_lines, gen_auto_file_structs_start, gen_auto_file_structs_stop, file_struct_lines)

    # Write changes to header file
    with open(node_paths[0], "w") as h_file:
        h_file.writelines(h_lines)

    #
    # Configure source file -----------
    #
    with open(node_paths[1], "r") as c_file:
        c_lines = c_file.readlines()

    # define variable includes
    var_includes = node_config['includes'] + '\n'
    generator.insert_lines(c_lines, gen_auto_var_includes_start, gen_auto_var_includes_stop, var_includes)

    # define file defaults
    file_struct_lines = []
    if 'files' in node_config:
        for file in node_config['files']:
            file_struct_lines.append(f"{file['name']}_t {file['name']} = {{\n")
            for file_var in file['contents']:
                file_struct_lines.append(f"    .{file_var['var_name']} = {file_var['default']},\n")
            file_struct_lines.append(f"}};\n")
    generator.insert_lines(c_lines, gen_auto_file_defaults_start, gen_auto_file_defaults_stop, file_struct_lines)

    # define file mappings
    init_lines = []
    init_lines.append(f"    uint8_t ret = daqInitBase(tx_a, NUM_VARS, {node_config['daq_rsp_msg_periph']}, ID_DAQ_RESPONSE_{node_config['node_name'].upper()}, tracked_vars);\n")
    if 'files' in node_config:
        for file in node_config['files']:
            init_lines.append(f"    mapMem((uint8_t *) &{file['name']}, sizeof({file['name']}), \"{file['eeprom_lbl']}\", 1);\n")
    init_lines.append(f"    return ret;\n")
    generator.insert_lines(c_lines, gen_auto_init_start, gen_auto_init_stop, init_lines)


    # define variable definitions
    var_defs = ["daq_variable_t tracked_vars[NUM_VARS] = {\n"]
    for var in node_config['variables']:

        # calculate bit length
        bit_length = generator.data_type_length[var['type']]
        if ('length' in var):
            if ('uint' in var['type']):
                if (var['length'] > bit_length):
                    generator.log_error(f"Variable {var['var_name']} length too large for defined data type")
                    quit(1)
                else:
                    bit_length = var['length']
            else:
                generator.log_error(f"Don't define length for types other than unsigned integers, variable: {var['var_name']}")
                quit(1)
        var['length'] = bit_length

        line = f"    {{.is_read_only={int(var['read_only'])}, .bit_length={var['length']}, "

        if ('has_read_func' in var and var['has_read_func']):
            if ('access_phrase_write' not in var and not var['read_only']):
                generator.log_error(f"If access phrase is a read function, and not read only, must define access_phrase_write: {var['var_name']}")
                quit(1)
            line += f".has_read_func=1, .read_func_a=(read_func_ptr_t){var['access_phrase']}, "
        else:
            line += f".read_var_a=&{var['access_phrase']}, "
        if (not var['read_only']):
            if ('access_phrase_write' in var):
                if ('has_write_func' in var and var['has_write_func']):
                    line += f".has_write_func=1, .write_func_a={var['access_phrase_write']}, "
                else:
                    line += f".write_var_a=&{var['access_phrase_write']}, "
            else:
                line += f".write_var_a=&{var['access_phrase']}, "
        else:
            line += f".write_var_a=NULL, "
            
        line += "},\n"
        var_defs.append(line)

    # file variables are similar, but slighlty different
    if 'files' in node_config:
        for file in node_config['files']:
            for var in file['contents']:
                # calculate bit length
                bit_length = generator.data_type_length[var['type']]
                if ('length' in var):
                    generator.log_error(f"Variable {var['var_name']} custom length not supported for file variable")
                    quit(1)
                var['length'] = bit_length

                line = f"    {{.is_read_only=0, .bit_length={var['length']}, "
                line += f".read_var_a=&({file['name']}.{var['var_name']}), "
                line += f".write_var_a=&({file['name']}.{var['var_name']}), "
                line += "},\n"
                var_defs.append(line)

    var_defs.append("};\n")
    generator.insert_lines(c_lines, gen_auto_var_defs_start, gen_auto_var_defs_stop, var_defs)

    # callback def
    callback_def = [f"void daq_command_{node_config['node_name'].upper()}_CALLBACK(CanMsgTypeDef_t* msg_header_a)\n"]
    generator.insert_lines(c_lines, gen_auto_callback_def_start, gen_auto_callback_def_stop, callback_def)

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