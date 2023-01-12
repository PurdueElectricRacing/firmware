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
gen_enum_start = "BEGIN AUTO ENUM DEFS"
gen_enum_stop = "END AUTO ENUM DEFS"

#
# GENERATION STRINGS FOR C FILE
#
gen_info_array_start = "BEGIN AUTO FAULT INFO ARRAY DEFS"
gen_info_array_stop = "END AUTO FAULT INFO ARRAY DEFS"
gen_includes_start = "BEGIN AUTO INCLUDES"
gen_includes_end = "END AUTO INCLUDES"
gen_tx_start = "BEGIN AUTO TX COMMAND"
gen_tx_end = "END AUTO TX COMMAND"
gen_tx_specific_start = "BEGIN AUTO TX COMMAND SPECIFIC"
gen_tx_specific_end = "END AUTO TX COMMAND SPECIFIC"
gen_recieve_start = "BEGIN AUTO RECIEVE FUNCTIONS"
gen_recieve_end = "END AUTO RECIEVE FUNCTIONS"

def gen_totals(fault_config):
    print("Generating Total Values")
    total_faults = 0
    total_mcus = 0
    total_array = []
    for node in fault_config['modules']:
        total_mcus += 1
        current_total = 0
        for fault in node['faults']:
            current_total += 1
            total_faults += 1
        total_array.append(f"#define TOTAL_{node['node_name'].upper()}_FAULTS {current_total}\n")
    total_array.append(f"#define TOTAL_MCU_NUM {total_mcus}\n")
    total_array.append(f"#define TOTAL_NUM_FAULTS {total_faults}\n")
    return total_array

def gen_nodes(fault_config) :
    print("Generating nodes")
    node_array = []
    node_num = 0
    for node in fault_config['modules']:
        node_array.append(f"#define NODE_{node['node_name'].upper()} {node_num}\n")
        node_num += 1
    return node_array



def gen_ids(fault_config):
    print("Generating IDs")
    id_lines = []
    for node in fault_config['modules']:
        for fault in node['faults']:
            id_lines.append(f"#define ID_{fault['fault_name'].upper()}_FAULT {hex(fault['id'])}\n")
    return id_lines

def gen_priorities(fault_config):
    print("Generating Priorities")
    pri_lines = []
    for node in fault_config['modules']:
        for fault in node['faults']:
            pri_lines.append(f"#define {fault['fault_name'].upper()}_PRIORITY {fault['pri_interp']}\n")
    return pri_lines

def gen_max(fault_config):
    print("Generating Max Values")
    max_lines = []
    for node in fault_config['modules']:
        for fault in node['faults']:
            max_lines.append(f"#define {fault['fault_name'].upper()}_MAX {fault['max']}\n")
    return max_lines

def gen_min(fault_config):
    print("Generating Min Values")
    min_lines = []
    for node in fault_config['modules']:
        for fault in node['faults']:
            min_lines.append(f"#define {fault['fault_name'].upper()}_MIN {fault['min']}\n")
    return min_lines

def gen_latch(fault_config):
    print("Generating Latch Times")
    latch_lines = []
    for node in fault_config['modules']:
        for fault in node['faults']:
            latch_lines.append(f"#define {fault['fault_name'].upper()}_LATCH_TIME {fault['time_to_latch']}\n")
    return latch_lines

def gen_unlatch(fault_config):
    print("Generating Unlatch Times")
    latch_lines = []
    for node in fault_config['modules']:
        for fault in node['faults']:
            latch_lines.append(f"#define {fault['fault_name'].upper()}_UNLATCH_TIME {fault['time_to_unlatch']}\n")
    return latch_lines

def gen_screenmsg(fault_config):
    print("Generating Messages")
    msg = []
    for node in fault_config['modules']:
        for fault in node['faults']:
            msg.append(f"#define {fault['fault_name'].upper()}_MSG \"{fault['lcd_message']}\\0\" \n")
    return msg

def gen_node_enum(fault_config):
    print("Generating ENUMs")
    enum = []
    enum.append("typedef enum {\n")
    for node in fault_config['modules']:
        enum.append(f"\t{node['node_name'].upper()} = {node['name_interp']},\n")
    enum.append("} fault_owner_t;\n")
    return enum

def gen_fault_info_arrays(fault_config):
    print("Populating Arrays")
    array = []
    array.append("int idArray[TOTAL_NUM_FAULTS] = {")
    i = 0
    for node in fault_config['modules']:
        for fault in node['faults']:
            if i == 5:
                array.append(f" ID_{fault['fault_name'].upper()}_FAULT,\n")
                i += 1
            elif i == 6:
                array.append(f"\t\t\tID_{fault['fault_name'].upper()}_FAULT,")
                i = 0
            else:
                array.append(f" ID_{fault['fault_name'].upper()}_FAULT,")
                i += 1
    array.append("};\nint maxArray[TOTAL_NUM_FAULTS] = {")
    i = 0
    for node in fault_config['modules']:
        for fault in node['faults']:
            if i == 5:
                array.append(f" {fault['fault_name'].upper()}_MAX,\n")
                i += 1
            elif i == 6:
                array.append(f"\t\t\t{fault['fault_name'].upper()}_MAX,")
                i = 0
            else:
                array.append(f" {fault['fault_name'].upper()}_MAX,")
                i += 1
    array.append("};\nint minArray[TOTAL_NUM_FAULTS] = {")
    i = 0
    for node in fault_config['modules']:
        for fault in node['faults']:
            if i == 5:
                array.append(f" {fault['fault_name'].upper()}_MIN,\n")
                i += 1
            elif i == 6:
                array.append(f"\t\t\t{fault['fault_name'].upper()}_MIN,")
                i = 0
            else:
                array.append(f" {fault['fault_name'].upper()}_MIN,")
                i += 1
    array.append("};\nfault_priority_t priorityArray[TOTAL_NUM_FAULTS] = {")
    i = 0
    for node in fault_config['modules']:
        for fault in node['faults']:
            if i == 5:
                array.append(f" {fault['fault_name'].upper()}_PRIORITY,\n")
                i += 1
            elif i == 6:
                array.append(f"\t\t\t{fault['fault_name'].upper()}_PRIORITY,")
                i = 0
            else:
                array.append(f" {fault['fault_name'].upper()}_PRIORITY,")
                i += 1
    array.append("};\nchar msgArray[TOTAL_NUM_FAULTS][MAX_MSG_SIZE] = {")
    i = 0
    for node in fault_config['modules']:
        for fault in node['faults']:
            if i == 5:
                array.append(f" {fault['fault_name'].upper()}_MSG,\n")
                i += 1
            elif i == 6:
                array.append(f"\t\t\t{fault['fault_name'].upper()}_MSG,")
                i = 0
            else:
                array.append(f" {fault['fault_name'].upper()}_MSG,")
                i += 1
    array.append("};\nint faultLatchTime[TOTAL_NUM_FAULTS] = {")
    i = 0
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
    array.append("};\nint faultULatchTime[TOTAL_NUM_FAULTS] = {")
    i = 0
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
    array.append("};\n")
    return array


def gen_includes(fault_config):
    print("Generating Includes")
    gen_arr = []
    idx = 0
    for node in fault_config['modules']:
        # if node['node_name'] == target:
            # gen_arr.append(f"#include \"source/{node['can_name'].lower()}/can/can_parse.h\"\n")
        gen_arr.append(f"#if FAULT_NODE_NAME == {idx}\n\t#include \"source/{node['can_name'].lower()}/can/can_parse.h\"\n#endif\n")
        idx += 1
    return gen_arr

def gen_tx_msg(fault_config):
    print("Generating CAN TX Commands")
    tx = []
    idx = 0
    # tx.append("\t\tswitch(FAULT_NODE_NAME) {\n")
    for node in fault_config['modules']:
        # tx.append(f"\t\t\tcase {node['node_name'].upper()}:\n \
        #     \tSEND_FAULT_SYNC_{node['can_name'].upper()}(*q_tx, message->f_ID, message->latched);\n \
        #     \tbreak;\n")
        tx.append(f"\t\t\t#if FAULT_NODE_NAME == {idx}\n \
            \tSEND_FAULT_SYNC_{node['can_name'].upper()}(*q_tx, message->f_ID, message->latched);\n \
            #endif\n")
        idx += 1
        # if node['node_name'] == target:
        #     tx.append(f"\t\tSEND_FAULT_SYNC_{node['can_name'].upper()}(*q_tx, message->f_ID, message->latched);\n")
    # tx.append("\t\t}\n")
    return tx

def gen_rx_msg(fault_config):
    print("Generating Fault Functions")
    rx = []
    for node in fault_config['modules']:
        msg = f"void fault_sync_{node['can_name'].lower()}_CALLBACK(CanParsedData_t *msg_header_a) {{\n"
        # if node['node_name'] != target:
        if msg in rx:
            generator.log_warning("Multiple fault nodes refer to the same CAN Node")
            continue
        rx.append(f"void fault_sync_{node['can_name'].lower()}_CALLBACK(CanParsedData_t *msg_header_a) {{\n")
        rx.append(f"\tfault_message_t recievedMessage = {{msg_header_a->fault_sync_{node['can_name'].lower()}.latched, msg_header_a->fault_sync_{node['can_name'].lower()}.idx}};\n")
        rx.append("\tfault_message_t *currMessage = &messageArray[GET_IDX(recievedMessage.f_ID)];\n\thandleCallbacks(recievedMessage, currMessage);\n")
    #     rx.append("\tswitch (recievedMessage.latched) {\n\t\tcase true:\n\t\t\tif (!currMessage->latched) {\
    #         \n\t\t\t\tcurrMessage->latched = recievedMessage.latched;\n\t\t\t\tswitch(faultArray[GET_IDX(recievedMessage.f_ID)].priority) {\
    #         \n\t\t\t\t\tcase INFO:\n\t\t\t\t\t\tinfoCount++;\n\t\t\t\t\t\tbreak;\n\t\t\t\t\tcase WARNING:\
    #         \n\t\t\t\t\t\twarnCount++;\n\t\t\t\t\t\tbreak;\n\t\t\t\t\tcase CRITICAL:\n\t\t\t\t\t\tcritCount++;\n\t\t\t\t\t\tbreak;\n\t\t\t\t}\n\t\t\t}\
    #         \n\t\t\tbreak;\n\t\tcase false:\n\t\t\tif (currMessage->latched) {\n\t\t\t\tcurrMessage->latched = recievedMessage.latched;\
    #         \n\t\t\t\tswitch(faultArray[GET_IDX(recievedMessage.f_ID)].priority) {\n\t\t\t\t\tcase INFO:\n\t\t\t\t\t\tinfoCount--;\n\t\t\t\t\t\tbreak;\
    #         \n\t\t\t\t\tcase WARNING:\n\t\t\t\t\t\twarnCount--;\n\t\t\t\t\t\tbreak;\n\t\t\t\t\tcase CRITICAL:\n\t\t\t\t\t\tcritCount--;\n\t\t\t\t\t\tbreak;\
    #         \n\t\t\t\t}\n\t\t\t}\n\t\t\tbreak;\n\t\t}\n}\n\n\n")
        rx.append("}\n")
        # else:
        #     if msg in rx:
        #         generator.log_warning("Multiple fault nodes refer to the same CAN Node")
        #         continue
        #     rx.append(f"void fault_sync_{node['can_name'].lower()}_CALLBACK(CanParsedData_t *msg_header_a) {{\n")
        #     rx.append("\treturn;\n}\n")
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
    h_lines = generator.insert_lines(h_lines, gen_enum_start, gen_enum_stop, gen_node_enum(config))


    # Write changes to header file
    with open(h, "w") as h_file:
        h_file.writelines(h_lines)

    #
    # Configure c file ------------------
    #
    with open(c, "r") as c_file:
        c_lines = c_file.readlines()

    c_lines = generator.insert_lines(c_lines, gen_info_array_start, gen_info_array_stop, gen_fault_info_arrays(config))
    c_lines = generator.insert_lines(c_lines, gen_includes_start, gen_includes_end, gen_includes(config))
    c_lines = generator.insert_lines(c_lines, gen_tx_start, gen_tx_end, gen_tx_msg(config))
    c_lines = generator.insert_lines(c_lines, gen_tx_specific_start, gen_tx_specific_end, gen_tx_msg(config))
    c_lines = generator.insert_lines(c_lines, gen_recieve_start, gen_recieve_end, gen_rx_msg(config))

    # Write changes to c file
    with open(c, "w") as c_file:
        c_file.writelines(c_lines)


    with open(nodes, "r") as node_file:
        node_lines = node_file.readlines()

    node_lines = generator.insert_lines(node_lines, gen_nodes_start, gen_nodes_stop, gen_nodes(config))

    with open(nodes, "w") as node_file:
        node_file.writelines(node_lines)