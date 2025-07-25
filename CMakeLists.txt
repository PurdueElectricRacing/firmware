cmake_minimum_required(VERSION 3.13)

# Run DAQ automatic code generation
execute_process(COMMAND python3 ${CMAKE_SOURCE_DIR}/common/daq/generation/generator.py
                RESULT_VARIABLE ret)
if(ret EQUAL "1")
    message(FATAL_ERROR "CAN Generation Failed")
endif()

# Run automatic code generation for Fault Library
execute_process(COMMAND python3 ${CMAKE_SOURCE_DIR}/common/faults/generation/generator.py
                RESULT_VARIABLE ret)
if(ret EQUAL "1")
    message(FATAL_ERROR "Fault Generation Failed")
endif()

# Check for bootloader generation
if (NOT BOOTLOADER_BUILD)
    message(STATUS "Not building applications for bootloaders.")
else()
    message(STATUS
    "Building applications and bootloaders! "
    "Your application will NOT work if you are NOT using the bootloader download tool.") # You have been warned
endif()

# Setup arm-none-eabi as our compile toolcahin
set(CMAKE_TOOLCHAIN_FILE cmake/toolchain.cmake)

# Setup project
project(PER_FIRMWARE VERSION 1.0 LANGUAGES C ASM)

# Archive to new outut directory
set(PROJECT_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/output)

# Common Variables
set(COMMON_SOURCE_DIR ${CMAKE_SOURCE_DIR}/common)

# Add Libraries
include(cmake/utils.cmake)
include(cmake/FindCMSIS.cmake)
include(cmake/FindSTM32HAL.cmake)

# See cmake/FindCMSIS.cmake and cmake/FindSTM32HAL.cmake for constructing additional libraries
make_cmsis_library(CMSIS_L432 STM32L4xx STM32L432xx ${CMAKE_SOURCE_DIR}/external/STM32CubeL4/Drivers/CMSIS)
make_cmsis_library(CMSIS_L471 STM32L4xx STM32L471xx ${CMAKE_SOURCE_DIR}/external/STM32CubeL4/Drivers/CMSIS)
make_cmsis_library(CMSIS_L496 STM32L4xx STM32L496xx ${CMAKE_SOURCE_DIR}/external/STM32CubeL4/Drivers/CMSIS)
make_cmsis_library(CMSIS_F407 STM32F4xx STM32F407xx ${CMAKE_SOURCE_DIR}/external/STM32CubeF4/Drivers/CMSIS)
make_cmsis_library(CMSIS_F732 STM32F7xx STM32F732xx ${CMAKE_SOURCE_DIR}/external/STM32CubeF7/Drivers/CMSIS)
make_cmsis_library(CMSIS_G474 STM32G4xx STM32G474xx ${CMAKE_SOURCE_DIR}/external/STM32CubeG4/Drivers/CMSIS)

include(cmake/FindFreeRTOS.cmake)

# Add Common modules
add_subdirectory(common/phal_F4_F7)
add_subdirectory(common/phal_L4)
add_subdirectory(common/phal_G4)
add_subdirectory(common/psched)
add_subdirectory(common/freertos)
add_subdirectory(common/bootloader)
add_subdirectory(common/queue)
add_subdirectory(common/common_defs)
add_subdirectory(common/log)
add_subdirectory(common/faults)
add_subdirectory(common/daq)
add_subdirectory(common/amk)
add_subdirectory(common/modules/wheel_speeds)
add_subdirectory(common/syscalls)

# Properties that are set for each firmware component
# Used to generate the .elf and other files for each project
define_property(TARGET PROPERTY COMPONENT_NAME
    BRIEF_DOCS "Friendly name of the firmware component with no file extensions"
    FULL_DOCS "USed in file formatting and other message outputs")
define_property(TARGET PROPERTY COMPONENT_DIR
    BRIEF_DOCS "Source code directroy for the component"
    FULL_DOCS "Relative directory from the root firmware directroy for the component source")
define_property(TARGET PROPERTY LINKER_SCRIPT
    BRIEF_DOCS "Linker script used for component."
    FULL_DOCS "Linker script used from ./common/linker/ directory.")
define_property(TARGET PROPERTY COMMON_LIBS
    BRIEF_DOCS "List of common libraries to link against for the target"
    FULL_DOCS "String list with each element seperated by a ;")
define_property(TARGET PROPERTY OUTPUT_DIR
    BRIEF_DOCS "Archive directory override"
    FULL_DOCS "Change where the generated files are archvied to for the target")

# Macro for defining the common build process across all firmware components
include(cmake/common_component.cmake)

# Make MODULES overridable from the command line
set(MODULES "" CACHE STRING "Semicolon-separated list of source modules to build")

if(MODULES STREQUAL "")
    set(MODULES
        a_box
        bootloader
        dashboard
        daq
        f4_testing
        f7_testing
        g4_testing
        l4_testing
        main_module
        pdu
        torque_vector
    )
endif()


# Loop over and add subdirectories
foreach(MODULE ${MODULES})
    message(STATUS "Adding module: ${MODULE}")
    add_subdirectory(source/${MODULE})
endforeach()

# Deprecated Components
# add_subdirectory(source/template)
