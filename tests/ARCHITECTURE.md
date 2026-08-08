# Host tests

The host test suite uses GoogleTest and CTest for firmware unit tests and firmware-to-DAQApp CAN contract tests. CMake builds the C and C++ targets, generates current CAN artifacts, and builds the Rust bridge used by the integration tests.

## Requirements

- Python 3.11 or newer
- CMake 3.21 or newer
- A C23 and C++17 host compiler
- Cargo and a Rust toolchain
- Network access for initial dependency downloads

## Running the tests

Run tests through `per_build.py` from the repository root:

| Command | Action |
| --- | --- |
| `python3 per_build.py tests` | Build and run every host test |
| `python3 per_build.py tests unit` | Build and run unit tests |
| `python3 per_build.py tests integration` | Build and run CAN contract integration tests |
| `python3 per_build.py tests --sanitizers` | Build and run every host test with AddressSanitizer and UBSan |
| `python3 per_build.py tests unit --sanitizers` | Build and run unit tests with AddressSanitizer and UBSan |

`per_build.py` configures `tests/CMakeLists.txt`, builds the selected test layer, and runs CTest with failure output enabled. Build artifacts are stored in `firmware/build/host-tests`.

The same steps can be run directly:

```sh
cmake -S tests -B firmware/build/host-tests \
  -DPER_TEST_LAYER=all \
  -DPER_TEST_SANITIZERS=OFF
cmake --build firmware/build/host-tests
ctest --test-dir firmware/build/host-tests --output-on-failure
```

`PER_TEST_LAYER` accepts `all`, `unit`, or `integration`.

## Existing tests

### Firmware unit tests

Unit tests live under `tests/unit/firmware`:

- `lerp_lut_test.cpp` covers exact lookup points, interpolation, and upper and lower clamping in `firmware/common/lerp_lut/lerp_lut.c`.
- `can_codec_test.cpp` covers payload loading and storage, byte swapping, signal packing and unpacking, sign extension, and float bit conversion in `firmware/can_library/can_codec.h`. A C23 shim ensures these header-only inline functions are compiled as C rather than as part of the C++17 GoogleTest translation unit.

`tests/cmake/FirmwareUnitTest.cmake` provides `add_firmware_unit_test`. It configures production C sources as C23 static libraries, test sources as C++17, strict compiler warnings, GoogleTest discovery, the CTest `unit` label, and optional sanitizers.

### CAN contract integration tests

`tests/integration/can_contract_test.cpp` contains the CAN contract tests:

- A generated firmware `main_hb` transmission is converted to the production `timestamped_frame_t` layout, parsed by DAQApp's UDP parser, and decoded using DAQApp's CAN dependencies.
- A `start_button` frame is encoded and looped back through DAQApp, dispatched through the generated firmware `CAN_rx_dispatcher`, and verified through firmware CAN data and its receive timestamp.
- An unknown CAN message is dispatched while the existing firmware `start_button` state and timestamp are verified as preserved.

The integration bridges are grouped by component:

- `tests/integration/bridges/firmware` contains the C adapter and host declarations for the FreeRTOS and PHAL types used by generated firmware CAN code.
- `tests/integration/bridges/daqapp` contains the C-compatible bridge interface and Rust `cdylib`. The Rust bridge includes DAQApp's `connection.rs` and `can/driver.rs` implementations.

The integration CMake target performs these build steps:

1. Run `firmware/can_library/canpiler/build.py` to generate current firmware CAN headers and DBC files.
2. Validate the Rust bridge dependencies against DAQApp with `tests/cmake/validate_bridge_dependencies.py`.
3. Build the Rust bridge with Cargo into the host-test build directory.
4. Build and register `daqapp_integration_test` under the CTest `integration` label.

## Directory layout

```text
tests/
├── ARCHITECTURE.md
├── CMakeLists.txt
├── cmake/
│   ├── FirmwareUnitTest.cmake
│   └── validate_bridge_dependencies.py
├── unit/
│   ├── CMakeLists.txt
│   └── firmware/
│       ├── can_codec_test.cpp
│       └── lerp_lut_test.cpp
└── integration/
    ├── CMakeLists.txt
    ├── can_contract_test.cpp
    └── bridges/
        ├── daqapp/
        │   ├── daqapp_bridge.hpp
        │   └── rust/
        │       ├── Cargo.toml
        │       ├── Cargo.lock
        │       └── src/lib.rs
        └── firmware/
            ├── firmware_can_bridge.c
            ├── firmware_can_bridge.h
            └── stubs/
```

## Adding a unit test

1. Add a GoogleTest source file under `tests/unit/firmware`.
2. Register a target in `tests/unit/CMakeLists.txt` with `add_firmware_unit_test`:

   ```cmake
   add_firmware_unit_test(
       NAME example_test
       SOURCES "${CMAKE_CURRENT_LIST_DIR}/../../firmware/common/example/example.c"
       TEST_SOURCES "${CMAKE_CURRENT_LIST_DIR}/firmware/example_test.cpp"
       INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/../../firmware/common/example"
   )
   ```

3. Use `SOURCES` for production `.c` files. For header-only C modules, add a `.c` shim to `SOURCES` and call it from the C++ test so inline implementation code is compiled under C23 rather than C++17.
4. Run `python3 per_build.py tests unit --sanitizers`.

CTest discovers each GoogleTest case from the registered target automatically.

## Adding an integration test

1. Add the GoogleTest case to `tests/integration/can_contract_test.cpp`.
2. Add firmware-facing bridge functions to `tests/integration/bridges/firmware/firmware_can_bridge.c` and its header.
3. Add DAQApp-facing bridge functions to `tests/integration/bridges/daqapp/rust/src/lib.rs` and declare the C ABI in `tests/integration/bridges/daqapp/daqapp_bridge.hpp`.
4. Add host declarations required by generated firmware code under `tests/integration/bridges/firmware/stubs`.
5. Add new bridge source files and include directories to `tests/integration/CMakeLists.txt`.
6. Add matching DAQApp dependencies to the Rust bridge manifest and update its lockfile when the bridge uses another crate.
7. Run `python3 per_build.py tests integration`, followed by `python3 per_build.py tests`.
