## PER CAN Library
Standardized framework for CAN communication and system-wide fault management within PER vehicles.

- **`canpiler/`**: Jinja2-based Python toolchain for parsing configurations and generating C23-compliant code.
- **`configs/`**: "Source of Truth" definitions for nodes, buses, and faults.
- **`generated/`**: Auto-generated C headers and sources utilizing `static constexpr` for type-safety.
- **`dbc/`**: Deterministic CAN database (DBC) files for telemetry and analysis.

**Core Features:**
- **C23 Standard**: Leverages `static constexpr` for zero-cost, type-safe identifiers.
- **Unified Faults**: Faults are defined directly within node configurations and automatically linked to CAN communications.
- **Hardened Packing**: Endianness-aware bit-packing with explicit sign-extension protection.
- **Water Level Linker**: Deterministic ID assignment with priority monotonicity enforcement.
- `can_common.h / .c`: Shared hardware abstraction and logic.
- `can_library.cmake`: CMake integration and node library generation.

## Usage
1. Define your CAN network in `common/can_library/configs/` using the provided JSON schemas.
2. Add to `COMMON_LIBRARIES` of your target: `can_node_<node_name>`.
3. Define your RX interrupt handlers to call `CAN_handle_irq(CAN_TypeDef *bus, uint8_t fifo)`

> [!NOTE]
> Weird quirk: we run the CANpiler twice.
> Once during CMake configuration time and one during build time.
> This avoids any possibility of building with stale generated files.

## CAN Common


## Fault Common