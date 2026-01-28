## Bus Attributes
- `peripheral`: Hardware identifier (e.g., `CAN1`, `CAN2`, `CAN3`).
- `accept_all_messages`: Boolean. If true, disables hardware filter optimization (promiscuous mode).
- `is_extended_id`: Boolean. Set at the bus level. Mixed-ID buses are not supported; all messages on a bus will share this framing.
- `host_fault_library`: Boolean. If true, indicates this bus is the primary uplink for fault communication (usually VCAN).

## Node Attributes
- `node_name`: Name of the node. Must be unique.
- `scheduler`: Target operating system/scheduler (`freertos` or `psched`).
- `faults`: (Optional) Array of fault definitions specific to this node.

## CAN Message Attributes (TX)
- `msg_name`: Unique identifier.
- `msg_desc`: Short description of message purpose.
- `msg_priority`: [0-5]. See priority convention below.
- `msg_period`: Transmission frequency in ms.
- `msg_id_override`: Optional. Manual ID (supports hex `0x...`). Overrides automatic linking. Must respect the bus framing limits.

## RX Message Attributes
- `msg_name`: Name of the message to receive (must exist on the bus).
- `callback`: Boolean. If true, generates a weak-linked callback function in the driver.
- `irq`: Boolean. If true, processes the message in the RX interrupt context.

## Signal Attributes
- `sig_name`: Signal name. Must be unique within the message.
- `sig_desc`: (Optional) Short description of signal purpose.
- `type`: C-type (`uint8_t`...`double`) or a custom type defined in `common_types.json`.
- `length`: (Optional for standard types) Bit-length. Required for custom packing or sub-byte types.
- `unit`: (Optional) Physical unit label for DBC generation (e.g., `V`, `Amps`, `C`).
- `scale`: Multiplier for physical value conversion. Default 1.0.
- `min_val`: (Optional) Minimum theoretical value.
- `max_val`: (Optional) Maximum theoretical value.
- `choices`: (Optional) List of strings for enum-like labels in DBC.

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

## Fault Configuration
Node-specific faults are defined directly in the node JSON under the `"faults"` key. The library automatically generates bitfield-sync and event messages for fault communication based on these definitions.

### Node Level
- `generate_fault_messages`: Boolean. If true, generates LCD string arrays for the node (usually only true for nodes with displays like the Dashboard).

### Fault Attributes
- `fault_name`: Unique name within the node.
- `min`: Minimum healthy value (inclusive). Triggers fault if value < min.
- `max`: Maximum healthy value (exclusive). Triggers fault if value >= max.
- `priority`: Impact of the fault (`warning`, `error`, `fatal`).
- `time_to_latch`: Time in ms the condition must persist before the fault is latched.
- `time_to_unlatch`: Time in ms the condition must be healthy before the fault is cleared.
- `lcd_message`: String text for display on the dashboard or log.
