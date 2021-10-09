cmake_minimum_required(VERSION 3.13)

# Run DAQ automatic code generation
execute_process(COMMAND python ${CMAKE_SOURCE_DIR}/common/daq/generation/generator.py)

# Setup arm-none-eabi as our compile toolcahin
set(CMAKE_TOOLCHAIN_FILE cmake/toolchain.cmake)
include(cmake/utils.cmake)

# Setup project 
project(PER_FIRMWARE VERSION 1.0 LANGUAGES C ASM)

# Archive to new outut directory
set(PROJECT_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/output)

# Common Variables
set(COMMON_SOURCE_DIR ${CMAKE_SOURCE_DIR}/common)

# Add Libraries
include(cmake/FindCMSIS.cmake)
include(cmake/FindSTM32HAL.cmake)

# See cmake/FindCMSIS.cmake and cmake/FindSTM32HAL.cmake for constructing additional libraries
make_cmsis_library(CMSIS_L432 STM32L4xx STM32L432xx ${CMAKE_SOURCE_DIR}/common/STM32CubeL4/Drivers/CMSIS)
make_stm32_hal_library(STM32_L4_HAL_LIB ${CMAKE_SOURCE_DIR}/common/STM32CubeL4/Drivers/STM32L4xx_HAL_Driver)

# Add Common modules
add_subdirectory(common/psched)
add_subdirectory(common/phal_L4)
add_subdirectory(common/queue)

# Add Components
add_subdirectory(source/main_module)
add_subdirectory(source/pedal)
add_subdirectory(source/bootloader_l4)
add_subdirectory(source/l4_testing)
add_subdirectory(source/torque_vector)
