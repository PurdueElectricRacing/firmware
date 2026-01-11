## CANpiler
The CANpiler is the centralized code generation authority. It handles CAN message generation, fault library, and (soon) daq variable generation.


## Software Architecture
Modeled After the traditional compiler pipeline. Written in python for ease of maintainence and development.

`build.py`: Main entry point for CANpiler. Coordinates the top-level data flow.


#### 1. Schema Validation
`validator.py`: Catches syntax errors in the configuration files using `json_schema` library.
- typos, missing fields, etc

#### 2. Parsing and Semantic Validation
`parser.py`: Parses configurations into IR `@dataclasses`. "Logical" config errors are caught during this phase.
- Invalid types, signals exceeding 64 bits, etc.
- Aggregates all data into a unified `SystemContext` for subsequent stages.

#### 3. Linker
`linker.py`: Transforms the IR into a linked IR with resolved message IDs using a two-pass "Water Level" algorithm.
- **Pass 1: Constraint Verification**: Ensures manual `msg_id_override` values respect priority monotonicity (messages in priority $N$ cannot have IDs higher than messages in priority $N+1$).
- **Pass 2: ID Assignment**: Assigns remaining dynamic IDs priority-by-priority.
    - Within each priority group, messages are sorted alphabetically by `msg_name` to ensure deterministic builds.
    - The "Water Level" tracks the next available ID; it incrementially rises as IDs are assigned and skips over reserved override ranges.
- **Dependency Resolution**: Resolves RX message references and calculates bit-level layout (offsets, shifts, masks).

#### 4. Mapper
`mapper.py`: Maps CAN IDs to physical hardware resources.
- **bxCAN Filter Groups**: Optimizes STM32 bxCAN filter bank usage by grouping messages into 32-bit filter slots.
- **Promiscuous Mode**: Configures all-pass filters for buses where `accept_all_messages` is enabled.
- **Initialization Code**: Generates the bitmasks required for hardware filter initialization.

#### 5. Generation
Produces the final build artifacts from the `SystemContext`.
- `codegen.py`: Generates C headers and source files for each node, including packed bit-field structs and endianness-safe accessors.
- `dbcgen.py`: Produces deterministic, versioned DBC files using the `cantools` library for external telemetry and analysis.
- `faultgen.py`: Generates global fault data maps (`fault_data.c/h`) to bridge CAN messages with the system-wide fault library.
