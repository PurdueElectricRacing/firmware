from dataclasses import dataclass, field
from typing import List, Dict, Tuple, Optional
from parser import Node, Message, RxMessage

@dataclass
class FilterBank:
    bank_idx: int
    msg1: Optional[Message] = None
    msg2: Optional[Message] = None
    is_ext1: bool = False
    is_ext2: bool = False

@dataclass
class NodeMapping:
    node_name: str
    # Peripheral -> List of FilterBanks
    filters: Dict[str, List[FilterBank]] = field(default_factory=dict)

def map_hardware(nodes: List[Node], bus_configs: Dict) -> Dict[str, NodeMapping]:
    """
    Hardware Mapper stage.
    Assigns physical resources (like bxCAN filter banks) to nodes.
    """
    mappings = {}
    for node in nodes:
        if node.is_external:
            continue
        mappings[node.name] = map_node_hardware(node, bus_configs)
    return mappings

def map_node_hardware(node: Node, bus_configs: Dict) -> NodeMapping:
    mapping = NodeMapping(node_name=node.name)
    
    peripherals = sorted(list(set(bus.peripheral for bus in node.busses.values())))
    
    # Group RX messages by peripheral
    periph_to_msgs = {p: [] for p in peripherals}
    for bus_name, bus in node.busses.items():
        for rx_msg in bus.rx_messages:
            if rx_msg.resolved_message:
                periph_to_msgs[bus.peripheral].append((rx_msg.resolved_message, bus_name))
    
    for periph in peripherals:
        msgs = periph_to_msgs[periph]
        mapping.filters[periph] = []
        
        if not msgs:
            continue
            
        # bxCAN filter bank assignment
        # CAN1: 0-13, CAN2: 14-27
        bank_offset = 0 if periph == "CAN1" else 14
        
        for i in range(0, len(msgs), 2):
            bank_idx = bank_offset + (i // 2)
            msg1, bus1 = msgs[i]
            msg2, bus2 = msgs[i+1] if i+1 < len(msgs) else (None, None)
            
            is_ext1 = msg1.is_extended or bus_configs.get(bus1, {}).get('is_extended_id', False)
            is_ext2 = False
            if msg2:
                is_ext2 = msg2.is_extended or bus_configs.get(bus2, {}).get('is_extended_id', False)
            
            fb = FilterBank(
                bank_idx=bank_idx,
                msg1=msg1,
                msg2=msg2,
                is_ext1=is_ext1,
                is_ext2=is_ext2
            )
            mapping.filters[periph].append(fb)
            
    return mapping
