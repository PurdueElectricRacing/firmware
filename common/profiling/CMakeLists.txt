cmake_minimum_required(VERSION 3.13)

MACRO(MAKE_PROFILING_LIB LIB_NAME CMSIS_TARGET)
    set(TARGET_NAME ${LIB_NAME})
    add_library(${TARGET_NAME} STATIC)

    # Find all .c sources in this directory
    file(GLOB glob_sources "*.c")
    target_sources(${TARGET_NAME} PRIVATE ${glob_sources})

    # Find directories for '#include'
    target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR})
    
    # Link to the appropriate CMSIS library for MCU-specific headers
    target_link_libraries(${TARGET_NAME} PUBLIC ${CMSIS_TARGET})
ENDMACRO()

# Create profiling libraries for each MCU family
MAKE_PROFILING_LIB(PROFILING_F407 CMSIS_F407)
MAKE_PROFILING_LIB(PROFILING_F732 CMSIS_F732)
MAKE_PROFILING_LIB(PROFILING_G474 CMSIS_G474)