cmake_minimum_required(VERSION 3.10)

# Setup arm-none-eabi as our compile toolcahin
set (CMAKE_CXX_FLAGS "-nostdlib -nostartfiles")
include(cmake/toolchain.cmake)
set(CMAKE_GENERATOR "Unix Makefiles")

# Setup project 
project(PER_FIRMWARE VERSION 1.0 LANGUAGES C ASM)

# Add Modules
include(cmake/FindCMSIS.cmake)


# Add subdirectiory modules
add_subdirectory(main_module)
add_definitions(-DCMAKE_EXPORT_COMPILE_COMMANDS=ON)
