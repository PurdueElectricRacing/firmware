cmake_minimum_required(VERSION 3.13)

set(TARGET_NAME Pedal.elf)
add_executable(${TARGET_NAME})

# Add CMSIS Library
target_link_libraries(${TARGET_NAME} CMSIS_L432)

# Find all .c sources in project
file(GLOB_RECURSE glob_sources "*.c")
target_sources(${TARGET_NAME} PRIVATE ${glob_sources} startup_stm32l432xx.s)

# Find directories for '#include'
target_include_directories(${TARGET_NAME} 
    PRIVATE pedals
)

# GCC compile options specific to this target
target_compile_options(${TARGET_NAME} PRIVATE
        -mcpu=cortex-m4
        -mthumb
        -mfpu=fpv4-sp-d16
        -mfloat-abi=hard

        -fdata-sections
        -ffunction-sections
        -ffreestanding

        -Wall
)

# Linker options specific to this target
target_link_options(${TARGET_NAME} PRIVATE 
    -T${CMAKE_CURRENT_SOURCE_DIR}/STM32L432KCUx_FLASH.ld
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
    --specs=nosys.specs
)