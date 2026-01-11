from typing import List, Dict
from parser import Node, Message
from cantools import db
from utils import DBC_DIR, load_json, COMMON_TYPES_CONFIG_PATH, print_as_success, get_git_hash

def generate_dbcs(nodes: List[Node]):
    """
    Generates DBC files for each bus in the system.
    """
    print("Generating DBCs...")

    custom_types = {}
    try:
        types_data = load_json(COMMON_TYPES_CONFIG_PATH)
        for t in types_data.get("types", []):
            custom_types[t["name"]] = t
    except FileNotFoundError:
        pass

    # Group messages by bus
    bus_messages: Dict[str, List[Message]] = {}
    bus_nodes: Dict[str, set] = {}

    for node in nodes:
        for bus_name, bus in node.busses.items():
            if bus_name not in bus_messages:
                bus_messages[bus_name] = []
                bus_nodes[bus_name] = set()
            
            bus_messages[bus_name].extend(bus.tx_messages)
            bus_nodes[bus_name].add(node.name)

    # Ensure output directory exists and is clean
    if DBC_DIR.exists():
        for f in DBC_DIR.glob("*.dbc"):
            f.unlink()
    DBC_DIR.mkdir(exist_ok=True)

    git_hash = get_git_hash()

    for bus_name, messages in bus_messages.items():
        can_db = db.Database()
        
        # Add nodes
        for node_name in sorted(bus_nodes[bus_name]):
            can_db.nodes.append(db.Node(name=node_name, comment=""))

        # Sort messages by ID for deterministic output
        messages.sort(key=lambda x: x.final_id)

        # Add messages
        for msg in messages:
            signals = []
            
            # Sort signals by bit offset for deterministic output
            sorted_signals = sorted(msg.signals, key=lambda x: x.bit_offset)
            
            for sig in sorted_signals:
                signals.append(db.Signal(
                    name=sig.name,
                    start=sig.bit_offset,
                    length=sig.length,
                    byte_order="little_endian",
                    is_signed=sig.is_signed,
                    initial=sig.offset,
                    scale=sig.scale,
                    offset=sig.offset,
                    minimum=sig.min_val,
                    maximum=sig.max_val,
                    unit=sig.unit if sig.unit else "",
                    comment=sig.desc
                ))
            
            # Find the sender node for this message on this bus
            sender = "Vector__XXX"
            for node in nodes:
                if bus_name in node.busses:
                    if any(m.name == msg.name for m in node.busses[bus_name].tx_messages):
                        sender = node.name
                        break

            can_db.messages.append(db.Message(
                frame_id=msg.final_id,
                name=msg.name,
                length=msg.get_dlc(custom_types),
                signals=signals,
                comment=msg.desc,
                is_extended_frame=msg.is_extended_frame,
                senders=[sender]
            ))
        
        filename = f"{bus_name}_{git_hash}.dbc"
        output_path = DBC_DIR / filename
        with open(output_path, 'w', newline='\n') as f:
            f.write(can_db.as_dbc_string())
        
        print_as_success(f"Generated {filename}")

def generate_debug(nodes: List[Node]):
    """
    Temporary debug generation to verify linker output.
    Writes message names and IDs to debug_<BUS>.txt
    """
    # Dictionary to hold messages per bus: bus_name -> list of (msg_name, msg_id)
    bus_map: Dict[str, List[tuple]] = {}

    for node in nodes:
        for bus_name, bus in node.busses.items():
            if bus_name not in bus_map:
                bus_map[bus_name] = []
            
            for msg in bus.tx_messages:
                # msg.final_id should be populated by linker
                bus_map[bus_name].append((msg.name, msg.final_id))

    # Write output files
    for bus_name, msgs in bus_map.items():
        # Sort by ID for easier reading
        msgs.sort(key=lambda x: x[1])
        
        filename = f"debug_{bus_name}.txt"
        with open(filename, "w") as f:
            f.write(f"Debug Output for Bus: {bus_name}\n")
            f.write("=" * 30 + "\n")
            for name, mid in msgs:
                f.write(f"0x{mid:03X} : {name}\n")
        
        print(f"Generated debug file: {filename}")

