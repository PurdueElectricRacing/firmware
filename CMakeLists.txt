cmake_minimum_required(VERSION 3.13)

# Setup arm-none-eabi as our compile toolcahin
include(cmake/toolchain.cmake)

# Setup project 
project(PER_FIRMWARE VERSION 1.0 LANGUAGES C ASM)

# Archive to new outut directory
set (EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/output)
set (LIBRARY_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH})

# Add Libraries
include(cmake/FindCMSIS.cmake)

# Add Components
add_subdirectory(components/main_module)
add_subdirectory(components/pedal)

# Eventually use this file for VSCODE code completion
add_definitions(-DCMAKE_EXPORT_COMPILE_COMMANDS=ON)
