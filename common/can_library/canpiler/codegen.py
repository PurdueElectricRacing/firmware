from typing import List, Dict, Tuple, Optional
from parser import Node, Message, RxMessage, SystemContext
from mapper import NodeMapping
from utils import GENERATED_DIR, print_as_success, print_as_ok, to_c_enum_prefix, get_git_hash

def generate_headers(context: SystemContext):
    print("Generating headers...")
    
    # Ensure generated directory exists
    GENERATED_DIR.mkdir(exist_ok=True)

    # Generate types header
    generate_types_header(context.custom_types)

    # Generate header for each bus
    for bus_name, view in context.busses.items():
        config = context.bus_configs.get(bus_name, {})
        generate_bus_header(bus_name, config, view.messages, context.custom_types)

    # Generate header for each node
    generate_node_headers(context)

    # Generate router header
    generate_router_header(context.nodes)
    
    print_as_success("Successfully generated C headers")

def format_float(val: float) -> str:
    """Format float to .6g and ensure it looks like a float literal in C"""
    s = f"{val:.6g}"
    if '.' not in s and 'e' not in s:
        s = f"{val:.1f}"
    return s + "f"

def generate_types_header(custom_types: Dict):
    filename = GENERATED_DIR / "can_types.h"
    with open(filename, 'w') as f:
        f.write("#ifndef CAN_TYPES_H\n")
        f.write("#define CAN_TYPES_H\n\n")
        f.write("#include <stdint.h>\n\n")
        
        for type_name, config in custom_types.items():
            prefix = to_c_enum_prefix(type_name)
            
            f.write(f"typedef enum {{\n")
            for i, choice in enumerate(config.get('choices', [])):
                f.write(f"\t{prefix}_{choice.upper()} = {i},\n")
            f.write(f"}} {type_name};\n\n")
            
        f.write("#endif\n")
    print_as_ok(f"Generated {filename.name}")

def generate_router_header(nodes: List[Node]):
    filename = GENERATED_DIR / "can_router.h"
    
    with open(filename, 'w') as f:
        f.write("#ifndef CAN_ROUTER_H\n")
        f.write("#define CAN_ROUTER_H\n\n")
        
        internal_nodes = sorted([n for n in nodes if not n.is_external], key=lambda x: x.name)
        
        if internal_nodes:
            for i, node in enumerate(internal_nodes):
                directive = "#if" if i == 0 else "#elif"
                f.write(f"{directive} defined(CAN_NODE_{node.macro_name})\n")
                f.write(f'\t#include "{node.name}.h"\n')
            f.write("#endif\n")
            
        f.write("\n#endif\n")
    print_as_ok(f"Generated {filename.name}")

def generate_node_headers(context: SystemContext):
    for node in context.nodes:
        if node.is_external:
            continue
        generate_node_header(node, context.bus_configs, context.custom_types, context.mappings.get(node.name))

def generate_node_header(node: Node, bus_configs: Dict, custom_types: Dict, mapping: Optional[NodeMapping]):
    filename = GENERATED_DIR / f"{node.name}.h"
    
    with open(filename, 'w') as f:
        f.write(f"#ifndef {node.name}_H\n")
        f.write(f"#define {node.name}_H\n\n")
        
        # Macros
        peripherals = sorted(list(set(bus.peripheral for bus in node.busses.values())))
        f.write(f"#define NUM_CAN_PERIPHERALS {len(peripherals)}\n")
        for i, peripheral in enumerate(peripherals):
            f.write(f"#define USE_{peripheral}\n")
            f.write(f"#define {peripheral}_IDX {i}\n")
        f.write("\n")

        # Includes
        for bus_name in sorted(node.busses.keys()):
            f.write(f'#include "{bus_name}.h"\n')
        f.write('\n#include <string.h>\n')
        
        if node.scheduler == "freertos":
            f.write('#include "common/freertos/freertos.h"\n')
            f.write('#define OS_TICKS (xTaskGetTickCount())\n')
        else:
            f.write('#include "common/psched/psched.h"\n')
            f.write('#define OS_TICKS (sched.os_ticks)\n')

        f.write('#include "common/can_library/can_common.h"\n')
        f.write('#include "fault_data.h"\n\n')

        # Git Hash
        f.write(f"#define GIT_HASH \"{get_git_hash()}\"\n\n")
        
        # System IDs
        if node.system_ids:
            f.write("// System IDs\n")
            for name, val in sorted(node.system_ids.items()):
                # Determine primary macro name
                if name.startswith("daq_response"):
                    bus_name = name.replace("daq_response_", "").upper()
                    macro_name = f"ID_DAQ_RESPONSE_{node.macro_name}_{bus_name}"
                elif name == "fault_sync":
                    macro_name = f"ID_FAULT_SYNC_{node.macro_name}"
                else:
                    macro_name = f"ID_{name.upper()}_{node.macro_name}"
                
                f.write(f"#define {macro_name} (0x{val:x})\n")
                
                # MAIN_MODULE alias for backward compatibility
                if node.macro_name == "MAIN":
                    alias_name = macro_name.replace("MAIN", "MAIN_MODULE")
                    f.write(f"#define {alias_name} {macro_name}\n")
            f.write("\n")

        # Scaling Macros
        f.write("// Scaling Macros\n")
        macros_written = False
        for bus_name, bus in sorted(node.busses.items()):
            # TX Messages -> PACK_COEFF
            for msg in bus.tx_messages:
                for sig in msg.signals:
                    if sig.scale != 1.0:
                        s_name = f"PACK_COEFF_{msg.macro_name}_{sig.macro_name}"
                        f.write(f"#define {s_name} ({format_float(1.0/sig.scale)})\n")
                        macros_written = True
            
            # RX Messages -> UNPACK_COEFF
            for rx_msg in bus.rx_messages:
                if rx_msg.resolved_message:
                    msg = rx_msg.resolved_message
                    for sig in msg.signals:
                        if sig.scale != 1.0:
                            s_name = f"UNPACK_COEFF_{msg.macro_name}_{sig.macro_name}"
                            f.write(f"#define {s_name} ({format_float(sig.scale)})\n")
                            macros_written = True
        if macros_written:
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
        
        # Callback Declarations
        for rx_msg, _, _ in rx_msgs:
            if rx_msg.callback:
                f.write(f"extern void {rx_msg.name}_CALLBACK(can_data_t* can_data);\n")
        f.write("\n")

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
        if mapping:
            generate_filter_funcs(f, mapping)
                
        f.write("#endif\n")
    print_as_ok(f"Generated {filename.name}")

def generate_filter_funcs(f, mapping: NodeMapping):
    for periph, banks in mapping.filters.items():
        f.write(f"static inline void {periph}_set_filters() {{\n")
        
        accept_all = mapping.accept_all.get(periph, False)
        
        if not banks and not accept_all:
            f.write("\treturn;\n")
            f.write("}\n\n")
            continue
            
        f.write(f"\t{periph}->FMR  |= CAN_FMR_FINIT;\n")
        
        if accept_all:
            f.write("\t// Accept all messages\n")
            bank_idx = 0 if periph == "CAN1" else 14
            f.write(f"\t{periph}->FM1R &= ~(1 << {bank_idx}); // Mask mode\n")
            f.write(f"\t{periph}->FS1R |= (1 << {bank_idx});  // 32-bit scale\n")
            f.write(f"\t{periph}->sFilterRegister[{bank_idx}].FR1 = 0;\n")
            f.write(f"\t{periph}->sFilterRegister[{bank_idx}].FR2 = 0;\n")
            f.write(f"\t{periph}->FA1R |= (1 << {bank_idx});\n")
        else:
            for fb in banks:
                f.write(f"\t// Bank {fb.bank_idx}: {fb.msg1.name}" + (f", {fb.msg2.name}" if fb.msg2 else "") + "\n")
                f.write(f"\t{periph}->FM1R |= (1 << {fb.bank_idx});\n")
                f.write(f"\t{periph}->FS1R |= (1 << {fb.bank_idx});\n")
                
                m1_macro = f"{fb.msg1.macro_name}_MSG_ID"
                if fb.is_ext1:
                    f.write(f"\t{periph}->sFilterRegister[{fb.bank_idx}].FR1 = ({m1_macro} << 3) | 4;\n")
                else:
                    f.write(f"\t{periph}->sFilterRegister[{fb.bank_idx}].FR1 = ({m1_macro} << 21);\n")
                
                if fb.msg2:
                    m2_macro = f"{fb.msg2.macro_name}_MSG_ID"
                    if fb.is_ext2:
                        f.write(f"\t{periph}->sFilterRegister[{fb.bank_idx}].FR2 = ({m2_macro} << 3) | 4;\n")
                    else:
                        f.write(f"\t{periph}->sFilterRegister[{fb.bank_idx}].FR2 = ({m2_macro} << 21);\n")
                else:
                    # If no second message, repeat the first one to fill the bank
                    if fb.is_ext1:
                        f.write(f"\t{periph}->sFilterRegister[{fb.bank_idx}].FR2 = ({m1_macro} << 3) | 4;\n")
                    else:
                        f.write(f"\t{periph}->sFilterRegister[{fb.bank_idx}].FR2 = ({m1_macro} << 21);\n")
                
                f.write(f"\t{periph}->FA1R |= (1 << {fb.bank_idx});\n")
        
        f.write(f"\t{periph}->FMR &= ~CAN_FMR_FINIT;\n")
        f.write("}\n\n")

def generate_rx_dispatcher(f, node: Node, rx_msgs: List[Tuple[RxMessage, str, str]], custom_types: Dict):
    f.write("static inline void CAN_rx_dispatcher(uint32_t id, uint8_t* data, uint8_t len, uint8_t peripheral_idx) {\n")
    f.write("\tswitch(id) {\n")
    
    for rx_msg, periph, bus_name in rx_msgs:
        msg = rx_msg.resolved_message
        f.write(f"\t\tcase {msg.macro_name}_MSG_ID:\n")
        f.write(f"\t\t\tif (peripheral_idx == {periph}_IDX) {{\n")
        
        f.write(f"\t\t\t\tuint64_t wire_data = 0;\n")
        f.write(f"\t\t\t\tmemcpy(&wire_data, data, len);\n")
        f.write(f"\t\t\t\tuint64_t host_data = __builtin_bswap64(wire_data);\n")
        
        for sig in msg.signals:
            if sig.is_signed and sig.length < 64:
                # Sign extension using arithmetic shift
                f.write(f"\t\t\t\tcan_data.{msg.name}.{sig.name} = ({sig.c_type})(((int64_t)((host_data >> {sig.bit_shift}) & {hex(sig.mask)}) << {64 - sig.length}) >> {64 - sig.length});\n")
            else:
                f.write(f"\t\t\t\tcan_data.{msg.name}.{sig.name} = ({sig.c_type})((host_data >> {sig.bit_shift}) & {hex(sig.mask)});\n")
            
        f.write(f"\t\t\t\tcan_data.{msg.name}.last_rx = OS_TICKS;\n")
        f.write(f"\t\t\t\tcan_data.{msg.name}.stale = false;\n")
        if rx_msg.callback:
            f.write(f"\t\t\t\t{rx_msg.name}_CALLBACK(&can_data);\n")
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
            f.write(f"\tif (OS_TICKS - can_data.{msg.name}.last_rx > {threshold}) {{\n")
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
    is_ext = msg.is_extended_frame
    
    f.write(f"[[gnu::always_inline]]\n")
    if not msg.signals:
        f.write(f"static inline void CAN_SEND_{msg.name}(void) {{\n")
    else:
        f.write(f"static inline void CAN_SEND_{msg.name}(\n")
        for i, sig in enumerate(msg.signals):
            comma = "," if i < len(msg.signals) - 1 else ""
            f.write(f"\t{sig.c_type} {sig.name}{comma}\n")
        f.write(") {\n")
    
    f.write(f"\tCanMsgTypeDef_t outgoing = {{\n")
    f.write(f"\t\t.Bus={peripheral},\n")
    f.write(f"\t\t.ExtId={msg.macro_name}_MSG_ID,\n")
    f.write(f"\t\t.DLC={msg.macro_name}_DLC,\n")
    f.write(f"\t\t.IDE={1 if is_ext else 0}\n")
    f.write(f"\t}};\n\n")
    f.write(f"\tuint64_t data = 0;\n")
    
    for sig in msg.signals:
        if sig.is_floating_point:
            union_type = "uint32_t" if sig.c_type == 'float' else "uint64_t"
            f.write(f"\tunion {{ {sig.c_type} f; {union_type} u; }} _{sig.name}_u;\n")
            f.write(f"\t_{sig.name}_u.f = {sig.name};\n")
            f.write(f"\tdata |= ((uint64_t)(_{sig.name}_u.u & {hex(sig.mask)}) << {sig.bit_shift});\n")
        else:
            f.write(f"\tdata |= ((uint64_t)({sig.name} & {hex(sig.mask)}) << {sig.bit_shift});\n")
        
    f.write(f"\n\tuint64_t wire_data = __builtin_bswap64(data);\n")
    f.write(f"\tmemcpy(outgoing.Data, &wire_data, {msg.macro_name}_DLC);\n\n")
    f.write(f"\tCAN_enqueue_tx(&outgoing);\n")
    f.write(f"}}\n\n")

def generate_bus_header(bus_name: str, config: Dict, messages: List[Message], custom_types: Dict):
    filename = GENERATED_DIR / f"{bus_name}.h"
    
    with open(filename, 'w') as f:
        f.write(f"#ifndef {bus_name}_H\n")
        f.write(f"#define {bus_name}_H\n\n")
        
        f.write("#include <stdint.h>\n")
        f.write("#include \"can_types.h\"\n\n")
        
        # Bus Properties
        f.write("// Bus Properties\n")
        f.write(f"#define {bus_name}_BAUD_RATE ({config.get('baud_rate', 500000)})\n\n")
        
        # Messages
        f.write("// Messages\n")
        # Sort messages by ID for cleaner output
        messages.sort(key=lambda m: m.final_id)
        
        for msg in messages:
            f.write(f"#define {msg.macro_name}_MSG_ID (0x{msg.final_id:03X})\n")
            f.write(f"#define {msg.macro_name}_DLC ({msg.get_dlc(custom_types)})\n")
            if msg.period > 0:
                f.write(f"#define {msg.macro_name}_PERIOD ({msg.period})\n")
                f.write(f"#define PERIOD_{msg.macro_name}_MS ({msg.period})\n")
            
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
        
    print_as_ok(f"Generated {filename.name}")

