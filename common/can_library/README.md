## PER CAN Library
Standardized framework for CAN communication within PER vehicles.

- **`canpiler/`**: Python toolchain for parsing configurations and generating code.
- **`configs/`**: "source of truth" JSON definitions for nodes, buses, and system-wide faults.
- **`generated/`**: Auto-generated C headers and sources updated on every build.
- **`dbc/`**: CAN database (DBC) files for external analysis tools.
- **`schema/`**: JSON schemas for validating configuration files.

**Core Files:**
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