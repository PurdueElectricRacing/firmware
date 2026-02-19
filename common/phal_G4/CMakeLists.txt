cmake_minimum_required(VERSION 3.13)

MACRO(MAKE_PHAL_LIBRARY lib_name lib_link)
    set(TARGET_NAME ${lib_name})
    add_library(${TARGET_NAME} INTERFACE)

    # Find all .c sources in project
    file(GLOB_RECURSE glob_sources "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
    target_sources(${TARGET_NAME} INTERFACE ${glob_sources})

    # Find directories for '#include'
    target_include_directories(${TARGET_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

    target_link_libraries(${TARGET_NAME} INTERFACE ${lib_link} common_defs)
ENDMACRO(MAKE_PHAL_LIBRARY)

MAKE_PHAL_LIBRARY(PHAL_G474 CMSIS_G474)
