## Heartbeat Module
This module standardizes the handling of status LEDs.
- All low-voltage boards with MCUs have the same status LED connections (heartbeat, connection, error)
- The heartbeat LED (blue) should blink at a steady rate to indicate the system is alive and running.
- The connection LED (amber) should remain on when the system is actively receiving data from the CAN bus, and off otherwise.
- The error LED (red) turns on when the system detects a fault condition, and off otherwise.
