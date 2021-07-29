cmake_minimum_required(VERSION 3.13)

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
include(cmake/FindFREERTOS.cmake)

# Add Components
add_subdirectory(components/main_module)
add_subdirectory(components/pedal)
