"""
codegen.py

Author: Irving Wang (irvingw@purdue.edu)
"""

from typing import List, Dict
from parser import Node, Message, SystemContext
from utils import GENERATED_DIR, print_as_success, print_as_ok, get_jinja_env, render_template

def generate_headers(context: SystemContext):
    print("Generating headers...")
    env = get_jinja_env()
    
    # Ensure generated directory exists
    GENERATED_DIR.mkdir(exist_ok=True)

    # Generate types header
    generate_types_header(env, context.custom_types)

    # Generate header for each bus
    for bus_name, view in context.busses.items():
        config = context.bus_configs.get(bus_name, {})
        generate_bus_header(env, bus_name, config, view.messages, context.custom_types)

    # Generate header for each node
    generate_node_headers(env, context)

    # Generate router header
    generate_router_header(env, context.nodes)
    
    print_as_success("Successfully generated C headers")

def generate_types_header(env, custom_types: Dict):
    render_template(env, 'can_types.h.jinja', 
                    GENERATED_DIR / "can_types.h", 
                    custom_types=custom_types)
    print_as_ok("Generated can_types.h")

def generate_router_header(env, nodes: List[Node]):
    render_template(env, 'can_router.h.jinja',
                    GENERATED_DIR / "can_router.h",
                    nodes=nodes)
    print_as_ok("Generated can_router.h")

def generate_node_headers(env, context: SystemContext):
    for node in context.nodes:
        if node.is_external:
            continue
        generate_node_header(env, node, context)

def generate_node_header(env, node: Node, context: SystemContext):
    filename = GENERATED_DIR / f"{node.name}.h"
    mapping = context.mappings.get(node.name)
    
    # Collect all messages this node sees (TX and RX)
    rx_msgs = []
    tx_msgs = []
    
    peripherals = sorted(list(set(bus.peripheral for bus in node.busses.values())))
    node_busses = sorted(node.busses.keys())
    
    for bus_name in node_busses:
        bus = node.busses[bus_name]
        for rx_msg in bus.rx_messages:
            if rx_msg.resolved_message:
                rx_msgs.append((rx_msg, bus.peripheral, bus_name))
        for msg in bus.tx_messages:
            tx_msgs.append((msg, bus.peripheral, bus_name))
            
    render_template(env, 'node_header.h.jinja',
                    filename,
                    node=node,
                    context=context,
                    mapping=mapping,
                    rx_msgs=rx_msgs,
                    tx_msgs=tx_msgs,
                    peripherals=peripherals,
                    node_busses=node_busses)
    print_as_ok(f"Generated {filename.name}")

def generate_bus_header(env, bus_name: str, config: Dict, messages: List[Message], custom_types: Dict):
    render_template(env, 'bus_header.h.jinja',
                    GENERATED_DIR / f"{bus_name}.h",
                    bus_name=bus_name,
                    config=config,
                    messages=messages)
    print_as_ok(f"Generated {bus_name}.h")
