MACRO(MAKE_BOOTLOADER_COMMON_LIBRARY lib_name lib_link)
    set(TARGET_NAME ${lib_name})
    add_library(${TARGET_NAME} INTERFACE)

    # Find all .c sources in project
    file(GLOB_RECURSE glob_sources "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
    target_sources(${TARGET_NAME} INTERFACE ${glob_sources})

    # Find directories for '#include'
    target_include_directories(${TARGET_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

    target_link_libraries(${TARGET_NAME} INTERFACE ${lib_link} common_defs)
ENDMACRO(MAKE_BOOTLOADER_COMMON_LIBRARY)

MAKE_BOOTLOADER_COMMON_LIBRARY(BOOTLOADER_COMMON_F407 CMSIS_F407)
MAKE_BOOTLOADER_COMMON_LIBRARY(BOOTLOADER_COMMON_F732 CMSIS_F732)
