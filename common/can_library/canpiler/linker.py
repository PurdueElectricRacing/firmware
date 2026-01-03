from typing import List, Dict, Set, DefaultDict
from collections import defaultdict
from parser import Node, Message, Signal
from utils import print_as_error, print_as_success

class BusLinker:
    def __init__(self, bus_name: str):
        self.bus_name = bus_name
        self.next_avail_id = 1
        self.msgs_by_prio: DefaultDict[int, List[Message]] = defaultdict(list)
        self.overrides_by_prio: DefaultDict[int, Set[int]] = defaultdict(set)

    def add_message(self, msg: Message):
        self.msgs_by_prio[msg.priority].append(msg)
        if msg.id_override:
            self.overrides_by_prio[msg.priority].add(int(msg.id_override, 0))

    def link(self):
        """Execute the two-pass linking algorithm"""
        self._validate_overrides() # Pass 1
        
        # Pass 2: Assign IDs priority by priority
        # We must process priorities in order (0, 1, 2...)
        sorted_priorities = sorted(self.msgs_by_prio.keys())
        for prio in sorted_priorities:
            self._link_priority_group(prio)

    def _validate_overrides(self):
        """
        Pass 1: Validate that overrides respect priority monotonicity.
        An override in Priority 0 (e.g. 0x10) must be < an override in Priority 1 (e.g. 0x20).
        """
        sorted_priorities = sorted(self.overrides_by_prio.keys())
        max_override_seen = -1
        
        for prio in sorted_priorities:
            overrides = self.overrides_by_prio[prio]
            if not overrides:
                continue
                
            current_min = min(overrides)
            current_max = max(overrides)
            
            # Check if this priority group starts before the previous one ended
            if current_min <= max_override_seen:
                print_as_error(f"Bus '{self.bus_name}': Priority {prio} override {hex(current_min)} violates priority order (previous max was {hex(max_override_seen)})")
                raise ValueError("Priority Inversion in Overrides")
            
            max_override_seen = max(max_override_seen, current_max)

    def _link_priority_group(self, priority: int):
        """
        Pass 2: Assign IDs to dynamic messages in this priority group.
        Respects the 'Water Level' (next_avail_id) and skips reserved overrides.
        """
        msgs = self.msgs_by_prio[priority]
        # Get overrides for THIS priority group only
        group_overrides = self.overrides_by_prio[priority]
        
        # 1. Crash Check: Did previous groups push us past our own overrides?
        if group_overrides:
            min_override = min(group_overrides)
            if self.next_avail_id > min_override:
                print_as_error(f"Bus '{self.bus_name}': Dynamic IDs from higher priorities pushed water level to {hex(self.next_avail_id)}, colliding with Priority {priority} override at {hex(min_override)}")
                raise ValueError("Dynamic ID Overflow")

        # 2. Assign IDs
        # Sort messages: Overrides first, then dynamic (stable sort)
        # Actually, order doesn't strictly matter for dynamic within same priority, 
        # but let's keep it deterministic.
        for msg in msgs:
            if msg.id_override:
                msg.final_id = int(msg.id_override, 0)
            else:
                # Find next free slot
                while self.next_avail_id in group_overrides:
                    self.next_avail_id += 1
                
                msg.final_id = self.next_avail_id
                self.next_avail_id += 1

        # 3. Update Water Level
        # Ensure next priority group starts AFTER the highest ID used in this group
        if group_overrides:
            max_override = max(group_overrides)
            self.next_avail_id = max(self.next_avail_id, max_override + 1)

def link_all(nodes: List[Node]) -> List[Node]:
    """
    Main entry point for the Linker stage.
    Resolves CAN IDs for all messages across all buses.
    """
    print("Linking CAN IDs...")
    
    # 1. Group messages by Bus
    linkers: Dict[str, BusLinker] = {}
    
    for node in nodes:
        for bus_name, bus in node.busses.items():
            if bus_name not in linkers:
                linkers[bus_name] = BusLinker(bus_name)
            
            for msg in bus.tx_messages:
                linkers[bus_name].add_message(msg)

    # 2. Execute Linker for each bus
    for bus_name, linker in linkers.items():
        try:
            linker.link()
            print_as_success(f"Bus '{bus_name}' linked successfully")
        except ValueError:
            print_as_error(f"Failed to link bus '{bus_name}'")
            raise

    # 3. Resolve RX Messages
    # Build global message map
    all_tx_messages: Dict[str, Message] = {}
    for node in nodes:
        for bus in node.busses.values():
            for msg in bus.tx_messages:
                if msg.name in all_tx_messages:
                    # This should be caught by validator, but good to check
                    print_as_error(f"Duplicate message name '{msg.name}' found during linking")
                all_tx_messages[msg.name] = msg

    for node in nodes:
        for bus_name, bus in node.busses.items():
            for rx_msg in bus.rx_messages:
                if rx_msg.name in all_tx_messages:
                    rx_msg.resolved_message = all_tx_messages[rx_msg.name]
                else:
                    print_as_error(f"Node '{node.name}': RX message '{rx_msg.name}' not found in any bus")
                    raise ValueError(f"Unresolved RX message: {rx_msg.name}")

    return nodes
