from typing import List, Dict, Tuple
from parser import Node, Message, RxMessage, load_custom_types
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
    for node in nodes:
        if node.is_external:
            continue
        generate_node_header(node, bus_configs, custom_types)

def generate_node_header(node: Node, bus_configs: Dict, custom_types: Dict):
    filename = GENERATED_DIR / f"{node.name}.h"
    
    with open(filename, 'w') as f:
        f.write(f"#ifndef {node.name}_H\n")
        f.write(f"#define {node.name}_H\n\n")
        
        # Includes
        for bus_name in sorted(node.busses.keys()):
            f.write(f'#include "{bus_name}.h"\n')
        f.write('\n#include <string.h>\n')
        f.write('#include "common/psched/psched.h"\n')
        f.write('#include "common/can_library/can_common.h"\n\n')
        
        # Macros
        peripherals = sorted(list(set(bus.peripheral for bus in node.busses.values())))
        f.write(f"#define NUM_CAN_PERIPHERALS {len(peripherals)}\n")
        for i, peripheral in enumerate(peripherals):
            f.write(f"#define USE_{peripheral}\n")
            f.write(f"#define {peripheral}_IDX {i}\n")
        f.write("\n")
        
        # RX Data Struct
        f.write("typedef struct {\n")
        rx_msgs = [] # List of (RxMessage, peripheral, bus_name)
        for bus_name in sorted(node.busses.keys()):
            bus = node.busses[bus_name]
            for rx_msg in bus.rx_messages:
                if rx_msg.resolved_message:
                    f.write(f"\t{rx_msg.name}_data_t {rx_msg.name};\n")
                    rx_msgs.append((rx_msg, bus.peripheral, bus_name))
        f.write("} can_data_t;\n")
        f.write("extern can_data_t can_data;\n\n")
        
        # RX Functions
        generate_rx_dispatcher(f, node, rx_msgs, custom_types)
        generate_stale_check(f, node, rx_msgs)
        generate_data_init(f, node, rx_msgs)
        
        # TX Functions
        for bus_name in sorted(node.busses.keys()):
            bus = node.busses[bus_name]
            for msg in bus.tx_messages:
                generate_tx_func(f, msg, bus.peripheral, bus_name, bus_configs, custom_types)
        
        # Filter Functions
        generate_filter_funcs(f, node, rx_msgs, bus_configs)
                
        f.write("#endif\n")
    print_as_success(f"Generated {filename.name}")

def generate_filter_funcs(f, node: Node, rx_msgs: List[Tuple[RxMessage, str, str]], bus_configs: Dict):
    peripherals = sorted(list(set(bus.peripheral for bus in node.busses.values())))
    
    # Group RX messages by peripheral
    periph_to_msgs = {p: [] for p in peripherals}
    for rx_msg, periph, bus_name in rx_msgs:
        periph_to_msgs[periph].append((rx_msg, bus_name))
        
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
            rx_msg1, bus1 = msgs[i]
            msg1 = rx_msg1.resolved_message
            rx_msg2, bus2 = msgs[i+1] if i+1 < len(msgs) else (None, None)
            msg2 = rx_msg2.resolved_message if rx_msg2 else None
            
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
                # If no second message, repeat the first one to fill the bank
                if is_ext1:
                    f.write(f"\tCAN1->sFilterRegister[{bank_idx}].FR2 = ({m1_macro} << 3) | 4;\n")
                else:
                    f.write(f"\tCAN1->sFilterRegister[{bank_idx}].FR2 = ({m1_macro} << 21);\n")
            
            f.write(f"\tCAN1->FA1R |= (1 << {bank_idx});\n")
        
        f.write(f"\tCAN1->FMR &= ~CAN_FMR_FINIT;\n")
        f.write("}\n\n")

def generate_rx_dispatcher(f, node: Node, rx_msgs: List[Tuple[RxMessage, str, str]], custom_types: Dict):
    f.write("static inline void CAN_rx_dispatcher(uint32_t id, uint8_t* data, uint8_t len, uint8_t peripheral_idx) {\n")
    f.write("\tswitch(id) {\n")
    
    for rx_msg, periph, bus_name in rx_msgs:
        msg = rx_msg.resolved_message
        msg_macro_base = to_macro_name(msg.name)
        f.write(f"\t\tcase {msg_macro_base}_MSG_ID:\n")
        f.write(f"\t\t\tif (peripheral_idx == {periph}_IDX) {{\n")
        
        f.write(f"\t\t\t\tuint64_t wire_data = 0;\n")
        f.write(f"\t\t\t\tmemcpy(&wire_data, data, len);\n")
        f.write(f"\t\t\t\tuint64_t host_data = __builtin_bswap64(wire_data);\n")
        
        bit_offset = 0
        for sig in msg.signals:
            length = sig.get_bit_length(custom_types)
            mask = (1 << length) - 1
            shift = 64 - bit_offset - length
            
            # Determine if we need sign extension
            base_type = sig.datatype
            if custom_types and sig.datatype in custom_types:
                base_type = custom_types[sig.datatype]['base_type']
            
            is_signed = base_type.startswith('int')
            
            if is_signed and length < 64:
                # Sign extension using arithmetic shift
                f.write(f"\t\t\t\tcan_data.{msg.name}.{sig.name} = ({sig.datatype})(((int64_t)((host_data >> {shift}) & {hex(mask)}) << {64 - length}) >> {64 - length});\n")
            else:
                f.write(f"\t\t\t\tcan_data.{msg.name}.{sig.name} = ({sig.datatype})((host_data >> {shift}) & {hex(mask)});\n")
            
            bit_offset += length
            
        f.write(f"\t\t\t\tcan_data.{msg.name}.last_rx = sched.os_ticks;\n")
        f.write(f"\t\t\t\tcan_data.{msg.name}.stale = false;\n")
        f.write("\t\t\t}\n")
        f.write("\t\t\tbreak;\n")
        
    f.write("\t\tdefault:\n")
    f.write("\t\t\tbreak;\n")
    f.write("\t}\n")
    f.write("}\n\n")

def generate_stale_check(f, node: Node, rx_msgs: List[Tuple[RxMessage, str, str]]):
    f.write("static inline void CAN_check_stale() {\n")
    for rx_msg, periph, bus_name in rx_msgs:
        msg = rx_msg.resolved_message
        if msg.period > 0:
            # Use 2.5x period as threshold
            threshold = int(msg.period * 2.5)
            f.write(f"\tif (sched.os_ticks - can_data.{msg.name}.last_rx > {threshold}) {{\n")
            f.write(f"\t\tcan_data.{msg.name}.stale = true;\n")
            f.write(f"\t}}\n")
    f.write("}\n\n")

def generate_data_init(f, node: Node, rx_msgs: List[Tuple[RxMessage, str, str]]):
    f.write("static inline void CAN_data_init() {\n")
    f.write("\tmemset(&can_data, 0, sizeof(can_data));\n")
    for rx_msg, periph, bus_name in rx_msgs:
        msg = rx_msg.resolved_message
        if msg.period > 0:
            f.write(f"\tcan_data.{msg.name}.stale = true;\n")
    f.write("}\n\n")

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

