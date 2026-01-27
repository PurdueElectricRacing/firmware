set(TARGET_NAME ADBMS)
add_library(${TARGET_NAME})

# Find all .c sources in project
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS"${CMAKE_CURRENT_SOURCE_DIR}/*.c")
target_sources(${TARGET_NAME} PRIVATE ${SRC_FILES})

# Find directories for '#include'
# For libraries, these directories are all referenced to the top level firmware directory, CMAKE_SOURCE_DIR
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR})

# target_link_libraries(${TARGET_NAME} "QUEUE;common_defs;PHAL_F407")

