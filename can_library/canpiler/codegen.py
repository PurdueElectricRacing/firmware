"""
codegen.py

Author: Irving Wang (irvingw@purdue.edu)
"""

from dataclasses import dataclass, field
from typing import List, Dict, Optional, Any
from parser import Node, Message, SystemContext
from utils import GENERATED_DIR, print_as_success, print_as_ok, get_jinja_env, render_template


@dataclass
class SignalCodec:
    signal: Any
    bswap_width: str
    sign_extend_shift: Optional[int]
    is_float32: bool


@dataclass
class RxEntry:
    rx_msg: Any
    msg: Message
    periph: str
    bus_name: str
    codecs: List[SignalCodec]


@dataclass
class TxEntry:
    msg: Message
    periph: str
    bus_name: str
    codecs: List[SignalCodec]


@dataclass
class ScalingMessage:
    msg: Message
    signals: List[Any]


@dataclass
class PeripheralContext:
    name: str
    index: int


@dataclass
class FdcanFilterContext:
    periph: str
    accept_all: bool
    std_ids: List[Message] = field(default_factory=list)
    ext_ids: List[Message] = field(default_factory=list)

    @property
    def has_filters(self) -> bool:
        return bool(self.std_ids or self.ext_ids)


@dataclass
class BxcanFilterBankContext:
    bank_idx: int
    msg1: Message
    msg2: Message
    is_ext1: bool
    is_ext2: bool
    has_second_msg: bool


@dataclass
class BxcanFilterContext:
    periph: str
    accept_all: bool
    accept_bank_idx: int
    banks: List[BxcanFilterBankContext] = field(default_factory=list)


@dataclass
class FilterRenderContext:
    fdcan: List[FdcanFilterContext] = field(default_factory=list)
    bxcan: List[BxcanFilterContext] = field(default_factory=list)

    @property
    def has_fdcan(self) -> bool:
        return bool(self.fdcan)

    @property
    def has_bxcan(self) -> bool:
        return bool(self.bxcan)


@dataclass
class NodeRenderContext:
    node: Node
    context: SystemContext
    mapping: Any
    rx_entries: List[RxEntry]
    tx_entries: List[TxEntry]
    peripherals: List[str]
    peripheral_entries: List[PeripheralContext]
    node_busses: List[str]
    scaling_messages: List[ScalingMessage]
    stale_rx_entries: List[RxEntry]
    filters: FilterRenderContext


@dataclass
class TypeRenderContext:
    name: str
    prefix: str
    base_type: Optional[str]
    choices: List[str]


def build_type_render_context(custom_types: Dict) -> List[TypeRenderContext]:
    rows = []
    for name, config in custom_types.items():
        prefix = name[:-2].upper() if name.endswith("_t") else name.upper()
        rows.append(TypeRenderContext(
            name=name,
            prefix=prefix,
            base_type=config.get("base_type"),
            choices=config.get("choices", []),
        ))
    return rows


def build_signal_codec(sig, direction: str) -> SignalCodec:
    if direction not in ("rx", "tx"):
        raise ValueError(f"Unknown codec direction: {direction}")

    bswap_width = "BSWAP_NONE"
    if sig.byte_order == "big_endian" and sig.length in (16, 32, 64):
        bswap_width = f"BSWAP_{sig.length}"

    return SignalCodec(
        signal=sig,
        bswap_width=bswap_width,
        sign_extend_shift=64 - sig.length if sig.is_signed and sig.length < 64 else None,
        is_float32=sig.is_floating_point and sig.length == 32,
    )


def build_scaling_messages(rx_entries: List[RxEntry], tx_entries: List[TxEntry]) -> List[ScalingMessage]:
    by_name: Dict[str, Message] = {}
    for entry in tx_entries:
        by_name.setdefault(entry.msg.name, entry.msg)
    for entry in rx_entries:
        by_name.setdefault(entry.msg.name, entry.msg)

    scaling_messages = []
    for msg in sorted(by_name.values(), key=lambda m: m.name):
        signals = [sig for sig in msg.signals if sig.scale != 1.0]
        if signals:
            scaling_messages.append(ScalingMessage(msg=msg, signals=signals))
    return scaling_messages


def build_filter_render_context(mapping, peripherals: List[str]) -> FilterRenderContext:
    filters = FilterRenderContext()
    if mapping is None:
        return filters

    for periph in peripherals:
        accept_all = mapping.accept_all.get(periph, False)
        if periph.startswith("FDCAN"):
            fdcan_filt = mapping.fdcan_filters.get(periph)
            filters.fdcan.append(FdcanFilterContext(
                periph=periph,
                accept_all=accept_all,
                std_ids=list(fdcan_filt.std_ids) if fdcan_filt else [],
                ext_ids=list(fdcan_filt.ext_ids) if fdcan_filt else [],
            ))
            continue

        banks = []
        for fb in mapping.filters.get(periph, []):
            banks.append(BxcanFilterBankContext(
                bank_idx=fb.bank_idx,
                msg1=fb.msg1,
                msg2=fb.msg2 if fb.msg2 else fb.msg1,
                is_ext1=fb.is_ext1,
                is_ext2=fb.is_ext2 if fb.msg2 else fb.is_ext1,
                has_second_msg=fb.msg2 is not None,
            ))

        filters.bxcan.append(BxcanFilterContext(
            periph=periph,
            accept_all=accept_all,
            accept_bank_idx=0 if periph == "CAN1" else 14,
            banks=banks,
        ))
    return filters


def build_node_render_context(node: Node, context: SystemContext) -> NodeRenderContext:
    mapping = context.mappings.get(node.name)
    rx_entries: List[RxEntry] = []
    tx_entries: List[TxEntry] = []
    peripherals = sorted(list(set(bus.peripheral for bus in node.busses.values())))
    peripheral_entries = [
        PeripheralContext(name=periph, index=int(periph[-1]) - 1)
        for periph in peripherals
    ]
    node_busses = sorted(node.busses.keys())

    for bus_name in node_busses:
        bus = node.busses[bus_name]
        for rx_msg in bus.rx_messages:
            if rx_msg.resolved_message:
                msg = rx_msg.resolved_message
                rx_entries.append(RxEntry(
                    rx_msg=rx_msg,
                    msg=msg,
                    periph=bus.peripheral,
                    bus_name=bus_name,
                    codecs=[build_signal_codec(sig, "rx") for sig in msg.signals],
                ))
        for msg in bus.tx_messages:
            tx_entries.append(TxEntry(
                msg=msg,
                periph=bus.peripheral,
                bus_name=bus_name,
                codecs=[build_signal_codec(sig, "tx") for sig in msg.signals],
            ))

    return NodeRenderContext(
        node=node,
        context=context,
        mapping=mapping,
        rx_entries=rx_entries,
        tx_entries=tx_entries,
        peripherals=peripherals,
        peripheral_entries=peripheral_entries,
        node_busses=node_busses,
        scaling_messages=build_scaling_messages(rx_entries, tx_entries),
        stale_rx_entries=[entry for entry in rx_entries if entry.msg.period > 0],
        filters=build_filter_render_context(mapping, peripherals),
    )

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
        generate_bus_header(env, bus_name, config, view.messages)

    # Generate header for each node
    generate_node_headers(env, context)

    # Generate router header
    generate_router_header(env, context.nodes)
    
    print_as_success("Successfully generated C headers")

def generate_types_header(env, custom_types: Dict):
    render_template(env, 'can_types.h.jinja', 
                    GENERATED_DIR / "can_types.h", 
                    types=build_type_render_context(custom_types))
    print_as_ok("Generated can_types.h")

def generate_router_header(env, nodes: List[Node]):
    render_template(env, 'can_router.h.jinja',
                    GENERATED_DIR / "can_router.h",
                    nodes=nodes,
                    router_nodes=[node for node in nodes if not node.is_external])
    print_as_ok("Generated can_router.h")

def generate_node_headers(env, context: SystemContext):
    for node in context.nodes:
        if node.is_external:
            continue
        generate_node_header(env, node, context)

def generate_node_header(env, node: Node, context: SystemContext):
    filename = GENERATED_DIR / f"{node.name}.h"
    render_context = build_node_render_context(node, context)
    
    # TODO: don't require messages that start with "reserved_" to be used in the CAN_SEND_
    # Instead have them get 0-ed automatically
    render_template(env, 'node_header.h.jinja',
                    filename,
                    ctx=render_context)
    print_as_ok(f"Generated {filename.name}")

def generate_bus_header(env, bus_name: str, config: Dict, messages: List[Message]):
    render_template(env, 'bus_header.h.jinja',
                    GENERATED_DIR / f"{bus_name}.h",
                    bus_name=bus_name,
                    config=config,
                    messages=messages)
    print_as_ok(f"Generated {bus_name}.h")
