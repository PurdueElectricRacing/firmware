## Bus Attributes

## Node Attributes
- `node_name`: name of the node. must be unique.
- `scheduler`: use either `freertos` or `psched`.

## CAN Message Attributes
Required:
- `msg_name`: name of the message. must be unique. 
- `msg_desc`: name of 

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