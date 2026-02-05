## Dashboard
This node bears the primary responsibility of managing the pedalbox and thus is a torque-path critical board.

Additionally, it hosts the main user interface for the vehicle, displaying telemetry via an LCD and handling user input through buttons.

Notable Files
- `main.c / .h`: Main initialization and application logic for the dashboard node.
- `pedals/ pedals.c / .h`: Pedalbox interface and processing logic. Filtering, fault detection, CAN broadcasting for pedal signals.
- `lcd/`: LCD display driver and UI rendering code.
