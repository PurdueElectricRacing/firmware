"""
dbcgen.py

Author: Irving Wang (irvingw@purdue.edu)
"""

from parser import SystemContext
from cantools import db
from utils import DBC_DIR, print_as_success, print_as_ok, get_git_hash

def generate_dbcs(context: SystemContext):
    """
    Generates DBC files for each bus in the system.
    """
    print("Generating DBCs...")

    # Ensure output directory exists and is clean
    if DBC_DIR.exists():
        for f in DBC_DIR.glob("*.dbc"):
            f.unlink()
    DBC_DIR.mkdir(exist_ok=True)

    git_hash = get_git_hash()

    for bus_name, view in context.busses.items():
        can_db = db.Database()
        
        # Add nodes
        for node_name in sorted(view.nodes):
            can_db.nodes.append(db.Node(name=node_name, comment=""))

        # Add messages
        for msg in view.messages:
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
            
            # Use pre-calculated sender mapping
            sender = view.sender_map.get(msg.name, "Vector__XXX")

            can_db.messages.append(db.Message(
                frame_id=msg.final_id,
                name=msg.name,
                length=msg.get_dlc(context.custom_types),
                signals=signals,
                comment=msg.desc,
                is_extended_frame=msg.is_extended,
                senders=[sender]
            ))
        
        filename = f"{bus_name}_{git_hash}.dbc"
        output_path = DBC_DIR / filename
        with open(output_path, 'w', newline='\n') as f:
            f.write(can_db.as_dbc_string())
        
        print_as_ok(f"Generated {filename}")

    print_as_success("Successfully generated DBC files")

