cmake_minimum_required(VERSION 3.13)


set(COMPONENT_NAME Pedals)
set(TARGET_NAME ${COMPONENT_NAME}.elf)
add_executable(${TARGET_NAME})

# Add CMSIS Library
target_link_libraries(${TARGET_NAME} CMSIS_L432)

# Find all .c sources in project
file(GLOB_RECURSE glob_sources "*.c")
target_sources(${TARGET_NAME} PRIVATE ${glob_sources} startup_stm32l432xx.s)

# Find directories for '#include'
target_include_directories(${TARGET_NAME} PRIVATE pedals)

# Linker options
target_link_options(${TARGET_NAME} PRIVATE 
    -T${COMMON_SOURCE_DIR}/linker/STM32L432KCUx_FLASH.ld
)

postbuild_target(${COMPONENT_NAME})