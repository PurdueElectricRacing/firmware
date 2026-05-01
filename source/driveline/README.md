## Driveline
There are two nearly-identical driveline nodes on the vehicle. Front and rear.

Driveline nodes are reponsible for interfacing with sensors, performing unit conversions, and publishing the telemetry to the CAN bus.

[`main.c`](main.c) implements the main logic, while any configuration/harnessing differences are specified in `config.h`.

The front and rear nodes are built from the same source code, with CMake using different preprocessor definitions at compile time to generate two separate binaries