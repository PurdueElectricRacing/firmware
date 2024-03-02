set(TARGET_NAME W5500)
add_library(${TARGET_NAME})

set(LIB_DIR ${CMAKE_SOURCE_DIR}/common/modules/Wiznet)

# Find all .c sources in project
# Only using Ethernet for now
file(GLOB_RECURSE glob_sources ${LIB_DIR}/W5500/Ethernet/*.c)
target_sources(${TARGET_NAME} PRIVATE ${glob_sources})

# Find directories for '#include'
# Only using Ethernet for now
RECURSE_DIRECTORIES(${LIB_DIR} "W5500/Ethernet/*.h" include_dirs)
message("${include_dirs}")

target_include_directories(${TARGET_NAME} PUBLIC ${include_dirs})
