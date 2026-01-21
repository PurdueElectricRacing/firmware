## Bus Attributes
- `peripheral`: Hardware identifier (e.g., `CAN1`, `CAN2`, `CAN3`).
- `accept_all_messages`: Boolean. If true, disables hardware filter optimization (promiscuous mode).

## Node Attributes
- `node_name`: Name of the node. Must be unique.
- `scheduler`: Target operating system/scheduler (`freertos` or `psched`).

## CAN Message Attributes (TX)
- `msg_name`: Unique identifier.
- `msg_desc`: Short description of message purpose.
- `msg_priority`: [0-5]. See priority convention below.
- `msg_period`: Transmission frequency in ms.
- `msg_id_override`: Optional. Manual ID (supports hex `0x...`). Overrides automatic linking. Must be within bus framing limits (e.g. <= 0x7FF for Standard buses).

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
- `offset`: Manual offset for physical value conversion. Default 0.
- `min_val`: (Optional) Minimum theoretical value.
- `max_val`: (Optional) Maximum theoretical value.
- `choices`: (Optional) List of strings for enum-like labels in DBC.

### Message Priority
Lower = higher priority.
Range: [0-5].
PER vehicle convention:
0. event based, safety-critical
	- fault events, charge commands
1. periodic safety-critical vehicle operation
	- Motor commands, fault sync, charging
2. periodic torque path relevant
	- throttle, torque vectoring commands, steering angle
3. non torque path vehicle operation
	- cooling commands, daq log enable
4. Low frequency periodic telemetry
	- battery voltage
5. High frequency "best effort" telemetry
	- IMU raw data, shock pots, battery current

## Fault Configuration
Internal faults can be defined on a per-node basis. The library automatically generates bitfield-sync and event messages for fault communication.

### Node Level
- `generate_fault_messages`: Boolean. If true, generates LCD string arrays for the node (usually only true for nodes with displays).

### Fault Attributes
- `fault_name`: Unique name within the node.
- `min`: Minimum healthy value (inclusive). Triggers fault if value < min.
- `max`: Maximum healthy value (exclusive). Triggers fault if value >= max.
- `priority`: Impact of the fault (`warning`, `error`, `fatal`).
- `time_to_latch`: Time in ms the condition must persist before the fault is latched.
- `time_to_unlatch`: Time in ms the condition must be healthy before the fault is cleared.
- `lcd_message`: String text for display on the dashboard or log.
