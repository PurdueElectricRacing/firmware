## CANpiler
The CANpiler is the centralized code generation authority. It handles CAN message generation, fault library, and (soon) daq variable generation.


## Software Architecture
Modeled After the traditional compiler pipeline. Written in python for ease of maintainence and development.

`build.py`: Main entry point for CANpiler. Coordinates the top-level data flow.

`templates/`: Directory containing Jinja2 templates for all generated build artifacts. Decouples the output formatting from the generation logic.


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
- **bxCAN Filter Groups**: Optimizes STM32 bxCAN filter bank usage. It groups messages into 32-bit identifier-mask pairs to maximize throughput while respecting the hardware limit of **14 filter banks per peripheral**.
- **Promiscuous Mode**: Configures all-pass filters for buses where `accept_all_messages` is enabled.
- **Initialization Code**: Generates the bitmasks required for hardware filter initialization.

#### 5. Generation
Produces the final build artifacts from the `SystemContext` using the **Jinja2** templating engine.
- **C23 Standard**: All generated code targets the C23 standard, specifically utilizing `static constexpr` for type-safe constants and identifiers.
- `codegen.py`: Generates node headers with packed bit-field structs and endianness-safe accessors. Includes hardened sign-extension logic for signed signals.
- `dbcgen.py`: Produces deterministic, versioned DBC files using the `cantools` library.
- `faultgen.py`: Injects `FAULT_SYNC` and `FAULT_EVENT` messages into node configurations and generates global fault tracking maps (`fault_data.c/h`).

#### 6. Analysis
`load_calc.py`: Performs post-generation capacity analysis for each bus in the system.
- **Frame Estimation**: Calculates total bits per frame based on protocol overhead (Standard: 47 bits, Extended: 67 bits) and signal DLC.
- **Bit-Stuffing**: Applies a $1.2\times$ factor to account for average bit-stuffing overhead.
- **Health Monitoring**: Reports estimated bus utilization percentage with color-coded alerts (Green < 50%, Yellow < 70%, Red > 70%).

#### Health Monitoring Features
- **Stale-Checking**: The generated firmware includes `CAN_check_stale()`, which marks received messages as stale if they have not been updated within **2.5x** their expected transmission period.
