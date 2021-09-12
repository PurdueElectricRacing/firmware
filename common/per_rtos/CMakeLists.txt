cmake_minimum_required(VERSION 3.13)

set(TARGET_NAME PER_RTOS)
add_library(${TARGET_NAME})

# Find all .c sources in project
file(GLOB_RECURSE glob_sources "*.c")
target_sources(${TARGET_NAME} PRIVATE ${glob_sources})

# Find directories for '#include'
SUBDIRLIST(${CMAKE_CURRENT_LIST_DIR} include_dirs)
target_include_directories(${TARGET_NAME} PUBLIC ${include_dirs} ${CMAKE_CURRENT_LIST_DIR})
