""" gen_embedded_can.py: Generates embedded code for CAN message parsing using structures with bit fields """

import  generator

#
# GENERATION STRINGS
#
gen_id_start = "BEGIN AUTO ID DEFS"
gen_id_stop = "END AUTO ID DEFS"
gen_dlc_start = "BEGIN AUTO DLC DEFS"
gen_dlc_stop = "END AUTO DLC DEFS"
gen_send_macro_start = "BEGIN AUTO SEND MACROS"
gen_send_macro_stop = "END AUTO SEND MACROS"
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
gen_can_enums_start = "BEGIN AUTO CAN ENUMERATIONS"
gen_can_enums_stop = "END AUTO CAN ENUMERATIONS"

DEFAULT_PERIPHERAL = "CAN1"

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
            quit(1)
    
    return msg_defs

def gen_send_macro(lines, msg_config, peripheral):
    """ generates a send macro to add a message to the tx queue """
    cap = msg_config['msg_name'].upper()
    sig_args = ", ".join([sig['sig_name']+'_' for sig in msg_config['signals']])
    lines.append(f"#define SEND_{cap}(queue, {sig_args}) do {{\\\n")
    lines.append(f"        CanMsgTypeDef_t msg = {{.Bus={peripheral}, .ExtId=ID_{cap}, .DLC=DLC_{cap}, .IDE=1}};\\\n")
    lines.append(f"        CanParsedData_t* data_a = (CanParsedData_t *) &msg.Data;\\\n")
    for sig in msg_config['signals']:
        # if float, cannot simply cast to uint32, have to use union
        convert_str = f"FLOAT_TO_UINT32({sig['sig_name']}_)" if 'float' in sig['type'] else f"{sig['sig_name']}_"
        # conversion not necessary for signed integers (source: testing)
        lines.append(f"        data_a->{msg_config['msg_name']}.{sig['sig_name']} = {convert_str};\\\n")
    lines.append(f"        qSendToBack(&queue, &msg);\\\n")
    lines.append(f"    }} while(0)\n")

def gen_filter_lines(lines, rx_msg_configs, peripheral):
    """ generates hardware filters for a set of message definitions for a specific peripheral """
    on_mask = False
    filter_bank = 0
    filter_bank_max = 27

    if peripheral == "CAN1":
        filter_bank = 0
        filter_bank_max = 14
    elif peripheral == "CAN2":
        filter_bank = 14
        filter_bank_max = 27
    else:
        print(f"Unknown CAN peripheral {peripheral}")

    for msg in rx_msg_configs:
        if(filter_bank > filter_bank_max):
            generator.log_error(f"Max filter bank reached for node containing msg {msg['msg_name']}")
            quit(1)
        if not on_mask:
            lines.append(f"    CAN1->FA1R |= (1 << {filter_bank});    // configure bank {filter_bank}\n")
            lines.append(f"    CAN1->sFilterRegister[{filter_bank}].FR1 = (ID_{msg['msg_name'].upper()} << 3) | 4;\n")
            on_mask = True
        else:
            lines.append(f"    CAN1->sFilterRegister[{filter_bank}].FR2 = (ID_{msg['msg_name'].upper()} << 3) | 4;\n")
            on_mask = False
            filter_bank += 1

def gen_switch_case(lines, rx_msg_configs, rx_callbacks, ind=""):
    """ generates switch case for receiving messages """
    lines.append(ind+"        switch(msg_header.ExtId)\n")
    lines.append(ind+"        {\n")
    for msg in rx_msg_configs:
        lines.append(ind+f"            case ID_{msg['msg_name'].upper()}:\n")
        for sig in msg['signals']:
            var_str = f"msg_data_a->{msg['msg_name']}.{sig['sig_name']}"
            # converting from uint storage to either signed int or float
            convert_str = f"({sig['type']}) {var_str}" if ('int' in sig['type'] and 'u' not in sig['type']) else var_str
            convert_str = f"UINT32_TO_FLOAT({var_str})" if 'float' in sig['type'] else convert_str
            lines.append(ind+f"                can_data.{msg['msg_name']}.{sig['sig_name']} = {convert_str};\n")
        if msg['msg_period'] > 0:
            lines.append(ind+f"                can_data.{msg['msg_name']}.stale = 0;\n")
            lines.append(ind+f"                can_data.{msg['msg_name']}.last_rx = sched.os_ticks;\n")
        callback = [rx_config for rx_config in rx_callbacks if rx_config['msg_name'] == msg['msg_name']]
        if callback:
            if "arg_type" in callback[0] and callback[0]['arg_type'] == "header":
                lines.append(ind+f"                {msg['msg_name']}_CALLBACK(&msg_header);\n")
            else:
                lines.append(ind+f"                {msg['msg_name']}_CALLBACK(msg_data_a);\n")
        lines.append(ind+"                break;\n")
    lines.append(ind+"            default:\n")
    lines.append(ind+"                __asm__(\"nop\");\n")
    lines.append(ind+"        }\n")


def configure_node(node_config, node_paths):
    """ 
    Generates code for c and h files within a node
    @param  node_config     json config for the specific node
    @param  node_paths      paths to [h file, c file] for that node
    """

    print("Configuring Node " + node_config['node_name'])

    # Junction node?
    is_junc = False
    junc_config = None
    if 'is_junction' in node_config and node_config['is_junction']:
        is_junc = True
        print(f"Treating {node_config['node_name']} as junction")
        global can_config
        for bus in can_config['busses']:
            for node in bus['nodes']:
                if node['node_name'] == node_config['node_name'] and node['can_peripheral'] != node_config['can_peripheral']:
                    junc_config = node
                    break
            if junc_config: break

    # Combine message definitions
    raw_msg_defs = []
    raw_msg_defs += node_config['tx']
    if is_junc: raw_msg_defs += junc_config['tx']
    receiving_msg_defs = []
    node_specific_rx_msg_defs = find_rx_messages([rx_config["msg_name"] for rx_config in node_config['rx']])
    receiving_msg_defs += node_specific_rx_msg_defs
    junc_rx_msg_defs = []
    if is_junc: 
        junc_rx_msg_defs += find_rx_messages([rx_config['msg_name'] for rx_config in junc_config['rx']])
        receiving_msg_defs += junc_rx_msg_defs
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
    h_lines = generator.insert_lines(h_lines, gen_id_start, gen_id_stop, id_lines)
    h_lines = generator.insert_lines(h_lines, gen_dlc_start, gen_dlc_stop, dlc_lines)

    # Send Macros, requires knowledge of CAN peripheral
    macro_lines = []
    periph = DEFAULT_PERIPHERAL
    if 'can_peripheral' in node_config: periph = node_config['can_peripheral']
    for msg in node_config['tx']:
        gen_send_macro(macro_lines, msg, periph)
    if is_junc:
        periph = junc_config['can_peripheral']
        for msg in junc_config['tx']:
            gen_send_macro(macro_lines, msg, periph)
    h_lines = generator.insert_lines(h_lines, gen_send_macro_start, gen_send_macro_stop, macro_lines)

    # Message update periods
    up_lines = []
    for msg in receiving_msg_defs:
        if msg['msg_period'] > 0:
            up_lines.append(f"#define UP_{msg['msg_name'].upper()} {msg['msg_period']}\n")
    h_lines = generator.insert_lines(h_lines, gen_up_start, gen_up_stop, up_lines)

    # Define CanParsedData_t
    raw_struct_lines = []
    raw_struct_lines.append("typedef union { \n")
    for msg in raw_msg_defs:
        raw_struct_lines.append("    struct {\n")
        for sig in msg['signals']:
            raw_struct_lines.append(f"        uint64_t {sig['sig_name']}: {sig['length']};\n")
        raw_struct_lines.append(f"    }} {msg['msg_name']};\n") 
    raw_struct_lines.append("    uint8_t raw_data[8];\n")
    raw_struct_lines.append("} __attribute__((packed)) CanParsedData_t;\n")
    h_lines = generator.insert_lines(h_lines, gen_raw_struct_start, gen_raw_struct_stop, raw_struct_lines)

    # Define can_data_t
    can_struct_lines = []
    can_struct_lines.append("typedef struct {\n")
    for msg in receiving_msg_defs:
        can_struct_lines.append("    struct {\n")
        for sig in msg['signals']:
            dtype = sig['type']
            if 'choices' in sig: dtype = f"{sig['sig_name']}_t"
            can_struct_lines.append(f"        {dtype} {sig['sig_name']};\n")
        
        # stale checking variables
        if msg['msg_period'] > 0:
            can_struct_lines.append(f"        uint8_t stale;\n")
            can_struct_lines.append(f"        uint32_t last_rx;\n")

        can_struct_lines.append(f"    }} {msg['msg_name']};\n")
    can_struct_lines.append("} can_data_t;\n")
    h_lines = generator.insert_lines(h_lines, gen_can_struct_start, gen_can_struct_stop, can_struct_lines)

    # Enumerations from choices
    can_enum_lines = []
    for msg in raw_msg_defs:
        for sig in msg['signals']:
            if 'choices' in sig:
                can_enum_lines.append("typedef enum {\n")
                for choice in sig['choices']:
                    can_enum_lines.append(f"    {sig['sig_name'].upper()}_{choice.upper()},\n")
                can_enum_lines.append(f"}} {sig['sig_name']}_t;\n")
                can_enum_lines.append(f"\n")
    h_lines = generator.insert_lines(h_lines, gen_can_enums_start, gen_can_enums_stop, can_enum_lines)

    # Rx callbacks
    rx_callbacks = [rx_config for rx_config in node_config['rx'] if ("callback" in rx_config and rx_config["callback"])]
    if is_junc: rx_callbacks += [rx_config for rx_config in junc_config['rx'] if ("callback" in rx_config and rx_config["callback"])]
    extern_callback_lines = [f"extern void {rx_config['msg_name']}_CALLBACK(CanMsgTypeDef_t* msg_header_a);\n" for rx_config in rx_callbacks if ("arg_type" in rx_config and rx_config["arg_type"]=="header")]
    extern_callback_lines += [f"extern void {rx_config['msg_name']}_CALLBACK(CanParsedData_t* msg_data_a);\n" for rx_config in rx_callbacks if (("arg_type" not in rx_config) or rx_config["arg_type"]=="msg_data")]
    h_lines = generator.insert_lines(h_lines, gen_callback_start, gen_callback_stop, extern_callback_lines)

    rx_irq_names = [rx_config['msg_name'] for rx_config in node_config['rx'] if ("irq" in rx_config and rx_config["irq"])]
    if is_junc: rx_irq_names += [rx_config['msg_name'] for rx_config in junc_config['rx'] if ("irq" in rx_config and rx_config["irq"])]
    extern_callback_lines = [f"extern void {msg_name}_IRQ(CanParsedData_t* msg_data_a);\n" for msg_name in rx_irq_names]
    h_lines = generator.insert_lines(h_lines, gen_irq_extern_start, gen_irq_extern_stop, extern_callback_lines)


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
    periph = DEFAULT_PERIPHERAL
    if 'can_peripheral' in node_config: periph = node_config['can_peripheral']
    ind = ""
    if is_junc:
        ind = "    "
        # add if statement for distinguishing between peripherals
        case_lines.append(f"        if (msg_header.Bus == {periph})\n")
        case_lines.append(f"        {{\n")
    gen_switch_case(case_lines, node_specific_rx_msg_defs, rx_callbacks, ind=ind)
    if is_junc:
        periph = junc_config['can_peripheral']
        case_lines.append("        }\n")
        case_lines.append(f"        else if (msg_header.Bus == {periph})\n")
        case_lines.append("        {\n")
        gen_switch_case(case_lines, junc_rx_msg_defs, rx_callbacks, ind=ind)
        case_lines.append("        }\n")
    c_lines = generator.insert_lines(c_lines, gen_switch_case_start, gen_switch_case_stop, case_lines)

    # Stale checking
    stale_lines = []
    for msg in receiving_msg_defs:
        if msg['msg_period'] > 0:
            stale_lines.append(f"    CHECK_STALE(can_data.{msg['msg_name']}.stale,\n")
            stale_lines.append(f"                sched.os_ticks, can_data.{msg['msg_name']}.last_rx,\n")
            stale_lines.append(f"                UP_{msg['msg_name'].upper()});\n")
    c_lines = generator.insert_lines(c_lines, gen_stale_case_start, gen_stale_case_stop, stale_lines)

    # Hardware filtering
    filter_lines = []
    if not ("accept_all_messages" in node_config and node_config["accept_all_messages"]):
        periph = DEFAULT_PERIPHERAL
        if "can_peripheral" in node_config: periph = node_config['can_peripheral']    
        gen_filter_lines(filter_lines, node_specific_rx_msg_defs, periph)
        if is_junc: gen_filter_lines(filter_lines, junc_rx_msg_defs, junc_config['can_peripheral'])

    c_lines = generator.insert_lines(c_lines, gen_filter_start, gen_filter_stop, filter_lines)
    
    # Rx IRQ callbacks
    rx_irq_lines = []
    for rx_irq in rx_irq_names:
        rx_irq_lines.append(f"            case ID_{rx_irq.upper()}:\n")
        rx_irq_lines.append(f"                {rx_irq}_IRQ(msg_data_a);\n")
        rx_irq_lines.append(f"                break;\n")
    c_lines = generator.insert_lines(c_lines, gen_rx_irq_start, gen_rx_irq_stop, rx_irq_lines)
    
    # Write changes to source file
    with open(node_paths[1], "w") as c_file:
        c_file.writelines(c_lines)

def configure_bus(bus, source_dir, c_dir, h_dir):
    """ 
    Generates c code for each node on bus
    @param bus  bus dictionary configuration
    """
    print('Configuring Bus ' + bus['bus_name'])

    # extract node names from config, don't configure junction nodes yet
    node_names = [node['node_name'] for node in bus['nodes']]

    # find file paths for each node
    node_paths = generator.find_node_paths(node_names, source_dir, c_dir, h_dir)
    matched_keys = node_paths.keys()

    configured_nodes = []
    # iterate through all matched nodes
    for node_key in matched_keys:
        # find the config for the node and configure it if not already configured
        for node in bus['nodes']:
            if node_key == node['node_name'] and node['node_name'] not in configured_nodes:
                configure_node(node, node_paths[node_key])
                configured_nodes.append(node['node_name'])
                break

def gen_embedded_can(config, source_dir, c_dir, h_dir):
    """ Generate can parsing code """

    global can_config
    can_config = config

    for bus in can_config['busses']:
        configure_bus(bus, source_dir, c_dir, h_dir)

    generator.log_success("Embedded CAN Code Generated")