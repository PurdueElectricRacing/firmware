cmake_minimum_required(VERSION 3.10)

# Setup arm-none-eabi as our compile toolcahin
include(toolchain.cmake)
set(CMAKE_GENERATOR make)

# Setup project 
project(PER_FIRMWARE VERSION 1.0)

# Add subdirectiory modules
add_subdirectory(main)
