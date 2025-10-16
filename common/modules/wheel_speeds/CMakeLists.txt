cmake_minimum_required(VERSION 3.13)

# Common architecture flags for L4 cores
set(_ARCH_FLAGS -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)

# L432
set(TARGET_NAME WHEEL_SPEEDS)
add_library(${TARGET_NAME})

file(GLOB_RECURSE glob_sources "*.c")
target_sources(${TARGET_NAME} PRIVATE ${glob_sources})

target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(${TARGET_NAME} CMSIS_L432 PHAL_L432)

target_compile_options(${TARGET_NAME} PRIVATE ${_ARCH_FLAGS})
target_link_options(${TARGET_NAME} PRIVATE ${_ARCH_FLAGS})

# L496
set(TARGET_NAME WHEEL_SPEEDS_L496)
add_library(${TARGET_NAME})

file(GLOB_RECURSE glob_sources "*.c")
target_sources(${TARGET_NAME} PRIVATE ${glob_sources})

target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(${TARGET_NAME} CMSIS_L496 PHAL_L496)

target_compile_options(${TARGET_NAME} PRIVATE ${_ARCH_FLAGS})
target_link_options(${TARGET_NAME} PRIVATE ${_ARCH_FLAGS})
