## Bus Attributes
- `peripheral`: Hardware identifier (e.g., `CAN1`, `CAN2`, `CAN3`).
- `accept_all_messages`: Boolean. If true, disables hardware filter optimization (promiscuous mode).

## Node Attributes
- `node_name`: Name of the node. Must be unique.
- `node_id`: [0-31] Used for generating system-level IDs (DAQ, Fault Sync).
- `scheduler`: Target operating system/scheduler (`freertos` or `psched`).

## CAN Message Attributes (TX)
- `msg_name`: Unique identifier.
- `msg_desc`: Short description of message purpose.
- `msg_priority`: [0-5]. See priority convention below.
- `msg_period`: Transmission frequency in ms.
- `msg_id_override`: Optional. Manual ID (supports hex `0x...`). Overrides automatic linking.
- `is_extended_id`: Boolean. Forces 29-bit frame. Automatically set if ID > 0x7FF.

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
0. uninterruptible critical messages
	- Bootloader
1. safety-critical vehicle operation
	- Motor commands, fault library, charging
2. torque path relevant
	- throttle, torque vectoring commands, steering angle
3. non torque path vehicle operation
	- cooling commands, daq log enable
4. Low frequency periodic telemetry
	- battery voltage
5. High frequency "best effort" telemetry
	- IMU raw data, shock pots, battery current