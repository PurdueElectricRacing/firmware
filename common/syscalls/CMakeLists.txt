cmake_minimum_required(VERSION 3.13)

set(TARGET_NAME SYSCALLS)
add_library(${TARGET_NAME} STATIC)

# Gather all .c files
file(GLOB_RECURSE glob_sources "*.c")
target_sources(${TARGET_NAME} PRIVATE ${glob_sources})

# Includes
target_include_directories(${TARGET_NAME}
    PUBLIC ${CMAKE_SOURCE_DIR}
)

set(ARCH_FLAGS -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)

target_compile_options(${TARGET_NAME} PRIVATE ${ARCH_FLAGS})
target_link_options(${TARGET_NAME} PRIVATE ${ARCH_FLAGS})

target_compile_options(${TARGET_NAME} PRIVATE -fno-analyzer)
