# Directory Structure
The codebase represents a real-time distributed system for an FSAE electric vehicle.
- Node source code is located in `source/`
- Shared library code and drivers are located in `common/`

## Build System Context
- The code is built using CMake, with separate targets for each node and shared libraries.
- The compiler toolchain is `arm-none-eabi-gcc` targeting ARM Cortex-M microcontrollers (STM32).
- Code generation related to CAN bus communication is in `can_library/`.
- The standard is set to C23, so keywords like `constexpr`, `static_assert`, `bool` are available.

## Code Standards
- Codestyle is defined in `docs/code_style.md`
- Dynamic memory allocation is FORBIDDEN
- In-repo documentation should be updated in the same PR as code changes.

## Real-Time Constraints
- Code must be deterministic
- Avoid blocking calls unless explicitly intended
- Minimize ISR execution time

## Concurrency Rules
- Make sure to evaluate interrupt priority inconsistencies that could lead to priority inversion
- Make sure to evaluate for deadlock potential when using multiple locks or critical sections
- Make sure to evaluate for race conditions when accessing shared resources
- Assume preemption between:
  - Main loop
  - Interrupt handlers
  - DMA callbacks
- Shared variables between ISR and main context must:
  - Be marked `volatile`
  - Be protected using atomic operations or critical sections if necessary

## Errors
- Do not silently ignore faults/errors