find_package(Python3 3.11 REQUIRED COMPONENTS Interpreter)

set(PER_ROOT "${PROJECT_SOURCE_DIR}/..")
set(DAQAPP_BRIDGE_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/bridges/daqapp/rust")
set(DAQAPP_BRIDGE_TARGET_DIR "${CMAKE_BINARY_DIR}/cargo")

if(WIN32)
  set(DAQAPP_RUST_BRIDGE "${DAQAPP_BRIDGE_TARGET_DIR}/debug/daqapp_bridge.dll")
  if(MSVC)
    set(DAQAPP_RUST_BRIDGE_IMPLIB "${DAQAPP_BRIDGE_TARGET_DIR}/debug/daqapp_bridge.dll.lib")
  else()
    set(DAQAPP_RUST_BRIDGE_IMPLIB "${DAQAPP_BRIDGE_TARGET_DIR}/debug/libdaqapp_bridge.dll.a")
  endif()
elseif(APPLE)
  set(DAQAPP_RUST_BRIDGE "${DAQAPP_BRIDGE_TARGET_DIR}/debug/libdaqapp_bridge.dylib")
elseif(UNIX)
  set(DAQAPP_RUST_BRIDGE "${DAQAPP_BRIDGE_TARGET_DIR}/debug/libdaqapp_bridge.so")
else()
  message(FATAL_ERROR "Unsupported host platform")
endif()

add_custom_target(generate_can_library
  COMMAND "${Python3_EXECUTABLE}" "${PER_ROOT}/firmware/can_library/canpiler/build.py"
  WORKING_DIRECTORY "${PER_ROOT}"
  COMMENT "Generating firmware CAN headers and DBC files"
  VERBATIM
)

set(_bridge_byproducts "${DAQAPP_RUST_BRIDGE}")
if(WIN32)
  list(APPEND _bridge_byproducts "${DAQAPP_RUST_BRIDGE_IMPLIB}")
endif()
add_custom_target(daqapp_rust_bridge_build
  COMMAND "${Python3_EXECUTABLE}"
    "${PROJECT_SOURCE_DIR}/cmake/validate_bridge_dependencies.py"
    --daqapp "${PER_ROOT}/daqapp"
    --bridge "${DAQAPP_BRIDGE_SOURCE}"
  COMMAND "${CMAKE_COMMAND}" -E env
    "CARGO_TARGET_DIR=${DAQAPP_BRIDGE_TARGET_DIR}"
    cargo build --locked --manifest-path "${DAQAPP_BRIDGE_SOURCE}/Cargo.toml"
  BYPRODUCTS ${_bridge_byproducts}
  WORKING_DIRECTORY "${PER_ROOT}"
  COMMENT "Building the DAQApp Rust test bridge"
  VERBATIM
)

add_library(daqapp_rust_bridge SHARED IMPORTED GLOBAL)
set_target_properties(daqapp_rust_bridge PROPERTIES
  IMPORTED_LOCATION "${DAQAPP_RUST_BRIDGE}"
)
if(WIN32)
  set_target_properties(daqapp_rust_bridge PROPERTIES
    IMPORTED_IMPLIB "${DAQAPP_RUST_BRIDGE_IMPLIB}"
  )
endif()

add_executable(daqapp_integration_test
  bridges/firmware/firmware_can_bridge.c
  can_contract_test.cpp
)
add_dependencies(daqapp_integration_test generate_can_library daqapp_rust_bridge_build)
target_include_directories(daqapp_integration_test PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/bridges/daqapp"
  "${CMAKE_CURRENT_SOURCE_DIR}/bridges/firmware"
  "${CMAKE_CURRENT_SOURCE_DIR}/bridges/firmware/stubs"
  "${PER_ROOT}/firmware"
  "${PER_ROOT}"
)
target_compile_definitions(daqapp_integration_test PRIVATE
  STM32G474xx
  PROJECT_ROOT="${PER_ROOT}"
)
target_link_libraries(daqapp_integration_test PRIVATE GTest::gtest_main daqapp_rust_bridge)
set_target_properties(daqapp_integration_test PROPERTIES
  C_STANDARD 23
  C_STANDARD_REQUIRED YES
  C_EXTENSIONS OFF
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED YES
  CXX_EXTENSIONS OFF
)
target_compile_options(daqapp_integration_test PRIVATE
  $<$<COMPILE_LANG_AND_ID:C,GNU,Clang,AppleClang>:-Wall;-Wextra;-Wpedantic;-Werror>
  $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wall;-Wextra;-Wpedantic;-Werror>
)
if(PER_TEST_SANITIZERS AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
  target_compile_options(daqapp_integration_test PRIVATE
    -fsanitize=address,undefined
    -fno-omit-frame-pointer
  )
  target_link_options(daqapp_integration_test PRIVATE -fsanitize=address,undefined)
endif()

if(WIN32)
  add_custom_command(TARGET daqapp_integration_test POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
      "${DAQAPP_RUST_BRIDGE}"
      "$<TARGET_FILE_DIR:daqapp_integration_test>"
  )
else()
  set_target_properties(daqapp_integration_test PROPERTIES
    BUILD_RPATH "${DAQAPP_BRIDGE_TARGET_DIR}/debug"
  )
endif()

gtest_discover_tests(daqapp_integration_test PROPERTIES LABELS integration)
