"""
linker.py

Author: Irving Wang (irvingw@purdue.edu)
"""

from typing import List, Dict, Set, DefaultDict
from collections import defaultdict
from parser import Node, Message
from utils import print_as_error, print_as_success, print_as_ok

class BusLinker:
    def __init__(self, bus_name: str):
        self.bus_name = bus_name
        self.next_avail_id = 1
        self.msgs_by_prio: DefaultDict[int, List[Message]] = defaultdict(list)
        self.overrides_by_prio: DefaultDict[int, Set[int]] = defaultdict(set)
        self.all_used_ids: Set[int] = set()

    def add_message(self, msg: Message):
        self.msgs_by_prio[msg.priority].append(msg)
        if msg.id_override:
            override_val = int(msg.id_override, 0)
            if override_val in self.all_used_ids:
                raise ValueError(f"Bus '{self.bus_name}': Duplicate ID override {hex(override_val)} detected.")
            self.overrides_by_prio[msg.priority].add(override_val)
            self.all_used_ids.add(override_val)

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
        # To ensure absolute determinism across systems, we sort messages
        # by name within the priority group before assigning IDs.
        # This keeps IDs stable even if node discovery order or JSON insertion order changes.
        sorted_msgs = sorted(msgs, key=lambda x: x.name)
        
        for msg in sorted_msgs:
            if msg.id_override:
                msg.final_id = int(msg.id_override, 0)
            else:
                # Find next free slot
                # Skip IDs used by overrides in ANY priority group
                while self.next_avail_id in self.all_used_ids:
                    self.next_avail_id += 1
                
                msg.final_id = self.next_avail_id
                self.all_used_ids.add(msg.final_id)
                self.next_avail_id += 1
            
            # ID Range Validation
            limit = 0x1FFFFFFF if msg.is_extended else 0x7FF
            if msg.final_id > limit:
                type_str = "Extended" if msg.is_extended else "Standard"
                raise ValueError(f"Bus '{self.bus_name}': Message '{msg.name}' ID {hex(msg.final_id)} exceeds {type_str} CAN limit ({hex(limit)})")

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
            print_as_ok(f"Bus '{bus_name}' linked successfully")
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

    # 4. Final Validation
    for node in nodes:
        for bus in node.busses.values():
            for msg in bus.tx_messages:
                if msg.final_id > 0x7FF and not msg.is_extended:
                    print_as_error(f"Message '{msg.name}' has ID {hex(msg.final_id)} but is not marked as extended.")
                    raise ValueError(f"Standard CAN ID exceeds 0x7FF: {hex(msg.final_id)}")

    print_as_success("Successfully linked all CAN IDs")
    return nodes
