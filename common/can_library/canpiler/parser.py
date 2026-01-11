from dataclasses import dataclass, field
from typing import List, Optional, Dict
from pathlib import Path
from utils import load_json, NODE_CONFIG_DIR, EXTERNAL_NODE_CONFIG_DIR, COMMON_TYPES_CONFIG_PATH, FAULT_CONFIG_PATH, CTYPE_SIZES, print_as_error, print_as_ok, print_as_success, to_macro_name

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
    def is_extended_frame(self) -> bool:
        """
        Source of Truth for frame type. 
        Forces Extended if explicitly requested OR if the ID exceeds 11 bits (0x7FF).
        """
        return self.is_extended or self.final_id > 0x7FF

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
    node_id: Optional[int] = None
    system_ids: Dict[str, int] = field(default_factory=dict)
    busses: Dict[str, Bus] = field(default_factory=dict)
    is_external: bool = False
    scheduler: str = "psched"

    @property
    def macro_name(self) -> str:
        return to_macro_name(self.name)

    def generate_system_ids(self) -> None:
        """Generate system-level CAN IDs based on node_id."""
        if self.node_id is None:
            return
            
        # Formula: ((priority - 1) << 26) | (node_id << 21) | (msg_index << 9)
        # Using high message indices (0x700+) for system messages to avoid collisions
        
        # DAQ Response: Priority 3 (bits 26-28 = 2), Msg Index 0x7E0
        daq_base = (2 << 26) | (self.node_id << 21) | (0x7E0 << 9)
        
        # Fault Sync: Priority 1 (bits 26-28 = 0), Msg Index 0x7E1
        self.system_ids["fault_sync"] = (0 << 26) | (self.node_id << 21) | (0x7E1 << 9)

        for bus_name in self.busses:
            self.system_ids[f"daq_response_{bus_name}"] = daq_base

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

def parse_all() -> List[Node]:
    """
    Parse all configuration files into Node objects.
    Performs semantic validation during parsing.
    """

    print("Parsing configs and performing semantic validation...")

    nodes = []
    custom_types = load_custom_types()

    # Parse Internal Nodes
    if NODE_CONFIG_DIR.exists():
        for node_file in sorted(NODE_CONFIG_DIR.glob('*.json')):
            node = parse_internal_node(node_file)
            validate_node(node, custom_types)
            nodes.append(node)
            print_as_ok(f"Parsed {node.name}")

    # Parse External Nodes
    if EXTERNAL_NODE_CONFIG_DIR.exists():
        for node_file in sorted(EXTERNAL_NODE_CONFIG_DIR.glob('*.json')):
            node = parse_external_node(node_file)
            validate_node(node, custom_types)
            nodes.append(node)
            print_as_ok(f"Parsed {node.name}")
    
    print_as_success("All nodes parsed successfully");
    return nodes

def parse_faults() -> List[FaultModule]:
    """Parse fault configurations and assign indices and IDs."""
    if not FAULT_CONFIG_PATH.exists():
        return []

    print("Parsing fault configurations...")
    data = load_json(FAULT_CONFIG_PATH)
    fault_modules = []
    global_index = 0

    for i, m_data in enumerate(data.get("nodes", [])):
        module = FaultModule(
            node_name=m_data["node_name"],
            generate_strings=m_data.get("generate_strings", False),
            node_id=i
        )
        
        for f_data in m_data.get("faults", []):
            fault = Fault(
                name=f_data["fault_name"],
                max_val=f_data["max"],
                min_val=f_data["min"],
                priority=f_data["priority"],
                time_to_latch=f_data["time_to_latch"],
                time_to_unlatch=f_data["time_to_unlatch"],
                lcd_message=f_data["lcd_message"],
                absolute_index=global_index,
                id=(i << 12) | global_index
            )
            module.faults.append(fault)
            global_index += 1
            
        fault_modules.append(module)
        print_as_ok(f"Parsed faults for {module.node_name}")

    print_as_success(f"Successfully parsed {global_index} faults")
    return fault_modules

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

def parse_message(data: Dict) -> Message:
    # Handle is_extended logic: default False, check is_standard_id (inverse) or is_extended_id
    # If a long ID override is provided, assume extended
    id_override = data.get('msg_id_override')
    is_extended = data.get('is_extended_id', not data.get('is_standard_id', True))
    
    if id_override and int(id_override, 0) > 0x7FF:
        is_extended = True

    return Message(
        name=data['msg_name'],
        desc=data.get('msg_desc', ''),
        signals=[parse_signal(s) for s in data.get('signals', [])],
        priority=data.get('msg_priority', 0),
        period=data.get('msg_period', 0),
        id_override=id_override,
        is_extended=is_extended
    )

def parse_rx_message(data: Dict) -> RxMessage:
    return RxMessage(
        name=data['msg_name'],
        callback=data.get('callback', False),
        irq=data.get('irq', False)
    )

def parse_bus(name: str, data: Dict) -> Bus:
    return Bus(
        name=name,
        peripheral=data.get('peripheral', 'UNKNOWN'),
        tx_messages=[parse_message(m) for m in data.get('tx', [])],
        rx_messages=[parse_rx_message(m) for m in data.get('rx', [])],
        accept_all_messages=data.get('accept_all_messages', False)
    )

def parse_internal_node(filepath: Path) -> Node:
    data = load_json(filepath)
    busses = {}
    for bus_name, bus_data in data.get('busses', {}).items():
        busses[bus_name] = parse_bus(bus_name, bus_data)
    
    node = Node(
        name=data['node_name'],
        node_id=data.get('node_id'),
        busses=busses,
        is_external=False,
        scheduler=data.get('scheduler', 'psched')
    )
    node.generate_system_ids()
    return node

def parse_external_node(filepath: Path) -> Node:
    data = load_json(filepath)
    bus_name = data['bus_name']
    
    # Create a single bus from the flattened structure
    bus = Bus(
        name=bus_name,
        peripheral="UNKNOWN",
        tx_messages=[parse_message(m) for m in data.get('tx', [])],
        rx_messages=[parse_rx_message(m) for m in data.get('rx', [])]
    )
    
    return Node(
        name=data['node_name'],
        busses={bus_name: bus},
        is_external=True
    )
