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

> [!NOTE]
> Weird quirk: we run the CANpiler twice.
> Once during CMake configuration time and one during build time.
> This avoids any possibility of building with stale generated files.