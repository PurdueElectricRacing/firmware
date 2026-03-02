# Directory Structure
The codebase represents a real-time distributed system for an FSAE electric vehicle.
- Node source code is located in `source/`
- Shared library code and drivers are located in `common/`

## Build System Context
This codebase is compiled with GCC for STM32 microcontrollers.
- Code generation related to CAN bus communication is in `common/can_library/`.
- The standard is set to C23, so keywords like `constexpr`, `static_assert`, `bool` are available.

## Review Points
- Codestyle is defined in `docs/code_style.md`
- Dynamic memory allocation is FORBIDDEN
- Evaluate potential race conditions in concurrent code, especially in interrupt handlers and shared resources.