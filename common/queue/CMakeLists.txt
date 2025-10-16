cmake_minimum_required(VERSION 3.13)

set(TARGET_NAME QUEUE)
add_library(${TARGET_NAME})

# Find all .c sources in project
file(GLOB_RECURSE glob_sources "*.c")
target_sources(${TARGET_NAME} PRIVATE ${glob_sources})

# Public include directory
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR})

# Apply architecture flags (baseline Cortex-M4 + hard float)
set(_ARCH_FLAGS -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
target_compile_options(${TARGET_NAME} PRIVATE ${_ARCH_FLAGS})
target_link_options(${TARGET_NAME} PRIVATE ${_ARCH_FLAGS})
