set(TARGET_NAME LOG)
add_library(${TARGET_NAME} INTERFACE)

# Find all .c sources in project
file(GLOB_RECURSE glob_sources "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
target_sources(${TARGET_NAME} INTERFACE ${glob_sources})

# Find directories for '#include'
target_include_directories(${TARGET_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})