cmake_minimum_required(VERSION 3.13)

MACRO(MAKE_BOOTLOADER_COMMON_LIBRARY lib_name lib_link)
    set(TARGET_NAME ${lib_name})
    add_library(${TARGET_NAME})

    # Find all .c sources in project
    file(GLOB_RECURSE glob_sources "*.c")
    target_sources(${TARGET_NAME} PRIVATE ${glob_sources})

    # Find directories for '#include'
    # For libraries, these directories are all referenced to the top level firmware directory, CMAKE_SOURCE_DIR
    target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR})

    target_link_libraries(${TARGET_NAME} ${lib_link})
    target_link_libraries(${TARGET_NAME} common_defs)
ENDMACRO(MAKE_BOOTLOADER_COMMON_LIBRARY)

MAKE_BOOTLOADER_COMMON_LIBRARY(BOOTLOADER_COMMON_L432 CMSIS_L432)
MAKE_BOOTLOADER_COMMON_LIBRARY(BOOTLOADER_COMMON_F407 CMSIS_F407)
MAKE_BOOTLOADER_COMMON_LIBRARY(BOOTLOADER_COMMON_F732 CMSIS_F732)
