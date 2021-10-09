""" gen_embedded_can.py: Generates embedded code for CAN message parsing using structures with bit fields """

import os
from os import path
import  generator
import json

#
# GENERATION STRINGS
#
gen_id_start = "BEGIN AUTO ID DEFS"
gen_id_stop = "END AUTO ID DEFS"
gen_dlc_start = "BEGIN AUTO DLC DEFS"
gen_dlc_stop = "END AUTO DLC DEFS"
gen_up_start = "BEGIN AUTO UP DEFS"
gen_up_stop = "END AUTO UP DEFS"
gen_raw_struct_start = "BEGIN AUTO MESSAGE STRUCTURE"
gen_raw_struct_stop = "END AUTO MESSAGE STRUCTURE"
gen_can_struct_start = "BEGIN AUTO CAN DATA STRUCTURE"
gen_can_struct_stop = "END AUTO CAN DATA STRUCTURE"
gen_switch_case_start = "BEGIN AUTO CASES"
gen_switch_case_stop = "END AUTO CASES"
gen_stale_case_start = "BEGIN AUTO STALE CHECKS"
gen_stale_case_stop = "END AUTO STALE CHECKS"
gen_filter_start = "BEGIN AUTO FILTER"
gen_filter_stop = "END AUTO FILTER"
gen_callback_start = "BEGIN AUTO EXTERN CALLBACK"
gen_callback_stop = "END AUTO EXTERN CALLBACK"
gen_rx_irq_start = "BEGIN AUTO RX IRQ"
gen_rx_irq_stop = "END AUTO RX IRQ"
gen_irq_extern_start = "BEGIN AUTO EXTERN RX IRQ"
gen_irq_extern_stop = "END AUTO EXTERN RX IRQ"


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
        # print("Searching for nodes in "+str(folder) + " directory")

        c_path = source_dir+'/'+folder+c_dir
        h_path = source_dir+'/'+folder+h_dir

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
                                generator.log_warning("C file not found for " + name +" at "+c_path)
                        break

        else:
            generator.log_warning("Header not found for "+ folder + " at " + h_path)
    print(f"Node matches found: {list(node_paths.keys())}") 
    return node_paths

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
        generator.log_error("Insert lines failed for start "+start+" and stop "+stop)
        generator.log_error("Check to make sure the start and stop phrases are correct")
        quit()

    # remove existing lines
    del source[start_idx+1:stop_idx]
    
    # add new lines
    for idx, nl in enumerate(new_lines):
        source.insert(start_idx + 1 + idx, nl)

    return source

def find_rx_messages(rx_names):
    """
    Searches the entire config for the definitions of the specified messages
    @param  rx_names    list of names of messages
    @return list of message definitions in rx_name order
    """
    global can_config
    msg_defs = []
    for rx in rx_names:
        rx_found = False
        # search until a tx message has msg_name
        for bus in can_config["busses"]:
            for node in bus["nodes"]:
                for msg in node["tx"]:
                    if rx == msg["msg_name"]:
                        msg_defs.append(msg)
                        rx_found = True
                        break
                if rx_found:
                    break
            if rx_found:
                break
        if not rx_found:
            generator.log_error("Message def not found for rx " + str(rx))
            quit()
    
    return msg_defs

def configure_node(node_config, node_paths):
    """ 
    Generates code for c and h files within a node
    @param  node_config     json config for the specific node
    @param  node_paths      paths to [h file, c file] for that node
    """

    print("Configuring Node " + node_config['node_name'])

    # Combine message definitions
    raw_msg_defs = []
    raw_msg_defs += node_config['tx']
    receiving_msg_defs = find_rx_messages([rx_config["msg_name"] for rx_config in node_config['rx']])
    for new_msg in receiving_msg_defs:
        if new_msg not in raw_msg_defs:
            raw_msg_defs.append(new_msg)

    #
    # Configure header file ------------------
    #
    with open(node_paths[0], "r") as h_file:
        h_lines = h_file.readlines()

    # Message IDs and DLCs
    id_lines = []
    dlc_lines = []
    for msg in raw_msg_defs:
        id_lines.append(f"#define ID_{msg['msg_name'].upper()} {hex(msg['id'])}\n")
        dlc_lines.append(f"#define DLC_{msg['msg_name'].upper()} {msg['dlc']}\n")
    h_lines = insert_lines(h_lines, gen_id_start, gen_id_stop, id_lines)
    h_lines = insert_lines(h_lines, gen_dlc_start, gen_dlc_stop, dlc_lines)

    # Message update periods
    up_lines = []
    for msg in receiving_msg_defs:
        if msg['msg_period'] > 0:
            up_lines.append(f"#define UP_{msg['msg_name'].upper()} {msg['msg_period']}\n")
    h_lines = insert_lines(h_lines, gen_up_start, gen_up_stop, up_lines)

    # Define CanParsedData_t
    raw_struct_lines = []
    raw_struct_lines.append("typedef union { __attribute__((packed))\n")
    for msg in raw_msg_defs:
        raw_struct_lines.append("    struct {\n")
        for sig in msg['signals']:
            raw_struct_lines.append(f"        uint64_t {sig['sig_name']}: {sig['length']};\n")
        raw_struct_lines.append(f"    }}{msg['msg_name']};\n") 
    raw_struct_lines.append("    uint8_t raw_data[8];\n")
    raw_struct_lines.append("} CanParsedData_t;\n")
    h_lines = insert_lines(h_lines, gen_raw_struct_start, gen_raw_struct_stop, raw_struct_lines)

    # Define can_data_t
    can_struct_lines = []
    can_struct_lines.append("typedef struct {\n")
    for msg in receiving_msg_defs:
        can_struct_lines.append("    struct {\n")
        for sig in msg['signals']:
            can_struct_lines.append(f"        {sig['type']} {sig['sig_name']};\n")
        
        # stale checking variables
        if msg['msg_period'] > 0:
            can_struct_lines.append(f"        uint8_t stale;\n")
            can_struct_lines.append(f"        uint32_t last_rx;\n")

        can_struct_lines.append(f"    }} {msg['msg_name']};\n")
    can_struct_lines.append("} can_data_t;\n")
    h_lines = insert_lines(h_lines, gen_can_struct_start, gen_can_struct_stop, can_struct_lines)

    # Rx callbacks
    rx_callbacks = [rx_config['msg_name'] for rx_config in node_config['rx'] if ("callback" in rx_config and rx_config["callback"])]
    extern_callback_lines = [f"extern void {msg_name}_CALLBACK(CanParsedData_t* msg_data_a);\n" for msg_name in rx_callbacks]
    h_lines = insert_lines(h_lines, gen_callback_start, gen_callback_stop, extern_callback_lines)

    rx_irq_names = [rx_config['msg_name'] for rx_config in node_config['rx'] if ("irq" in rx_config and rx_config["irq"])]
    extern_callback_lines = [f"extern void {msg_name}_IRQ(CanParsedData_t* msg_data_a);\n" for msg_name in rx_irq_names]
    h_lines = insert_lines(h_lines, gen_irq_extern_start, gen_irq_extern_stop, extern_callback_lines)


    # Write changes to header file
    with open(node_paths[0], "w") as h_file:
        h_file.writelines(h_lines)

    #
    # Configure source file ----------
    #
    with open(node_paths[1], "r") as c_file:
        c_lines = c_file.readlines()

    # Rx switch case
    case_lines = []
    for msg in receiving_msg_defs:
        case_lines.append(f"            case ID_{msg['msg_name'].upper()}:\n")
        for sig in msg['signals']:
            case_lines.append(f"                can_data.{msg['msg_name']}.{sig['sig_name']} = msg_data_a->{msg['msg_name']}.{sig['sig_name']};\n")
        if msg['msg_period'] > 0:
            case_lines.append(f"                can_data.{msg['msg_name']}.stale = 0;\n")
            case_lines.append(f"                can_data.{msg['msg_name']}.last_rx = curr_tick;\n")
        if msg['msg_name'] in rx_callbacks:
            case_lines.append(f"                {msg['msg_name']}_CALLBACK(msg_data_a);\n")
        case_lines.append("                break;\n")
    c_lines = insert_lines(c_lines, gen_switch_case_start, gen_switch_case_stop, case_lines)

    # Stale checking
    stale_lines = []
    for msg in receiving_msg_defs:
        if msg['msg_period'] > 0:
            stale_lines.append(f"    CHECK_STALE(can_data.{msg['msg_name']}.stale,\n")
            stale_lines.append(f"                curr_tick, can_data.{msg['msg_name']}.last_rx,\n")
            stale_lines.append(f"                UP_{msg['msg_name'].upper()});\n")
    c_lines = insert_lines(c_lines, gen_stale_case_start, gen_stale_case_stop, stale_lines)

    # Hardware filtering
    filter_lines = []
    on_mask = False
    filter_bank = 0
    for msg in receiving_msg_defs:
        if(filter_bank > 27):
            generator.log_error(f"Max filter bank reached for node {node_config['node_name']}")
            quit()
        if not on_mask:
            filter_lines.append(f"    CAN1->FA1R |= (1 << {filter_bank});    // configure bank {filter_bank}\n")
            filter_lines.append(f"    CAN1->sFilterRegister[{filter_bank}].FR1 = (ID_{msg['msg_name'].upper()} << 3) | 4;\n")
            on_mask = True
        else:
            filter_lines.append(f"    CAN1->sFilterRegister[{filter_bank}].FR2 = (ID_{msg['msg_name'].upper()} << 3) | 4;\n")
            on_mask = False
            filter_bank += 1
    c_lines = insert_lines(c_lines, gen_filter_start, gen_filter_stop, filter_lines)
    
    # Rx IRQ callbacks
    rx_irq_lines = []
    for rx_irq in rx_irq_names:
        rx_irq_lines.append(f"            case ID_{rx_irq.upper()}:\n")
        rx_irq_lines.append(f"                {rx_irq}_IRQ(msg_data_a);\n")
        rx_irq_lines.append(f"                break;\n")
    c_lines = insert_lines(c_lines, gen_rx_irq_start, gen_rx_irq_stop, rx_irq_lines)
    

    # Write changes to source file
    with open(node_paths[1], "w") as c_file:
        c_file.writelines(c_lines)

def configure_bus(bus, source_dir, c_dir, h_dir):
    """ 
    Generates c code for each node on bus
    @param bus  bus dictionary configuration
    """
    print('Configuring Bus ' + bus['bus_name'])

    # extract node names from config
    node_names = []
    for node in bus['nodes']:
        node_names.append(node['node_name'])

    # find file paths for each node
    node_paths = find_node_paths(node_names, source_dir, c_dir, h_dir)
    matched_keys = node_paths.keys()

    # iterate through all matched nodes
    for node_key in matched_keys:
        # find the config for the node and configure it
        for node in bus['nodes']:
            if node_key == node['node_name']:
                configure_node(node, node_paths[node_key])
                break

def gen_embedded_can(config, source_dir, c_dir, h_dir):
    """ Generate can parsing code """

    global can_config
    can_config = config

    for bus in can_config['busses']:
        configure_bus(bus, source_dir, c_dir, h_dir)

    generator.log_success("Embedded Code Generated")

if __name__ == "__main__":
    gen_config = json.load(open(generator.GENERATOR_CONFIG_JSON_PATH))
    config = generator.load_message_config(gen_config['can_json_config_path'], gen_config['can_json_schema_path'])
    gen_embedded_can(config, gen_config['source_directory'], gen_config['node_parse_c_dir'], gen_config['node_parse_h_dir'])