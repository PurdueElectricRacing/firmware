""" gen_embedded_daq.py: Adds can msg defs for daq protocol and generates the required embedded code """

import generator
import gen_embedded_can
import os

def _generate_uds_can_msgs(daq_config, can_config, _name, _hlp):
    """ generates message definitions for daq commands and responses """

    for daq_bus in daq_config['busses']:
        if (daq_bus['bus_name'] != "VCAN"): continue
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

        # Since CAN is message-based we need a sender and a receiver
        # If we want to send over TCP with DAQ as CAN gateway,
        # DAQ
        # tx: uds_command_main_module (relay from tcp to can)
        # rx: uds_response_main_module (relay from can to tcp)
        # NODE
        # tx: uds_response_main_module (tx over can)
        # rx: uds_command_main_module  (rx over can)

        # Special case, for UDS to DAQ, canable acts as DAQ

        # Find daq node
        for node in can_bus['nodes']:
            print(node['node_name'])
        daq_node = [node for node in can_bus['nodes'] if node['node_name'] == "daq"][0]
        canable_node = [node for node in can_bus['nodes'] if node['node_name'] == "canable"][0]

        bl_node = [node for node in can_bus['nodes'] if node['node_name'] == "bootloader"][0]
        #bs_node = [node for node in can_bus['nodes'] if node['node_name'] == "BITSTREAM"][0]
        part_nodes = daq_bus['nodes']
        for i,daq_node_config in enumerate(daq_bus['nodes']):
            # get node's ssa
            if daq_node_config['node_name'] == "daq":
                gate_node = canable_node
            else:
                gate_node = daq_node
            ssa = -1
            for can_node in can_bus['nodes']:
                if can_node['node_name'] == daq_node_config['node_name']:
                    if (can_node['node_name'] == "torque_vector" and _name == "daq"):
                        ssa = 0
                        break # no daq protocol for tv
                    print(f"match for can node {can_node['node_name']} found")
                    ssa = can_node['node_ssa']
                    name = daq_node_config['node_name'].lower()

                    # configure node rx message
                    _node_name = name.upper() if _name == "daq" else name
                    if (_name == "uds"):
                        cmd_msg = {"msg_name":f"{_name}_command_{_node_name}", "callback":True}
                    else:
                        cmd_msg = {"msg_name":f"{_name}_command_{_node_name}", "callback":True, "irq":False, "arg_type":"header"}
                    can_node['rx'].append(cmd_msg)
                    if (_name == "uds"): bl_node['rx'].append(cmd_msg)

                    # configure node tx message
                    rsp_msg = {"msg_name":f"{_name}_response_{_node_name}",
                                    "msg_desc":f"{_name.upper()} response from node {daq_node_config['node_name']}",
                                    "signals":[{"sig_name":"payload","type":"uint64_t","length":64}],
                                    "msg_period":0, "msg_hlp":_hlp, "msg_pgn":100 + i}
                    can_node['tx'].append(rsp_msg)
                    if (_name == "uds"): bl_node['tx'].append(rsp_msg)

                    # configure daq tx message
                    command_msg = {"msg_name":f"{_name}_command_{_node_name}",
                           "msg_desc":f"{_name.upper()} command for node {daq_node_config['node_name']}",
                           "signals":[{"sig_name":"payload","type":"uint64_t","length":64}],
                           "msg_period":0, "msg_hlp":_hlp, "msg_pgn":200 + i}
                    gate_node['tx'].append(command_msg)
                    # configure daq tx message
                    gate_node['rx'].append({"msg_name":f"{_name}_response_{_node_name}"})

                    # add to daq config
                    periph = gen_embedded_can.DEFAULT_PERIPHERAL
                    if 'can_periperal' in can_node: periph = can_node['can_peripheral']
                    daq_node_config['daq_rsp_msg_periph'] = periph

                    break
            if ssa == -1:
                generator.log_error(f"CAN node name not found: {daq_node_config['node_name']}")
                quit(1)

def generate_daq_can_msgs(daq_config, can_config):
    _generate_uds_can_msgs(daq_config, can_config, "daq", 5)

def generate_uds_can_msgs(daq_config, can_config):
    _generate_uds_can_msgs(daq_config, can_config, "uds", 6)

def configure_node(node_config):
    """
    Generates code for c and h files within a node
    @param  node_config     json config for the specific node
    """
    print(f"Configuring UDS for Node " + node_config['node_name'])

    #
    # Configure header file -----------
    #
    #with open(node_paths[0], "r") as h_file:
    #    h_lines = h_file.readlines()
    warning_msg = """
/* THIS FILE WAS AUTOGENERATED BY common/daq/generation/gen_embedded_daq.py */
/* DO NOT EDIT */
/* ADD/EDIT VARIABLES IN THE JSON common/daq/generation/daq_config.json INSTEAD */
""".strip()
    out_lines = """
/**
 * @file uds.h
 * @author Eileen Yoon (eyn@purdue.edu)
 * @brief  Unified Diagnostics Services (UDS)
 * @version 0.1
 * @date 2024-12-18
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef __%s_UDS_H__
#define __%s_UDS_H__

%s
""" % (node_config['node_name'].upper(), node_config['node_name'].upper(), warning_msg)
    # #include "common/uds/uds.h"
    assert(all(var['var_name'].isidentifier() for var in node_config['variables']))

    # define NUM_VARS
    num_vars = len(node_config['variables'])
    out_lines += "\n%-44s %d\n\n" % ("#define UDS_NUM_VARS", num_vars)

    # define variable IDs
    var_id_lines = ["%-42s 0x%02x\n" % (f"#define UDS_ID_{var['var_name'].upper()}", idx) for idx, var in enumerate(node_config['variables'])]
    out_lines += ''.join(var_id_lines)

    # function prototypes
    out_lines += "\nvoid udsInit(void);\n"
    if (node_config['node_name'] != "daq"):
        out_lines += "void udsFrameSend(uint64_t data);\n"
    #out_lines += "bool udsInitBase(uds_variable_t *uds_tracked_vars_, uint32_t uds_num_vars_);\n"
    out_lines += "void uds_command_%s_CALLBACK(uint64_t payload);\n" % (node_config['node_name'].lower())
    out_lines += "\n" + warning_msg + "\n"
    out_lines += f"\n#endif // __{node_config['node_name'].upper()}_UDS_H__\n"

    # Write changes to header file
    dirname = os.path.join(f"source/{node_config['node_name'].lower()}/uds")
    os.makedirs(dirname, exist_ok=True)
    c_path = os.path.join(dirname, "uds.h")
    with open(c_path, "w") as f:
        f.write(out_lines)

    out_lines = """
/**
 * @file uds.c
 * @author Eileen Yoon (eyn@purdue.edu)
 * @brief  Unified Diagnostics Services (UDS)
 * @version 0.1
 * @date 2024-12-18
 *
 * @copyright Copyright (c) 2024
 *
 */

%s

#include "common/uds/uds.h"
#include "uds.h"
#include "can_parse.h"

""" % (warning_msg)
    var_includes = "/* Module includes */\n" + node_config['includes'] + "\n"
    out_lines += var_includes

    out_lines += "\n#define UVAR(x) ((uint32_t)(&(x)))\n"

    """
    uds_variable_t uds_tracked_vars[UDS_NUM_VARS] = {
    {.id = DAQ_ID_SDC_MAIN_STATUS, .perm = (UDS_VAR_READ_FLAG), .addr = (uint32_t)&sdc_mux.main_stat, .size = 1, },
    {.id = DAQ_ID_SDC_CENTER_STOP_STATUS, .perm = (UDS_VAR_READ_FLAG), .addr = (uint32_t)&sdc_mux.c_stop_stat, .size = 1, },
    };
    """
    table_lines = "\nuds_variable_t uds_tracked_vars[UDS_NUM_VARS] = {\n"
    for idx, var in enumerate(node_config['variables']):
        name = var['var_name']
        perm = "(UDS_VAR_READ_FLAG"
        if (not var['read_only']):
            perm += " | UDS_VAR_WRITE_FLAG"
        perm += ")"
        #addr = f"(uint32_t)(&{var['access_phrase']})"
        addr = f"UVAR({var['access_phrase']})"
        l = var['length']
        entry = "    { .id=%s, .addr=%s, .len=%d, .perm=%s},\n" % (f"UDS_ID_{name.upper()}", addr, l, perm)
        table_lines += entry
    table_lines += "};\n"
    out_lines += table_lines

    s = """
void udsFrameSend(uint64_t data)
{
    SEND_UDS_RESPONSE_%s(data);
}
""" % (node_config['node_name'].upper())
    if (node_config['node_name'] == "daq"):
        s = """
        """

    fn_lines = """
void udsInit(void)
{
    udsInitBase(uds_tracked_vars, UDS_NUM_VARS);
}

%s

void uds_command_%s_CALLBACK(uint64_t payload)
{
    uint8_t cmd = payload & 0xff;
    uint64_t data = (payload >> 8); /* store u48 in u64 */
    uds_handle_command(cmd, data);
}

%s
""" % (s.strip(), node_config['node_name'].lower(), warning_msg)
    out_lines += fn_lines

    # Write changes to c file
    dirname = os.path.join(f"source/{node_config['node_name'].lower()}/uds")
    os.makedirs(dirname, exist_ok=True)
    c_path = os.path.join(dirname, "uds.c")
    with open(c_path, "w") as f:
        f.write(out_lines)

    return

def configure_bus(bus, source_dir, c_dir, h_dir):
    """ Generates daq code for nodes on a bus """

    for node in bus['nodes']:
        configure_node(node)

def gen_embedded_daq(daq_conf, source_dir, c_dir, h_dir):
    """ Generate daq code """

    matched_nodes: list[dict] = []

    all_node_paths = {}

    for bus in daq_conf['busses']:
        node_names = [node['node_name'] for node in bus['nodes']]
        node_paths = generator.find_node_paths(node_names, source_dir, c_dir, h_dir)

        for path, posixPath in node_paths.items():
            if path not in all_node_paths:
                all_node_paths.update({path: posixPath})

        # matched_nodes.extend([node for node in bus['nodes'] if node['node_name'] in node_paths.keys()] and not any(d['node_name'] == node['node_name'] for d in matched_nodes))
        for node in bus['nodes']:
            if node['node_name'] in node_paths.keys() and not any(d.get('node_name') == node['node_name'] for d in matched_nodes):
                matched_nodes.append(node)
                # Add the bus the node is on to the node name
                node['bus_names'] = [bus['bus_name']]
            else:
                # The node already exists in the matched nodes list
                for matched_node in matched_nodes:
                    if matched_node['node_name'] == node['node_name']:
                        matched_node['bus_names'].append(bus['bus_name'])
                # TODO: logic that handles nodes that already exist (adding DAQ variables that are still needed, with an existing node) - non issue currently because we do not have different daq variables per different busses

    for node in matched_nodes:
        configure_node(node)
        #configure_node(node, all_node_paths[node['node_name']])

    #generator.log_success("Embedded DAQ Code Generated")
    #configure_bus(bus, source_dir, c_dir, h_dir)
    generator.log_success("Embedded DAQ Code Generated")

def gen_can_dbc(can_config, path):
    # pop bl node to remove identical names in daqapp
    found = -1
    for i,node in enumerate(can_config['busses'][0]["nodes"]):
        if (node["node_name"] == "bootloader"):
            found = i
            break
    if (found >= 0):
        del can_config['busses'][0]["nodes"][found]
    import json
    with open(path, 'w', encoding='utf-8') as f:
        json.dump(can_config, f)
    generator.log_success("Embedded CAN Code Generated")
