# PDU

PDU controls and monitors low-voltage power rails and cooling hardware for the car.

## Responsibilities

- Control switchable rails (GPIO outputs).
- Sample ADC channels for rail voltage/current telemetry.
- Scan rail nFAULT lines and report/latch faults.
- Control fan/pump/hxfan outputs and fan PWM duty cycles.
- Publish PDU telemetry and coolant output status over CAN.

## Module Layout

- [`main.c`](main.c)
  - Board init, peripheral init, task definitions, task startup order.
  - No business logic beyond orchestration.

- `state.c` / `state.h`
  - Shared runtime state (`g_pdu_state`) for all PDU logic.

- `switches.c` / `switches.h`
  - Switch command/read API.
  - ADC and mux measurement pipeline.
  - Rail current/voltage conversions.
  - Default-rail enable sequence.

- [`faults.c`](faults/faults.c) / [`faults.h`](faults/faults.h)
  - Table-driven nFAULT polling.
  - One-fault-slot-per-cycle update cadence.
  - Rail fault LED behavior.

- `cooling.c` / `cooling.h`
  - Cooling policy computation.
  - Cooling actuation (switches + fan PWM).
  - Cooling CAN output publication.

- `cooling_bangbang.c` / `cooling_bangbang.h`
  - Optional bang-bang policy backend.
  - Compiled in but disabled by default.

- [`telemetry.c`](telemetry/telemetry.c) / [`telemetry.h`](telemetry/telemetry.h)
  - Power/thermal telemetry CAN publication.
  - Flowrate telemetry CAN publication.

- Hardware adapter modules (kept separate):
  - `led/`
  - `fan_control/`
  - `flow_rate/`

## Runtime State (`g_pdu_state`)

The shared state contains:

- `rail_voltage_mv` for 24V/5V/3V3 measurements.
- `switch_current_ma[]` for sensed currents (switch and rail channels).
- `mux_adc_counts[]` and `next_mux_channel` for mux-sampled inputs.
- `next_rail_fault_index` for staggered rail fault scanning.
- `cooling_command` for requested/applied cooling outputs.

## Task Model

Defined in [`main.c`](main.c):

- `CAN_rx_update` (0 ms)
- `CAN_tx_update` (5 ms)
- `switches_periodic` (15 ms)
- `cooling_periodic` (100 ms)
- `faults_periodic` (100 ms)
- `fault_library_periodic` (100 ms)
- `telemetry_flow_periodic` (200 ms)
- `LED_periodic` (500 ms)
- `telemetry_power_periodic` (500 ms)
- Heartbeat task (LED sweep callback)

## Cooling Backend: Bang-Bang (Disabled by Default)

The bang-bang backend is present for future policy work, but disabled to preserve current behavior.

- Toggle: `COOLING_ENABLE_BANGBANG` in `cooling_bangbang.h`
  - `0` (default): backend is a no-op.
  - `1`: motor temp based bang-bang updates are applied.

Current non-bangbang behavior remains the active policy path.

