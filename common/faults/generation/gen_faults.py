""" gen_faults.py: Generates embedded code for fault messages/structure, CAN Tx/Rx functionality"""

import  generator

#
# GENERATION STRINGS FOR H FILE
#
gen_id_start = "BEGIN AUTO ID DEFS"
gen_id_stop = "END AUTO ID DEFS"
gen_totals_start = "BEGIN AUTO TOTAL DEFS"
gen_totals_stop = "END AUTO TOTAL DEFS"
gen_nodes_start = "BEGIN AUTO NODE DEFS"
gen_nodes_stop = "END AUTO NODE DEFS"
gen_priority_start = "BEGIN AUTO PRIORITY DEFS"
gen_priority_stop = "END AUTO PRIORITY DEFS"
gen_max_start = "BEGIN AUTO MAX DEFS"
gen_max_stop = "END AUTO MAX DEFS"
gen_min_start = "BEGIN AUTO MIN DEFS"
gen_min_stop = "END AUTO MIN DEFS"
gen_latch_start = "BEGIN AUTO LATCH DEFS"
gen_latch_stop = "END AUTO LATCH DEFS"
gen_unlatch_start = "BEGIN AUTO UNLATCH DEFS"
gen_unlatch_stop = "END AUTO UNLATCH DEFS"
gen_screenmsg_start = "BEGIN AUTO SCREENMSG DEFS"
gen_screenmsg_stop = "END AUTO SCREENMSG DEFS"

#
# GENERATION STRINGS FOR C FILE
#
gen_info_array_start = "BEGIN AUTO FAULT INFO ARRAY DEFS"
gen_info_array_stop = "END AUTO FAULT INFO ARRAY DEFS"

def gen_totals(fault_config):
    """
    Generate C definitions for the total numbers
    @param fault_config    Fault JSON dictionary

    @return          Array of macros to add to file
    """
    print("Generating Total Values")
    total_faults = 0
    total_mcus = 0
    total_array = []
    #Total nodes
    for node in fault_config['modules']:
        total_mcus += 1
        current_total = 0
        #Total faults in each node
        for fault in node['faults']:
            current_total += 1
            total_faults += 1
        total_array.append(f"#define TOTAL_{node['node_name'].upper()}_FAULTS {current_total}\n")
    total_array.append(f"#define TOTAL_MCU_NUM {total_mcus}\n")
    total_array.append(f"#define TOTAL_NUM_FAULTS {total_faults}\n")
    return total_array

def gen_nodes(fault_config) :
    """
    Generate C definitions for the each node, and thier value (fault_owner enum)
    @param fault_config    Fault JSON dictionary

    @return          Array of macros to add to file
    """
    print("Generating nodes")
    node_array = []
    for idx, node in enumerate(fault_config['modules']):
        node_array.append(f"#define NODE_{node['node_name'].upper()} {idx}\n")
    return node_array



def gen_ids(fault_config):
    """
    Generate C definitions for each fault representing thier ID
    @param fault_config    Fault JSON dictionary

    @return          Array of macros to add to file
    """
    print("Generating IDs")
    id_lines = []
    for node in fault_config['modules']:
        for fault in node['faults']:
            id_lines.append(f"#define ID_{fault['fault_name'].upper()}_FAULT {hex(fault['id'])}\n")
    return id_lines

def gen_priorities(fault_config):
    """
    Generate C definitions each fault and thier priorities
    @param fault_config    Fault JSON dictionary

    @return          Array of macros to add to file
    """
    print("Generating Priorities")
    pri_lines = []
    for node in fault_config['modules']:
        for fault in node['faults']:
            pri_lines.append(f"#define {fault['fault_name'].upper()}_PRIORITY {fault['pri_interp']}\n")
    return pri_lines

def gen_max(fault_config):
    """
    Generate C definitions for the each fault and thier max value
    @param fault_config    Fault JSON dictionary

    @return          Array of macros to add to file
    """
    print("Generating Max Values")
    max_lines = []
    for node in fault_config['modules']:
        for fault in node['faults']:
            max_lines.append(f"#define {fault['fault_name'].upper()}_MAX {fault['max']}\n")
    return max_lines

def gen_min(fault_config):
    """
    Generate C definitions for each fault and thier min values
    @param fault_config    Fault JSON dictionary

    @return          Array of macros to add to file
    """
    print("Generating Min Values")
    min_lines = []
    for node in fault_config['modules']:
        for fault in node['faults']:
            min_lines.append(f"#define {fault['fault_name'].upper()}_MIN {fault['min']}\n")
    return min_lines

def gen_latch(fault_config):
    """
    Generate C definitions for each fault and thier latch states
    @param fault_config    Fault JSON dictionary

    @return          Array of macros to add to file
    """
    print("Generating Latch Times")
    latch_lines = []
    for node in fault_config['modules']:
        for fault in node['faults']:
            latch_lines.append(f"#define {fault['fault_name'].upper()}_LATCH_TIME {fault['time_to_latch']}\n")
    return latch_lines

def gen_unlatch(fault_config):
    """
    Generate C definitions each fault and thier unlatch time
    @param fault_config    Fault JSON dictionary

    @return          Array of macros to add to file
    """
    print("Generating Unlatch Times")
    latch_lines = []
    for node in fault_config['modules']:
        for fault in node['faults']:
            latch_lines.append(f"#define {fault['fault_name'].upper()}_UNLATCH_TIME {fault['time_to_unlatch']}\n")
    return latch_lines

def gen_screenmsg(fault_config):
    """
    Generate C definitions for each fault and thier LCD message
    @param fault_config    Fault JSON dictionary

    @return          Array of macros to add to file
    """
    print("Generating Messages")
    msg = []
    for node in fault_config['modules']:
        for fault in node['faults']:
            msg.append(f"#define {fault['fault_name'].upper()}_MSG \"{fault['lcd_message']}\\0\" \n")
    return msg

def gen_fault_info_arrays(fault_config):
    """
    Generate C definitions combining each macro into an array that will be added to the file
    @param fault_config    Fault JSON dictionary

    @return          Array of macros to add to file
    """
    print("Populating Arrays")
    array = []
    array.append("uint16_t faultLatchTime[TOTAL_NUM_FAULTS] = {")
    i = 0
    #Add each latch value to an array, splitting when the line gets too long
    for node in fault_config['modules']:
        for fault in node['faults']:
            if i == 5:
                array.append(f" {fault['fault_name'].upper()}_LATCH_TIME,\n")
                i += 1
            elif i == 6:
                array.append(f"\t\t\t{fault['fault_name'].upper()}_LATCH_TIME,")
                i = 0
            else:
                array.append(f" {fault['fault_name'].upper()}_LATCH_TIME,")
                i += 1
    array.append("};\nuint16_t faultULatchTime[TOTAL_NUM_FAULTS] = {")
    i = 0
    #Add each unlatch value to an array, splitting when the line gets too long
    for node in fault_config['modules']:
        for fault in node['faults']:
            if i == 5:
                array.append(f" {fault['fault_name'].upper()}_UNLATCH_TIME,\n")
                i += 1
            elif i == 6:
                array.append(f"\t\t\t{fault['fault_name'].upper()}_UNLATCH_TIME,")
                i = 0
            else:
                array.append(f" {fault['fault_name'].upper()}_UNLATCH_TIME,")
                i += 1
    array.append("};\n//Global arrays with all faults\nfault_status_t statusArray[TOTAL_NUM_FAULTS] = {\n")
    for node in fault_config['modules']:
        for fault in node['faults']:
                array.append(f"\t(fault_status_t){{false, ID_{fault['fault_name'].upper()}_FAULT}},\n")
    array.append("};\nfault_attributes_t faultArray[TOTAL_NUM_FAULTS] = {\n")
    idx = 0
    for node in fault_config['modules']:
        for fault in node['faults']:
            array.append(f"\t(fault_attributes_t){{false, false, {fault['fault_name'].upper()}_PRIORITY, 0, 0, {fault['fault_name'].upper()}_MAX, \
{fault['fault_name'].upper()}_MIN, &statusArray[{idx}], 0, {fault['fault_name'].upper()}_MSG}}, \n")
            idx += 1
    array.append("};\n")

    return array




def gen_rx_msg(fault_config):
    """
    Generate C functions to handle CAN callbacks when recieving fault data
    @param fault_config    Fault JSON dictionary

    @return          Array of macros to add to file
    """
    print("Generating Fault Functions")
    rx = []
    for node in fault_config['modules']:
        msg = f"void fault_sync_{node['can_name'].lower()}_CALLBACK(CanParsedData_t *msg_header_a) {{\n"
        if msg in rx:
            generator.log_warning("Multiple fault nodes refer to the same CAN Node")
            continue
        #Generate the function logic
        rx.append(f"//void fault_sync_{node['can_name'].lower()}_CALLBACK(CanParsedData_t *msg_header_a) {{\n")
        rx.append(f"\t//fault_status_t recievedStatus = {{msg_header_a->fault_sync_{node['can_name'].lower()}.latched, msg_header_a->fault_sync_{node['can_name'].lower()}.idx}};\n")
        rx.append("\t//handleCallbacks(recievedStatus);\n")
        rx.append("//}\n")
    return rx




def gen_faults(config, c, h, nodes):
    #
    # Configure header file ------------------
    #
    with open(h, "r") as h_file:
        h_lines = h_file.readlines()

    h_lines = generator.insert_lines(h_lines, gen_totals_start, gen_totals_stop, gen_totals(config))
    h_lines = generator.insert_lines(h_lines, gen_id_start, gen_id_stop, gen_ids(config))
    h_lines = generator.insert_lines(h_lines, gen_priority_start, gen_priority_stop, gen_priorities(config))
    h_lines = generator.insert_lines(h_lines, gen_max_start, gen_max_stop, gen_max(config))
    h_lines = generator.insert_lines(h_lines, gen_min_start, gen_min_stop, gen_min(config))
    h_lines = generator.insert_lines(h_lines, gen_latch_start, gen_latch_stop, gen_latch(config))
    h_lines = generator.insert_lines(h_lines, gen_unlatch_start, gen_unlatch_stop, gen_unlatch(config))
    h_lines = generator.insert_lines(h_lines, gen_screenmsg_start, gen_screenmsg_stop, gen_screenmsg(config))


    # Write changes to header file
    with open(h, "w") as h_file:
        h_file.writelines(h_lines)

    #
    # Configure c file ------------------
    #
    with open(c, "r") as c_file:
        c_lines = c_file.readlines()

    c_lines = generator.insert_lines(c_lines, gen_info_array_start, gen_info_array_stop, gen_fault_info_arrays(config))

    # Write changes to c file
    with open(c, "w") as c_file:
        c_file.writelines(c_lines)


    with open(nodes, "r") as node_file:
        node_lines = node_file.readlines()

    node_lines = generator.insert_lines(node_lines, gen_nodes_start, gen_nodes_stop, gen_nodes(config))

    with open(nodes, "w") as node_file:
        node_file.writelines(node_lines)