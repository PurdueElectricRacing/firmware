"""
codegen/hardware.py

Author: Irving Wang (irvingw@purdue.edu)
"""

from collections import defaultdict
from dataclasses import dataclass
from typing import Mapping

from ..pipeline.models import CompiledNode, LinkedCan, LinkedMessage, frozen_mapping
from core.utils import print_as_warning

# PHAL FDCAN per-ID-type filter limits.
MAX_SID_FILTERS = 28
MAX_XID_FILTERS = 8


@dataclass(frozen=True)
class PeripheralFilters:
    accept_all: bool
    std_ids: tuple[LinkedMessage, ...] = ()
    ext_ids: tuple[LinkedMessage, ...] = ()


@dataclass(frozen=True)
class NodeHardwareMap:
    filters: Mapping[str, PeripheralFilters]


HardwareMap = Mapping[str, NodeHardwareMap]


def map_hardware(linked: LinkedCan) -> HardwareMap:
    """
    Hardware Mapper stage.
    Builds per-peripheral PHAL filter lists for nodes.
    """
    hardware_map = {}

    for node in linked.nodes:
        if node.is_external:
            continue

        hardware_map[node.name] = map_node_hardware(node, linked)

    return frozen_mapping(hardware_map)


def map_node_hardware(
    node: CompiledNode,
    linked: LinkedCan,
) -> NodeHardwareMap:
    peripherals = sorted({bus.peripheral for bus in node.busses.values()})

    periph_to_buses: dict[str, list[str]] = defaultdict(list)

    for bus_name, bus in node.busses.items():
        periph_to_buses[bus.peripheral].append(bus_name)

    for periph, bus_names in sorted(periph_to_buses.items()):
        unique_buses = sorted(set(bus_names))
        if len(unique_buses) > 1:
            print_as_warning(
                f"Node '{node.name}': peripheral '{periph}' is used by multiple logical buses "
                f"({', '.join(unique_buses)}). RX filters are merged per peripheral; confirm this is intended."
            )

    # Group linked RX messages by peripheral.
    periph_to_msgs: dict[str, list[LinkedMessage]] = {
        peripheral: []
        for peripheral in peripherals
    }

    for subscription in linked.subscriptions:
        if subscription.node_name != node.name:
            continue

        attachment = node.busses[subscription.bus_name]
        periph_to_msgs[attachment.peripheral].append(subscription.resolved_message)

    filters = {}
    for periph in peripherals:
        msgs = periph_to_msgs[periph]
        accept_all = any(
            bus.accept_all_messages
            for bus in node.busses.values()
            if bus.peripheral == periph
        )

        filters[periph] = map_peripheral_filters(node.name, periph, msgs, accept_all)

    return NodeHardwareMap(filters=frozen_mapping(filters))


def map_peripheral_filters(
    node_name: str,
    periph: str,
    msgs: list[LinkedMessage],
    accept_all: bool,
) -> PeripheralFilters:
    """Build ID lists for PHAL FDCAN filter programming."""
    std_ids = tuple(msg for msg in msgs if not msg.is_extended)
    ext_ids = tuple(msg for msg in msgs if msg.is_extended)

    if len(std_ids) > MAX_SID_FILTERS:
        raise ValueError(
            f"Node '{node_name}' exceeds FDCAN standard ID filter limit for {periph} "
            f"({len(std_ids)} > {MAX_SID_FILTERS})"
        )
    if len(ext_ids) > MAX_XID_FILTERS:
        raise ValueError(
            f"Node '{node_name}' exceeds FDCAN extended ID filter limit for {periph} "
            f"({len(ext_ids)} > {MAX_XID_FILTERS})"
        )
    return PeripheralFilters(accept_all=accept_all, std_ids=std_ids, ext_ids=ext_ids)
