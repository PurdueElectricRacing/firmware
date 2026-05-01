# CAN Configs

## Bus Definition (`configs/system/bus_configs.json`)
Validated by `bus_schema.json`. Describes each logical CAN bus (not how a node attaches to it).

- `name`: Logical bus name (referenced by nodes and external nodes).
- `baud_rate`: CAN bitrate for this bus.
- `is_extended_id`: Boolean. Mixed-ID buses are not supported; all messages on a bus share this framing.
- `is_flexible_data_rate`: Boolean. Whether this bus uses CAN FD.
- `host_fault_library`: Boolean. If true, this bus is the primary uplink for fault communication (usually VCAN).

## Node-to-Bus Mapping (per node JSON, under `busses`)
Validated by `node_schema.json`. Maps a firmware node onto hardware peripherals and message lists.

- `peripheral`: Hardware peripheral identifier. Allowed values: `CAN1`, `CAN2`, `FDCAN1`, `FDCAN2`, `FDCAN3`.
- `accept_all_messages`: Boolean. If true, disables hardware filter optimization (promiscuous mode).
- `tx`: Array of TX message definitions for this node on this bus.
- `rx`: Array of RX message definitions for this node on this bus.

## Node Attributes
- `node_name`: Name of the node. Must be unique.
- `fault_library_enabled`: Required boolean. If true, this node participates in the FIDR fault sync system.
- `faults`: (Optional) Array of fault definitions specific to this node.

## CAN Message Attributes (TX)
- `msg_name`: Unique identifier.
- `msg_desc`: Short description of message purpose.
- `msg_priority`: [0-5]. See priority convention below.
- `msg_period`: Optional. Transmission period in ms. Omit or use `0` for event-triggered (non-periodic) messages.
- `byte_order`: Optional. `little_endian` (default) or `big_endian` for multi-byte signal packing within the frame.
- `msg_id_override`: Optional. Manual ID (supports hex `0x...`). Overrides automatic linking. Must respect the bus framing limits.

## RX Message Attributes
- `msg_name`: Name of the message to receive (must exist on the bus).
- `callback`: Boolean. If true, generates a weak-linked callback function in the driver.
- `irq`: Boolean. If true, processes the message in the RX interrupt context.

## Signal Attributes
- `sig_name`: Signal name. Must be unique within the message.
- `sig_desc`: (Optional) Short description of signal purpose.
- `type`: C-type (`uint8_t`...`double`) or a custom type defined in `configs/system/common_types.json`.
- `length`: (Optional for standard types) Bit-length. Required for custom packing or sub-byte types.
- `unit`: (Optional) Physical unit label for DBC generation (e.g., `V`, `Amps`, `C`).
- `scale`: Multiplier for physical value conversion. Default 1.0.
- `offset`: Optional. Linear offset applied during physical conversion. Default 0.0.
- `min`: (Optional) Minimum theoretical value.
- `max`: (Optional) Maximum theoretical value.
- `choices`: (Optional) List of strings for enum-like labels in DBC.

> [!NOTE]
> If `scale` is present, `unit` is required by the schema.

> [!TIP]
> **Scaling Constants:** The library generates `static constexpr float` constants for every signal with a scale != 1.0.
> format: `PACK_COEFF_<MSG>_<SIG>` and `UNPACK_COEFF_<MSG>_<SIG>`. Use these to avoid magic numbers in your application code.

### Message Priority
Lower = higher priority.
Range: [0-5].
PER vehicle convention:

0.  event based, safety-critical
	- fault events, charge commands
1. periodic safety-critical vehicle operation
	- Motor commands, fault sync, charging
2. periodic torque path relevant
	- throttle, torque vectoring commands, steering angle
3. non torque path vehicle operation
	- cooling commands, daq log enable
4. Low frequency (<5HZ) periodic telemetry
	- battery voltage
5. High frequency (>5HZ) "best effort" telemetry
	- IMU raw data, shock pots, battery current

## External Nodes
External nodes (non-PER devices such as chargers or inverters) are defined in `configs/external_nodes/` and validated by `external_node_schema.json`.

### Attributes
- `node_name`: Name of the external device. Must be unique.
- `bus_name`: The logical bus this device sits on (must match a bus in `bus_configs.json`).
- `tx`: Array of messages this device sends toward PER nodes.
- `rx`: Array of messages this device expects to receive.

## Custom Types
Define reusable signal types (enums and aliases) in `configs/system/common_types.json`. Validated by `type_registry.json`.

### Type Attributes
- `name`: Type name. Must end in `_t` (e.g. `car_state_t`).
- `base_type`: One of `uint8_t` ... `uint64_t`, `int8_t` ... `int64_t`, `bool`.
- `choices`: (Optional) Enum value list, generated as a C `enum`.

## Fault Configuration
Node-specific faults are defined directly in the node JSON under the `"faults"` key. The library automatically generates bitfield-sync and event messages for fault communication based on these definitions.

### Node Level
- `generate_fault_messages`: Boolean. If true, generates LCD string arrays for the node (usually only true for nodes with displays like the Dashboard).

### Fault Attributes
- `fault_name`: Unique name within the node.
- `min`: Minimum healthy value. Triggers fault if value < min.
- `max`: Maximum healthy value. Triggers fault if value >= max.
- `priority`: Impact of the fault (`warning`, `error`, `fatal`).
- `time_to_latch`: Time in ms the condition must persist before the fault is latched.
- `time_to_unlatch`: Time in ms the condition must be healthy before the fault is cleared.
- `lcd_message`: String text for display on the dashboard or log.
