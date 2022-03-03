""" gen_faults.py: Generates headers and lcd messages for faults """

import os
import generator

#
# GENERATION STRINGS
#

gen_fault_header_start = "BEGIN AUTO FAULT STRUCTURES"
gen_fault_header_stop = "END AUTO FAULT STRUCTURES"

def gen_struct(node_config):
    """ Generates single fault header """

    # find number of critical and warning faults
    fault_class_based = {}
    for fault in node_config['faults']:
        if (fault['class'] not in fault_class_based): fault_class_based[fault['class']] = [fault]
        else: fault_class_based[fault['class']].append(fault)

    fault_class_dtypes = {}
    # find data type for each class
    for fclass, flist in fault_class_based.items():
        amt = len(flist)
        if (amt > max(generator.data_lengths)):
            generator.log_error(f"Max number of faults for {node_config['node_name']} class {fclass} exceeded")
            generator.log_error(f"Found {amt}, but max is {max(generator.data_lengths)}")
            quit(1)
        fault_class_dtypes[fclass] = min([i for i in generator.data_lengths if i > amt])

    # Generate fault structure 
    lines = []
    lines.append("typedef struct\n")
    lines.append("{\n")

    for fclass, flist in fault_class_based.items():
        lines.append("    struct {\n")
        dtype = f"uint{fault_class_dtypes[fclass]}_t"
        for flt in flist:
            lines.append(f"        {dtype} {flt['name'].lower()}: 1;\n")
        lines.append(f"    }} {fclass};\n")

    lines.append(f"}} faults_{node_config['node_name'].lower()}_t;\n")
    lines.append("\n")

    return lines

def gen_header(h_path):
    """ Generates fault headers """
    global fault_config

    # generate fault struct for each node
    node_names = []
    struct_lines = []
    for node in fault_config['nodes']:
        if node['node_name'] not in node_names:
            node_names.append(node['node_name'])
            struct_lines += gen_struct(node)
        else:
            generator.log_error(f"{node['node_name']} is defined twice in fault config")
            quit(1)

    with open(h_path, "r") as h_file:
        h_lines = h_file.readlines()
    h_lines = generator.insert_lines(h_lines, gen_fault_header_start, gen_fault_header_stop, struct_lines)
    with open(h_path, "w") as h_file:
        h_file.writelines(h_lines)
    generator.log_success("Fault Headers Generated")

def gen_faults(config, h_path):
    """ Generate fault headers and lcd"""
    global fault_config
    fault_config = config
    gen_header(h_path)
