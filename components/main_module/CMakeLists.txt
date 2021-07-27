cmake_minimum_required(VERSION 3.13)

# Setup Component
set(COMPONENT_NAME MainModule)
set(TARGET_NAME ${COMPONENT_NAME}.elf)
add_executable(${TARGET_NAME})

# Add STM32_HAL Library
target_link_libraries(${TARGET_NAME} STM32_HAL_LIB)
target_link_libraries(STM32_HAL_LIB CMSIS_L432)
target_include_directories(STM32_HAL_LIB PUBLIC hal)

# Add FreeRTOS Library
target_link_libraries(${TARGET_NAME} FREERTOS_LIB)
target_link_libraries(FREERTOS_LIB CMSIS_L432)
target_include_directories(FREERTOS_LIB PUBLIC freertos)

# Find all .c sources in project
file(GLOB_RECURSE glob_sources "*.c")
target_sources(${TARGET_NAME} PRIVATE ${glob_sources} startup_stm32l432xx.s)

# Find directories for '#include'
target_include_directories(${TARGET_NAME} PRIVATE apps hal)

# Linker options
target_link_options(${TARGET_NAME} PUBLIC 
    -T${CMAKE_CURRENT_SOURCE_DIR}/STM32L432KCUx_FLASH.ld
)

postbuild_target(${COMPONENT_NAME})