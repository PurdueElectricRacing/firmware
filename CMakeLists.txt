cmake_minimum_required(VERSION 3.13)

# Run DAQ automatic code generation
execute_process(COMMAND python3 ${CMAKE_SOURCE_DIR}/common/daq/generation/generator.py 
                RESULT_VARIABLE ret)
if(ret EQUAL "1")
    message(FATAL_ERROR "Code Generation Failed")
endif()

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
make_cmsis_library(CMSIS_L496 STM32L4xx STM32L496xx ${CMAKE_SOURCE_DIR}/common/STM32CubeL4/Drivers/CMSIS)
# make_stm32_hal_library(STM32_L4_HAL_LIB ${CMAKE_SOURCE_DIR}/common/STM32CubeL4/Drivers/STM32L4xx_HAL_Driver)

# Add Common modules
add_subdirectory(common/psched)
add_subdirectory(common/phal_L4)
add_subdirectory(common/queue)
add_subdirectory(common/eeprom)

# Macro for defining hte common build process across all firmware components
include(cmake/common_component.cmake)

# Add Components
add_subdirectory(source/main_module)
add_subdirectory(source/bootloader_l4)
add_subdirectory(source/l4_testing)
add_subdirectory(source/torque_vector)
add_subdirectory(source/precharge)
add_subdirectory(source/driveline)
add_subdirectory(source/dashboard)
add_subdirectory(source/template)
