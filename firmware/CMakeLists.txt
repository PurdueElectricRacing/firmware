cmake_minimum_required(VERSION 3.10)

# Setup arm-none-eabi as our compile toolcahin
set (CMAKE_CXX_FLAGS "-nostdlib -nostartfiles")
include(cmake/toolchain.cmake)
set(CMAKE_GENERATOR "Unix Makefiles")

# Setup project 
project(PER_FIRMWARE VERSION 1.0 LANGUAGES C ASM)





find_library(stm32l4_cmsis "./cmake")

# Add subdirectiory modules
add_subdirectory(main)
