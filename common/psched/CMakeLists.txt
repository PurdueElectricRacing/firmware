cmake_minimum_required(VERSION 3.13)

# Set up for L432
set(TARGET_NAME PSCHED)
add_library(${TARGET_NAME})

# Find all .c sources in project
file(GLOB_RECURSE glob_sources "*.c")
target_sources(${TARGET_NAME} PRIVATE ${glob_sources})

# Find directories for '#include'
# For libraries, these directories are all referenced to the top level firmware directory, CMAKE_SOURCE_DIR
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR})

target_link_libraries(${TARGET_NAME} CMSIS_L432)
target_link_libraries(${TARGET_NAME} PROFILING)

# Set up for F407

set(TARGET_NAME PSCHED_F407)
add_library(${TARGET_NAME})

# Find all .c sources in project
file(GLOB_RECURSE glob_sources "*.c")
target_sources(${TARGET_NAME} PRIVATE ${glob_sources})

# Find directories for '#include'
# For libraries, these directories are all referenced to the top level firmware directory, CMAKE_SOURCE_DIR
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR})

target_link_libraries(${TARGET_NAME} CMSIS_F407)

#Set up for F732

set(TARGET_NAME PSCHED_F732)
add_library(${TARGET_NAME})

# Find all .c sources in project
file(GLOB_RECURSE glob_sources "*.c")
target_sources(${TARGET_NAME} PRIVATE ${glob_sources})

# Find directories for '#include'
# For libraries, these directories are all referenced to the top level firmware directory, CMAKE_SOURCE_DIR
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR})

target_link_libraries(${TARGET_NAME} CMSIS_F732)

#Set up for G474

set(TARGET_NAME PSCHED_G474)
add_library(${TARGET_NAME})

# Find all .c sources in project
file(GLOB_RECURSE glob_sources "*.c")
target_sources(${TARGET_NAME} PRIVATE ${glob_sources})

# Find directories for '#include'
# For libraries, these directories are all referenced to the top level firmware directory, CMAKE_SOURCE_DIR
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR})

target_link_libraries(${TARGET_NAME} CMSIS_G474)

set(TARGET_NAME PSCHED_TIM7)
add_library(${TARGET_NAME})

# Find all .c sources in project
file(GLOB_RECURSE glob_sources "*.c")
target_sources(${TARGET_NAME} PRIVATE ${glob_sources})

# Find directories for '#include'
# For libraries, these directories are all referenced to the top level firmware directory, CMAKE_SOURCE_DIR
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR})

target_link_libraries(${TARGET_NAME} CMSIS_L432)
target_compile_definitions(${TARGET_NAME} PUBLIC PSCHED_USE_TIM7=1)
