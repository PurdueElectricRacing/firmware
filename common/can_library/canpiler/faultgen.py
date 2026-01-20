"""
faultgen.py

Author: Irving Wang (irvingw@purdue.edu)
"""

import sys
from typing import List, Dict
from parser import Node, Message, Signal, RxMessage, FaultModule, Fault, SystemContext
from utils import (GENERATED_DIR, print_as_success, print_as_ok, 
                   print_as_error, load_json, NODE_CONFIG_DIR)

def validate_fault_configs(fault_modules: List[FaultModule]):
    """Semantic validation for faults."""
    print("Validating fault configurations...")
    fault_names = set()
    
    for module in fault_modules:
        if len(module.faults) > 64:
            print_as_error(f"Node '{module.node_name}' has {len(module.faults)} faults, exceeding the 64-bit bitfield limit.")
            raise ValueError("Too many faults for bitfield sync")
        
        for fault in module.faults:
            # Enforce global fault name uniqueness
            name_upper = fault.name.upper()
            if name_upper in fault_names:
                print_as_error(f"Duplicate fault name detected across system: {name_upper}")
                raise ValueError(f"Global fault name collision: {name_upper}")
            fault_names.add(name_upper)
            
            if fault.min_val >= fault.max_val:
                print_as_error(f"Fault '{fault.name}' in node '{module.node_name}' has invalid limits: min({fault.min_val}) >= max({fault.max_val})")
                raise ValueError("Invalid fault limits")
    
    print_as_success("Fault configurations validated")

def validate_fault_injection(nodes: List[Node], fault_modules: List[FaultModule], bus_configs: Dict) -> str:
    """
    Check for potential issues that would prevent successful injection.
    Runs BEFORE inject_fault_messages to ensure 'clean' state.
    """
    print("Pre-flight validation for fault injection...")

    # 1. Check for host bus library
    fault_bus_name = None
    for b_conf in bus_configs.get('busses', []):
        if b_conf.get('host_fault_library', False):
            fault_bus_name = b_conf['name']
            break
            
    if not fault_bus_name:
        print_as_error("No bus configured with 'host_fault_library': true. This is required for decentralised faults.")
        raise ValueError("Missing host_fault_library configuration")

    node_map = {n.name.upper(): n for n in nodes if not n.is_external}

    for module in fault_modules:
        m_name_upper = module.node_name.upper()
        if m_name_upper not in node_map:
            continue
            
        node = node_map[m_name_upper]
        
        # 2. Check if node is on the fault bus
        if fault_bus_name not in node.busses:
            print_as_error(f"Node '{node.name}' has faults but no {fault_bus_name} bus. This bus is required for fault communication.")
            raise ValueError(f"Node {node.name} missing fault bus {fault_bus_name}")

        # 3. Check for message shadowing
        bus = node.busses[fault_bus_name]
        existing_msg_names = {m.name.lower() for m in bus.tx_messages}
        
        for suffix in ["_fault_event", "_fault_sync"]:
            msg_name = f"{node.name.lower()}{suffix}"
            if msg_name in existing_msg_names:
                print_as_error(f"Conflict: Node {node.name} already has a message named {msg_name}. Shadowing injected faults is forbidden.")
                raise ValueError(f"Message name shadowing: {msg_name}")
    
    return fault_bus_name

def inject_fault_messages(nodes: List[Node], fault_modules: List[FaultModule], bus_configs: Dict, fault_bus_name: str):
    """Inject TX and RX messages for faults into Node objects."""
    
    print("Injecting fault communication messages into pipeline...")
    node_map = {n.name.upper(): n for n in nodes if not n.is_external}
    
    all_fault_sync_msgs = []
    all_fault_event_msgs = []
    
    # 1. Inject TX Messages
    for module in fault_modules:
        m_name_upper = module.node_name.upper()
        if m_name_upper not in node_map:
            continue
            
        node = node_map[m_name_upper]
        bus = node.busses[fault_bus_name]
        
        # Fault Event (Priority 0) - Fired on state change
        event_name = f"{node.name.lower()}_fault_event"
        event_msg = Message(
            name=event_name,
            desc=f"Immediate fault event signal for {node.name}",
            priority=0,
            signals=[
                Signal(name="idx", datatype="fault_index_t", desc="Global Fault Index"),
                Signal(name="state", datatype="bool", desc="Latch State (0=unlatched, 1=latched)"),
                Signal(name="val", datatype="uint16_t", desc="Trigger Value")
            ]
        )
        bus.tx_messages.append(event_msg)
        all_fault_event_msgs.append(event_name)
        
        # Fault Sync (Priority 1) - Periodic bitfield
        sync_name = f"{node.name.lower()}_fault_sync"
        sync_msg = Message(
            name=sync_name,
            desc=f"Periodic fault synchronization for {node.name}",
            priority=1,
            period=100,
            signals=[
                Signal(name=f.name, datatype="bool", length=1) for f in module.faults
            ]
        )
        bus.tx_messages.append(sync_msg)
        all_fault_sync_msgs.append(sync_name)
        
    # 2. Global Subscription
    # Every internal node subscribes to every other node's sync and event messages
    all_msgs = all_fault_event_msgs + all_fault_sync_msgs
    
    for node in nodes:
        if node.is_external or fault_bus_name not in node.busses:
            continue
            
        bus = node.busses[fault_bus_name]
        tx_names = {m.name for m in bus.tx_messages}
        rx_names = {m.name for m in bus.rx_messages}
        
        for msg_name in all_msgs:
            if msg_name not in tx_names and msg_name not in rx_names:
                bus.rx_messages.append(RxMessage(name=msg_name, callback=True))
                
    print_as_success(f"Injected {len(all_fault_event_msgs)} events and {len(all_fault_sync_msgs)} sync messages on {fault_bus_name}")

def inject_fault_types(custom_types: Dict, fault_modules: List[FaultModule]):
    """Inject fault_index_t enum into the common types list."""
    choices = []
    global_idx = 0
    for module in fault_modules:
        for fault in module.faults:
            # The prefix will be FAULT_INDEX_
            choices.append(f"{module.node_name}_{fault.name}")
            fault.absolute_index = global_idx
            global_idx += 1
            
    # Preserve base_type if it exists in the original definition from common_types.json
    base_type = custom_types.get("fault_index_t", {}).get("base_type", "uint16_t")

    custom_types["fault_index_t"] = {
        "name": "fault_index_t",
        "choices": choices,
        "base_type": base_type
    }
    print_as_ok(f"Injected fault_index_t with {global_idx} total faults")

def augment_system_with_faults(nodes: List[Node], bus_configs: Dict, custom_types: Dict):
    """
    Middleware: Enriches Nodes with fault communication protocols and 
    registers fault types in the global type system. (Change B)
    """
    fault_modules = [
        FaultModule(n.name, n.generate_fault_strings, n.faults, n.node_id) 
        for n in nodes if n.faults
    ]
    
    if not fault_modules:
        return

    validate_fault_configs(fault_modules)
    fault_bus_name = validate_fault_injection(nodes, fault_modules, bus_configs)
    inject_fault_messages(nodes, fault_modules, bus_configs, fault_bus_name)
    inject_fault_types(custom_types, fault_modules)

def generate_fault_data(context: SystemContext):
    """Entry point for implementation generation. Consumed by build.py."""
    print("Generating fault library implementation data...")
    
    generate_fault_header(context)
    generate_fault_source(context)
    
    print_as_success("Fault library implementation files generated")

def generate_fault_header(context: SystemContext):
    filename = GENERATED_DIR / "fault_data.h"
    total_faults = sum(len(m.faults) for m in context.fault_modules)
    
    with open(filename, 'w') as f:
        f.write("#ifndef FAULT_DATA_H\n")
        f.write("#define FAULT_DATA_H\n\n")
        f.write("#include <stdint.h>\n")
        f.write("#include \"common/can_library/generated/can_types.h\"\n\n")
        
        # Determine if current node should have strings
        f.write("// Fault string configuration\n")
        string_nodes = [m.node_name.upper() for m in context.fault_modules if m.generate_strings]
        if string_nodes:
            f.write("#if " + " || ".join([f"defined(CAN_NODE_{name})" for name in string_nodes]) + "\n")
            f.write("#define HAS_FAULT_STRINGS\n")
            f.write("#endif\n")

        f.write("#include \"common/faults/faults_common.h\"\n\n")
        f.write(f"#define TOTAL_NUM_FAULTS {total_faults}\n")
        
        f.write("\n// Global fault state and attribute arrays\n")
        f.write("extern fault_status_t faults[TOTAL_NUM_FAULTS];\n")
        f.write("extern const fault_attributes_t fault_attributes[TOTAL_NUM_FAULTS];\n\n")
        
        f.write("#ifdef HAS_FAULT_STRINGS\n")
        f.write("extern const char* const fault_strings[TOTAL_NUM_FAULTS];\n")
        f.write("#endif\n\n")

        f.write("// Node limits for current build\n")
        for module in context.fault_modules:
            f_start = f"FAULT_INDEX_{module.node_name.upper()}_{module.faults[0].name.upper()}"
            f_end = f"FAULT_INDEX_{module.node_name.upper()}_{module.faults[-1].name.upper()}"
            f.write(f"#ifdef CAN_NODE_{module.node_name.upper()}\n")
            f.write(f"#define MY_FAULT_START {f_start}\n")
            f.write(f"#define MY_FAULT_END {f_end}\n")
            f.write("#endif\n")
        f.write("\n")

        f.write("// Transmission Helpers\n")
        f.write("void tx_fault_sync(void);\n")
        f.write("void tx_fault_event(fault_index_t idx, uint16_t value);\n\n")
        
        f.write("#endif\n")
    print_as_ok(f"Generated {filename.name}")

def generate_fault_source(context: SystemContext):
    filename = GENERATED_DIR / "fault_data.c"
    
    with open(filename, 'w') as f:
        f.write("#include \"common/can_library/generated/fault_data.h\"\n")
        f.write("#include \"common/can_library/generated/can_router.h\"\n\n")
        
        f.write(f"fault_status_t faults[TOTAL_NUM_FAULTS] = {{0}};\n\n")
        
        f.write(f"const fault_attributes_t fault_attributes[TOTAL_NUM_FAULTS] = {{\n")
        for module in context.fault_modules:
            f.write(f"\t// --- {module.node_name} ---\n")
            for fault in module.faults:
                enum_val = f"FAULT_INDEX_{module.node_name.upper()}_{fault.name.upper()}"
                f.write(f"\t[{enum_val}] = {{{fault.time_to_latch}, {fault.max_val}, {fault.min_val}, {fault.time_to_latch}, {fault.time_to_unlatch}, FAULT_PRIO_{fault.priority.upper()}}},\n")
        f.write("};\n\n")

        # Conditional string array
        f.write("#ifdef HAS_FAULT_STRINGS\n")
        f.write("const char* const fault_strings[TOTAL_NUM_FAULTS] = {\n")
        for module in context.fault_modules:
            f.write(f"\t// --- {module.node_name} ---\n")
            for fault in module.faults:
                enum_val = f"FAULT_INDEX_{module.node_name.upper()}_{fault.name.upper()}"
                msg = f'"{fault.lcd_message}"' if fault.lcd_message else "nullptr"
                f.write(f"\t[{enum_val}] = {msg},\n")
        f.write("};\n")
        f.write("#endif\n\n")

        for module in context.fault_modules:
            f.write(f"// --- {module.node_name} Callbacks ---\n")
            f.write(f"#ifndef CAN_NODE_{module.node_name.upper()}\n")
            
            # SYNC CALLBACK
            f.write(f"void {module.node_name.lower()}_fault_sync_CALLBACK(can_data_t* can_data) {{\n")
            for fault in module.faults:
                enum_val = f"FAULT_INDEX_{module.node_name.upper()}_{fault.name.upper()}"
                f.write(f"\tfaults[{enum_val}].is_latched = can_data->{module.node_name.lower()}_fault_sync.{fault.name};\n")
            f.write("}\n\n")
            
            # EVENT CALLBACK
            f.write(f"void {module.node_name.lower()}_fault_event_CALLBACK(can_data_t* can_data) {{\n")
            f.write(f"\tuint16_t idx = can_data->{module.node_name.lower()}_fault_event.idx;\n")
            f.write(f"\tif (idx < TOTAL_NUM_FAULTS) {{\n")
            f.write(f"\t\tfaults[idx].is_latched = (can_data->{module.node_name.lower()}_fault_event.state != 0);\n")
            f.write(f"\t}}\n")
            f.write("}\n")
            f.write("#else\n")
            f.write(f"void {module.node_name.lower()}_fault_sync_CALLBACK(can_data_t* can_data) {{ (void)can_data; }}\n")
            f.write(f"void {module.node_name.lower()}_fault_event_CALLBACK(can_data_t* can_data) {{ (void)can_data; }}\n")
            f.write("#endif\n\n")

        # TX Helpfer: tx_fault_sync
        f.write("// --- Transmission Helpers ---\n")
        f.write("void tx_fault_sync(void) {\n")
        for module in context.fault_modules:
            f.write(f"#ifdef CAN_NODE_{module.node_name.upper()}\n")
            f.write(f"\tCAN_SEND_{module.node_name.lower()}_fault_sync(\n")
            for i, fault in enumerate(module.faults):
                enum_val = f"FAULT_INDEX_{module.node_name.upper()}_{fault.name.upper()}"
                comma = "," if i < len(module.faults) - 1 else ""
                f.write(f"\t\tfaults[{enum_val}].is_latched{comma}\n")
            f.write(f"\t);\n")
            f.write("#endif\n")
        f.write("}\n\n")

        # TX Helper: tx_fault_event
        f.write("void tx_fault_event(fault_index_t idx, uint16_t value) {\n")
        for module in context.fault_modules:
            # We determine the start and end of this node's range for validation
            f_start = f"FAULT_INDEX_{module.node_name.upper()}_{module.faults[0].name.upper()}"
            f_end = f"FAULT_INDEX_{module.node_name.upper()}_{module.faults[-1].name.upper()}"
            
            f.write(f"#ifdef CAN_NODE_{module.node_name.upper()}\n")
            f.write(f"\tif (idx >= {f_start} && idx <= {f_end}) {{\n")
            f.write(f"\t\tCAN_SEND_{module.node_name.lower()}_fault_event(idx, faults[idx].is_latched, value);\n")
            f.write(f"\t}}\n")
            f.write("#endif\n")
        f.write("}\n")

    print_as_ok(f"Generated {filename.name}")

