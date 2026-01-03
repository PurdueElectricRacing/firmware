from typing import List, Dict, Tuple
from parser import Node, Message, load_custom_types
from utils import load_json, BUS_CONFIG_PATH, GENERATED_DIR, print_as_success

def to_macro_name(name: str) -> str:
    return name.upper()

def generate_headers(nodes: List[Node]):
    # Load bus configs
    bus_configs = load_json(BUS_CONFIG_PATH)
    busses = {b['name']: b for b in bus_configs['busses']}
    custom_types = load_custom_types()
    
    # Aggregate messages by bus
    bus_messages: Dict[str, List[Message]] = {name: [] for name in busses}
    
    for node in nodes:
        for bus_name, bus in node.busses.items():
            if bus_name in bus_messages:
                bus_messages[bus_name].extend(bus.tx_messages)
    
    # Ensure generated directory exists
    GENERATED_DIR.mkdir(exist_ok=True)

    # Generate header for each bus
    for bus_name, config in busses.items():
        generate_bus_header(bus_name, config, bus_messages.get(bus_name, []), custom_types)

    # Generate header for each node
    generate_node_headers(nodes, busses, custom_types)

    # Generate router header
    generate_router_header(nodes)

def generate_router_header(nodes: List[Node]):
    filename = GENERATED_DIR / "can_router.h"
    
    with open(filename, 'w') as f:
        f.write("#ifndef CAN_ROUTER_H\n")
        f.write("#define CAN_ROUTER_H\n\n")
        
        internal_nodes = sorted([n for n in nodes if not n.is_external], key=lambda x: x.name)
        
        if internal_nodes:
            for i, node in enumerate(internal_nodes):
                directive = "#if" if i == 0 else "#elif"
                f.write(f"{directive} defined(NODE_{node.name})\n")
                f.write(f'\t#include "{node.name}.h"\n')
            f.write("#endif\n")
            
        f.write("\n#endif\n")
    print_as_success(f"Generated {filename.name}")

def generate_node_headers(nodes: List[Node], bus_configs: Dict, custom_types: Dict):
    # Build global message map for RX lookup
    all_messages: Dict[str, Message] = {}
    for node in nodes:
        for bus in node.busses.values():
            for msg in bus.tx_messages:
                all_messages[msg.name] = msg

    for node in nodes:
        if node.is_external:
            continue
        generate_node_header(node, all_messages, bus_configs, custom_types)

def generate_node_header(node: Node, all_messages: Dict[str, Message], bus_configs: Dict, custom_types: Dict):
    filename = GENERATED_DIR / f"{node.name}.h"
    
    with open(filename, 'w') as f:
        f.write(f"#ifndef {node.name}_H\n")
        f.write(f"#define {node.name}_H\n\n")
        
        # Includes
        for bus_name in sorted(node.busses.keys()):
            f.write(f'#include "{bus_name}.h"\n')
        f.write('\n#include <string.h>\n')
        f.write('#include "common/can_library/can_common.h"\n\n')
        
        # Macros
        peripherals = sorted(list(set(bus.peripheral for bus in node.busses.values())))
        f.write(f"#define NUM_CAN_PERIPHERALS {len(peripherals)}\n")
        for peripheral in peripherals:
            f.write(f"#define USE_{peripheral}\n")
        f.write("\n")
        
        # RX Data Struct
        f.write("typedef struct {\n")
        rx_msgs = [] # List of (Message, peripheral, bus_name)
        for bus_name in sorted(node.busses.keys()):
            bus = node.busses[bus_name]
            for rx_msg in bus.rx_messages:
                if rx_msg.name in all_messages:
                    f.write(f"\t{rx_msg.name}_data_t {rx_msg.name};\n")
                    rx_msgs.append((all_messages[rx_msg.name], bus.peripheral, bus_name))
        f.write("} can_data_t;\n")
        f.write("extern can_data_t can_data;\n\n")
        
        # RX Functions
        for msg, peripheral, bus_name in rx_msgs:
            generate_rx_func(f, msg, custom_types)
            
        # TX Functions
        for bus_name in sorted(node.busses.keys()):
            bus = node.busses[bus_name]
            for msg in bus.tx_messages:
                generate_tx_func(f, msg, bus.peripheral, bus_name, bus_configs, custom_types)
        
        # Filter Functions
        generate_filter_funcs(f, node, rx_msgs, bus_configs)
                
        f.write("#endif\n")
    print_as_success(f"Generated {filename.name}")

def generate_filter_funcs(f, node: Node, rx_msgs: List[Tuple[Message, str, str]], bus_configs: Dict):
    peripherals = sorted(list(set(bus.peripheral for bus in node.busses.values())))
    
    # Group RX messages by peripheral
    periph_to_msgs = {p: [] for p in peripherals}
    for msg, periph, bus_name in rx_msgs:
        periph_to_msgs[periph].append((msg, bus_name))
        
    for periph in peripherals:
        msgs = periph_to_msgs[periph]
        f.write(f"static inline void {periph}_set_filters() {{\n")
        
        if not msgs:
            f.write("\treturn;\n")
            f.write("}\n\n")
            continue
            
        f.write(f"\tCAN1->FMR  |= CAN_FMR_FINIT;\n")
        
        # bxCAN filter bank assignment
        # CAN1: 0-13, CAN2: 14-27
        bank_offset = 0 if periph == "CAN1" else 14
        
        for i in range(0, len(msgs), 2):
            bank_idx = bank_offset + (i // 2)
            msg1, bus1 = msgs[i]
            msg2, bus2 = msgs[i+1] if i+1 < len(msgs) else (None, None)
            
            f.write(f"\t// Bank {bank_idx}: {msg1.name}" + (f", {msg2.name}" if msg2 else "") + "\n")
            f.write(f"\tCAN1->FM1R |= (1 << {bank_idx});\n")
            f.write(f"\tCAN1->FS1R |= (1 << {bank_idx});\n")
            
            m1_macro = f"{to_macro_name(msg1.name)}_MSG_ID"
            is_ext1 = msg1.is_extended or bus_configs.get(bus1, {}).get('is_extended_id', False)
            
            if is_ext1:
                f.write(f"\tCAN1->sFilterRegister[{bank_idx}].FR1 = ({m1_macro} << 3) | 4;\n")
            else:
                f.write(f"\tCAN1->sFilterRegister[{bank_idx}].FR1 = ({m1_macro} << 21);\n")
            
            if msg2:
                m2_macro = f"{to_macro_name(msg2.name)}_MSG_ID"
                is_ext2 = msg2.is_extended or bus_configs.get(bus2, {}).get('is_extended_id', False)
                if is_ext2:
                    f.write(f"\tCAN1->sFilterRegister[{bank_idx}].FR2 = ({m2_macro} << 3) | 4;\n")
                else:
                    f.write(f"\tCAN1->sFilterRegister[{bank_idx}].FR2 = ({m2_macro} << 21);\n")
            else:
                if is_ext1:
                    f.write(f"\tCAN1->sFilterRegister[{bank_idx}].FR2 = ({m1_macro} << 3) | 4;\n")
                else:
                    f.write(f"\tCAN1->sFilterRegister[{bank_idx}].FR2 = ({m1_macro} << 21);\n")
                
            f.write(f"\tCAN1->FA1R |= (1 << {bank_idx});\n")
            
        f.write(f"\tCAN1->FMR &= ~CAN_FMR_FINIT;\n")
        f.write(f"}}\n\n")

def generate_rx_func(f, msg: Message, custom_types: Dict):
    f.write(f"[[gnu::always_inline]]\n")
    f.write(f"static inline void CAN_RECEIVE_{msg.name}(const uint64_t data_raw) {{\n")
    f.write(f"\tuint64_t data = __builtin_bswap64(data_raw);\n\n")
    
    bit_offset = 0
    for sig in msg.signals:
        length = sig.get_bit_length(custom_types)
        mask = (1 << length) - 1
        shift = 64 - bit_offset - length
        f.write(f"\tcan_data.{msg.name}.{sig.name} = ({sig.datatype})((data >> {shift}) & {hex(mask)});\n")
        bit_offset += length
        
    f.write(f"\n\tcan_data.{msg.name}.last_rx = 0; // TODO: Get current timestamp\n")
    f.write(f"\tcan_data.{msg.name}.stale = false;\n")
    f.write(f"}}\n\n")

def generate_tx_func(f, msg: Message, peripheral: str, bus_name: str, bus_configs: Dict, custom_types: Dict):
    msg_macro_base = to_macro_name(msg.name)
    
    is_ext = msg.is_extended or bus_configs.get(bus_name, {}).get('is_extended_id', False)
    
    f.write(f"[[gnu::always_inline]]\n")
    if not msg.signals:
        f.write(f"static inline void CAN_SEND_{msg.name}(void) {{\n")
    else:
        f.write(f"static inline void CAN_SEND_{msg.name}(\n")
        for i, sig in enumerate(msg.signals):
            comma = "," if i < len(msg.signals) - 1 else ""
            f.write(f"\t{sig.datatype} {sig.name}{comma}\n")
        f.write(") {\n")
    
    f.write(f"\tCanMsgTypeDef_t outgoing = {{\n")
    f.write(f"\t\t.Bus={peripheral},\n")
    f.write(f"\t\t.ExtId={msg_macro_base}_MSG_ID,\n")
    f.write(f"\t\t.DLC={msg_macro_base}_DLC,\n")
    f.write(f"\t\t.IDE={1 if is_ext else 0}\n")
    f.write(f"\t}};\n\n")
    f.write(f"\tuint64_t data = 0;\n")
    
    bit_offset = 0
    for sig in msg.signals:
        length = sig.get_bit_length(custom_types)
        mask = (1 << length) - 1
        shift = 64 - bit_offset - length
        f.write(f"\tdata |= ((uint64_t)({sig.name} & {hex(mask)}) << {shift});\n")
        bit_offset += length
        
    f.write(f"\n\tuint64_t wire_data = __builtin_bswap64(data);\n")
    f.write(f"\tmemcpy(outgoing.Data, &wire_data, {msg_macro_base}_DLC);\n\n")
    f.write(f"\tCAN_enqueue_tx(&outgoing);\n")
    f.write(f"}}\n\n")

def generate_bus_header(bus_name: str, config: Dict, messages: List[Message], custom_types: Dict):
    filename = GENERATED_DIR / f"{bus_name}.h"
    
    with open(filename, 'w') as f:
        f.write(f"#ifndef {bus_name}_H\n")
        f.write(f"#define {bus_name}_H\n\n")
        
        f.write("#include <stdint.h>\n\n")
        
        # Bus Properties
        f.write("// Bus Properties\n")
        f.write(f"#define {bus_name}_BAUD_RATE ({config.get('baud_rate', 500000)})\n\n")
        
        # Messages
        f.write("// Messages\n")
        # Sort messages by ID for cleaner output
        messages.sort(key=lambda m: m.final_id)
        
        for msg in messages:
            msg_macro_base = to_macro_name(msg.name)
            f.write(f"#define {msg_macro_base}_MSG_ID (0x{msg.final_id:03X})\n")
            f.write(f"#define {msg_macro_base}_DLC ({msg.get_dlc(custom_types)})\n")
            
            f.write(f"typedef struct {{\n")
            for sig in msg.signals:
                comment = ""
                if sig.length > 0:
                     comment = f" // {sig.length} bits"
                f.write(f"\t{sig.datatype} {sig.name};{comment}\n")
            
            f.write("\n")
            f.write(f"\tuint32_t last_rx;\n")
            f.write(f"\tbool stale;\n")
            f.write(f"}} {msg.name}_data_t;\n\n")
            
        f.write("#endif\n")
        
    print_as_success(f"Generated {filename.name}")

