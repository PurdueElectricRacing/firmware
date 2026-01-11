## CANpiler
The CANpiler is the centralized code generation authority. It handles CAN message generation, fault library, and (soon) daq variable generation.


## Software Architecture
Modeled After the traditional compiler pipeline. Written in python for ease of maintainence and development.

`build.py`: Main entry point for CANpiler. Coordinates the top-level data flow.


#### 1. Schema Validation
`validator.py`: Catches syntax errors in the configuration files using `json_schema` library.
- typos, missing fields, etc

#### 2. Parsing and Semantic Validation
`parser.py`: Parses the configuration files into IR `@dataclasses`. "Logical" config errors are caught during this phase.
- Invalid types, signals contains more information than 64 bits, etc

#### 3. Linker
`linker.py`: Transforms the IR into a linked IR with resolved message IDs.
The linker algorithm is as follows:
1. Set "anchors" for each priority group using message ID overrides. If a priority group does not contain any messages with overrides, use the first item in sorted list.
2. "Fill" each priority group by assigning an ID in incrementing order
- A collision is when the algorithm assigns two messages the same ID. When this happens a valid bus cannot be generated with given the constraints, so the linker will throw an exception

#### 4. Mapper


#### 5. Generation