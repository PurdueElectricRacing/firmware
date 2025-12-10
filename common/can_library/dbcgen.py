from typing import List, Dict
from parser import Node

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
