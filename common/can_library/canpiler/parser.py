"""
parser.py

Author: Irving Wang (irvingw@purdue.edu)
"""

from dataclasses import dataclass, field
from typing import List, Optional, Dict, Set
from pathlib import Path
from utils import load_json, NODE_CONFIG_DIR, EXTERNAL_NODE_CONFIG_DIR, COMMON_TYPES_CONFIG_PATH, FAULT_CONFIG_PATH, BUS_CONFIG_PATH, CTYPE_SIZES, print_as_error, print_as_ok, print_as_success, to_macro_name

@dataclass
class Signal:
    name: str
    datatype: str
    desc: str = ""
    length: int = 0
    unit: Optional[str] = None
    choices: Optional[List[str]] = None
    scale: float = 1.0
    offset: float = 0.0
    min_val: Optional[float] = None
    max_val: Optional[float] = None
    
    # Resolved during parsing/linking
    bit_offset: int = 0
    bit_shift: int = 0
    is_signed: bool = False
    mask: int = 0

    @property
    def macro_name(self) -> str:
        return to_macro_name(self.name)

    @property
    def c_type(self) -> str:
        return self.datatype

    @property
    def is_floating_point(self) -> bool:
        return self.datatype in ['float', 'double']

    def get_bit_length(self, custom_types: Optional[Dict] = None) -> int:
        if self.length > 0:
            return self.length
        
        if self.datatype in CTYPE_SIZES:
            return CTYPE_SIZES[self.datatype]
        
        if custom_types and self.datatype in custom_types:
            base = custom_types[self.datatype]['base_type']
            return CTYPE_SIZES.get(base, 0)
            
        return 0

@dataclass
class Message:
    name: str
    desc: str = ""
    signals: List[Signal] = field(default_factory=list)
    priority: int = 0
    period: int = 0
    id_override: Optional[str] = None
    is_extended: bool = False
    final_id: int = 0

    @property
    def macro_name(self) -> str:
        return to_macro_name(self.name)

    @property
    def stale_threshold(self) -> int:
        return int(self.period * 2.5)

    def resolve_layout(self, custom_types: Dict) -> None:
        """
        Calculate bit offsets, shifts, and masks for all signals.
        This is intrinsic to the message definition.
        """
        current_offset = 0
        for sig in self.signals:
            length = sig.get_bit_length(custom_types)
            sig.length = length
            sig.bit_offset = current_offset
            sig.bit_shift = 64 - current_offset - length
            sig.mask = (1 << length) - 1
            
            # Resolve signedness
            base_type = sig.datatype
            if custom_types and sig.datatype in custom_types:
                base_type = custom_types[sig.datatype]['base_type']
            sig.is_signed = base_type.startswith('int')
            
            current_offset += length

    def validate_semantics(self, custom_types: Dict) -> None:
        """
        Perform semantic checks that require external context (like custom types).
        Raises ValueError if invalid.
        """
        total_length = sum(sig.get_bit_length(custom_types) for sig in self.signals)

        for sig in self.signals:
            if sig.datatype not in CTYPE_SIZES and sig.datatype not in custom_types:
                print_as_error(f"Signal '{sig.name}' in message '{self.name}' has unknown type '{sig.datatype}'")
                raise ValueError("Unknown signal type")

        if total_length > 64:
            print_as_error(f"Message '{self.name}' exceeds 64 bits (has {total_length})")
            raise ValueError("Message too long")
        
        # Validate ID Override Range
        if self.id_override:
            try:
                raw_id = int(self.id_override, 0)
                if not self.is_extended and raw_id > 0x7FF:
                    print_as_error(f"Message '{self.name}' has override ID {hex(raw_id)} which exceeds 11-bit limit for standard bus.")
                    raise ValueError(f"ID override too large for standard bus: {self.name}")
                if raw_id > 0x1FFFFFFF:
                    print_as_error(f"Message '{self.name}' has override ID {hex(raw_id)} which exceeds 29-bit CAN limit.")
                    raise ValueError(f"ID override exceeds CAN protocol limits: {self.name}")
            except ValueError as e:
                if "invalid literal" in str(e):
                    print_as_error(f"Message '{self.name}' has invalid ID override format: {self.id_override}")
                    raise ValueError("Invalid ID override format")
                raise

    def get_total_bit_length(self, custom_types: Optional[Dict] = None) -> int:
        return sum(sig.get_bit_length(custom_types) for sig in self.signals)

    def get_dlc(self, custom_types: Dict) -> int:
        """Calculate the Data Length Code (DLC) in bytes."""
        total_bits = self.get_total_bit_length(custom_types)
        return (total_bits + 7) // 8

@dataclass
class RxMessage:
    name: str
    callback: bool = False
    irq: bool = False
    resolved_message: Optional[Message] = None # Resolved during linking stage

@dataclass
class Bus:
    name: str
    peripheral: str
    tx_messages: List[Message] = field(default_factory=list)
    rx_messages: List[RxMessage] = field(default_factory=list)
    accept_all_messages: bool = False

    @property
    def macro_name(self) -> str:
        return to_macro_name(self.name)

@dataclass
class Node:
    name: str
    system_ids: Dict[str, int] = field(default_factory=dict)
    busses: Dict[str, Bus] = field(default_factory=dict)
    is_external: bool = False
    scheduler: str = "psched"

    @property
    def macro_name(self) -> str:
        return to_macro_name(self.name)

    @property
    def messages(self) -> List[Message]:
        """Returns all TX messages across all busses for this node."""
        all_msgs = []
        for bus in self.busses.values():
            all_msgs.extend(bus.tx_messages)
        return all_msgs

@dataclass
class Fault:
    name: str
    max_val: int
    min_val: int
    priority: str
    time_to_latch: int
    time_to_unlatch: int
    lcd_message: str
    absolute_index: int = 0
    id: int = 0

    @property
    def macro_name(self) -> str:
        return to_macro_name(self.name)

@dataclass
class FaultModule:
    node_name: str
    generate_strings: bool
    faults: List[Fault] = field(default_factory=list)
    node_id: int = 0

    @property
    def macro_name(self) -> str:
        return to_macro_name(self.node_name)

@dataclass
class BusView:
    name: str
    messages: List[Message] = field(default_factory=list)
    nodes: Set[str] = field(default_factory=set)
    sender_map: Dict[str, str] = field(default_factory=dict) # msg_name -> sender_node_name

@dataclass
class SystemContext:
    nodes: List[Node] = field(default_factory=list)
    busses: Dict[str, BusView] = field(default_factory=dict)
    mappings: Dict = field(default_factory=dict)
    fault_modules: List['FaultModule'] = field(default_factory=list)
    bus_configs: Dict = field(default_factory=dict)
    custom_types: Dict = field(default_factory=dict)

def create_system_context(nodes, mappings, fault_modules, bus_configs, custom_types) -> SystemContext:
    """Aggregates all system data into a single context object and performs final validation."""
    context = SystemContext(
        nodes=nodes, 
        fault_modules=fault_modules, 
        mappings=mappings,
        bus_configs=bus_configs,
        custom_types=custom_types
    )
    
    # Identify all busses across all nodes
    all_bus_names = set()
    for node in nodes:
        all_bus_names.update(node.busses)
    
    for bus_name in sorted(all_bus_names):
        view = BusView(name=bus_name)
        
        for node in nodes:
            if bus_name in node.busses:
                view.nodes.add(node.name)
                for msg in node.busses[bus_name].tx_messages:
                    view.messages.append(msg)
                    view.sender_map[msg.name] = node.name
        
        # Deterministic sorting for downstream generators
        view.messages.sort(key=lambda x: (x.final_id or 0, x.name))
        context.busses[bus_name] = view
        
    return context

# --- Parsing Logic ---

def load_custom_types() -> Dict:
    """Load custom types from common_types.json"""
    try:
        data = load_json(COMMON_TYPES_CONFIG_PATH)
        custom_types = {}
        for t in data.get("types", []):
            custom_types[t["name"]] = t
        return custom_types
    except FileNotFoundError:
        return {}

def load_bus_configs() -> Dict:
    """Load bus configurations from bus_configs.json"""
    try:
        data = load_json(BUS_CONFIG_PATH)
        return {b["name"]: b for b in data.get("busses", [])}
    except FileNotFoundError:
        return {}

def parse_all() -> List[Node]:
    """
    Parse all configuration files into Node objects.
    Performs semantic validation during parsing.
    """

    print("Parsing configs and performing semantic validation...")

    nodes = []
    custom_types = load_custom_types()
    bus_configs = load_bus_configs()

    # Parse Internal Nodes
    if NODE_CONFIG_DIR.exists():
        for node_file in sorted(NODE_CONFIG_DIR.glob('*.json')):
            node = parse_internal_node(node_file, bus_configs)
            validate_node(node, custom_types)
            nodes.append(node)
            print_as_ok(f"Parsed {node.name}")

    # Parse External Nodes
    if EXTERNAL_NODE_CONFIG_DIR.exists():
        for node_file in sorted(EXTERNAL_NODE_CONFIG_DIR.glob('*.json')):
            node = parse_external_node(node_file, bus_configs)
            validate_node(node, custom_types)
            nodes.append(node)
            print_as_ok(f"Parsed {node.name}")
    
    print_as_success("All nodes parsed successfully");
    return nodes

def validate_node(node: Node, custom_types: Dict):
    """Run semantic validation on a node"""
    for bus in node.busses.values():
        for msg in bus.tx_messages:
            msg.validate_semantics(custom_types)

def parse_signal(data: Dict) -> Signal:
    return Signal(
        name=data['sig_name'],
        datatype=data['type'],
        desc=data.get('sig_desc', ''),
        length=data.get('length', 0),
        unit=data.get('unit'),
        choices=data.get('choices'),
        scale=data.get('scale', 1.0),
        offset=data.get('offset', 0.0),
        min_val=data.get('min'),
        max_val=data.get('max')
    )

def parse_message(data: Dict, bus_config: Dict) -> Message:
    # Single source of truth: the bus configuration
    is_extended = bus_config.get('is_extended_id', False)

    return Message(
        name=data['msg_name'],
        desc=data.get('msg_desc', ''),
        signals=[parse_signal(s) for s in data.get('signals', [])],
        priority=data.get('msg_priority', 0),
        period=data.get('msg_period', 0),
        id_override=data.get('msg_id_override'),
        is_extended=is_extended
    )

def parse_rx_message(data: Dict) -> RxMessage:
    return RxMessage(
        name=data['msg_name'],
        callback=data.get('callback', False),
        irq=data.get('irq', False)
    )

def parse_bus(name: str, data: Dict, bus_configs: Dict) -> Bus:
    bus_config = bus_configs.get(name, {})
    return Bus(
        name=name,
        peripheral=data.get('peripheral', 'UNKNOWN'),
        tx_messages=[parse_message(m, bus_config) for m in data.get('tx', [])],
        rx_messages=[parse_rx_message(m) for m in data.get('rx', [])],
        accept_all_messages=data.get('accept_all_messages', False)
    )

def parse_internal_node(filepath: Path, bus_configs: Dict) -> Node:
    data = load_json(filepath)
    busses = {}
    for bus_name, bus_data in data.get('busses', {}).items():
        busses[bus_name] = parse_bus(bus_name, bus_data, bus_configs)
    
    node = Node(
        name=data['node_name'],
        busses=busses,
        is_external=False,
        scheduler=data.get('scheduler', 'psched')
    )
    return node

def parse_external_node(filepath: Path, bus_configs: Dict) -> Node:
    data = load_json(filepath)
    bus_name = data['bus_name']
    bus_config = bus_configs.get(bus_name, {})
    
    # Create a single bus from the flattened structure
    bus = Bus(
        name=bus_name,
        peripheral="UNKNOWN",
        tx_messages=[parse_message(m, bus_config) for m in data.get('tx', [])],
        rx_messages=[parse_rx_message(m) for m in data.get('rx', [])]
    )
    
    return Node(
        name=data['node_name'],
        busses={bus_name: bus},
        is_external=True
    )
