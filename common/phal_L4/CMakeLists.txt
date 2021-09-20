cmake_minimum_required(VERSION 3.13)

set(TARGET_NAME PHAL_L4)
add_library(${TARGET_NAME})

# Find all .c sources in project
file(GLOB_RECURSE glob_sources "*.c")
target_sources(${TARGET_NAME} PRIVATE ${glob_sources})

# Find directories for '#include'
# For libraries, these directories are all referenced to the top level firmware directory, CMAKE_SOURCE_DIR
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR})

target_link_libraries(${TARGET_NAME} CMSIS_L432)
