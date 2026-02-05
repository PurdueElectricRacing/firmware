cmake_minimum_required(VERSION 3.13)

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
make_cmsis_library(CMSIS_F407 STM32F4xx STM32F407xx ${CMAKE_SOURCE_DIR}/external/STM32CubeF4/Drivers/CMSIS)
make_cmsis_library(CMSIS_F732 STM32F7xx STM32F732xx ${CMAKE_SOURCE_DIR}/external/STM32CubeF7/Drivers/CMSIS)
make_cmsis_library(CMSIS_G474 STM32G4xx STM32G474xx ${CMAKE_SOURCE_DIR}/external/STM32CubeG4/Drivers/CMSIS)

include(cmake/FindFreeRTOS.cmake)

# Include new CAN Library
include(common/can_library/can_library.cmake)

# Function to apply analyzer flags to targets based on type
function(apply_analyzer_to_directory DIR_PATH TARGET_TYPE_FILTER)
    get_property(_targets DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${DIR_PATH}
                PROPERTY BUILDSYSTEM_TARGETS)
    foreach(TGT IN LISTS _targets)
        get_target_property(TGT_TYPE ${TGT} TYPE)
        if(TGT_TYPE MATCHES "${TARGET_TYPE_FILTER}")
            target_compile_options(${TGT} PRIVATE -fanalyzer -Wno-analyzer-infinite-loop)
        endif()
    endforeach()
endfunction()

set(COMMON_MODULES
    common/phal_F4_F7
    common/phal_G4
    common/psched
    common/freertos
    common/bootloader
    common/queue
    common/common_defs
    common/log
    common/amk
    common/strbuf
    common/syscalls
)

foreach(COMMON_MODULE IN LISTS COMMON_MODULES)
    add_subdirectory(${COMMON_MODULE})
    if(USE_FANALYZER)
        apply_analyzer_to_directory(${COMMON_MODULE} "STATIC_LIBRARY|SHARED_LIBRARY|MODULE_LIBRARY")
    endif()
endforeach()

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
        f4_testing
        g4_testing
        a_box
        dashboard
        daq
        driveline
        main_module
        pdu
        torque_vector
        bms_test
    )
endif()


# Loop over and add source modules
foreach(MODULE ${MODULES})
    message(STATUS "Adding module: ${MODULE}")
    add_subdirectory(source/${MODULE})
    if(USE_FANALYZER)
        apply_analyzer_to_directory(source/${MODULE} "EXECUTABLE")
    endif()
endforeach()

# Deprecated Components
# add_subdirectory(source/template)
