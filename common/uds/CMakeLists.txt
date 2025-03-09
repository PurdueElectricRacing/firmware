MACRO(MAKE_UDS_COMMON_LIBRARY lib_name lib_link, lib_libs)
    set(TARGET_NAME ${lib_name})
    add_library(${TARGET_NAME})

    # Find all .c sources in project
    file(GLOB_RECURSE glob_sources "*.c")
    target_sources(${TARGET_NAME} PRIVATE ${glob_sources})

    # Find directories for '#include'
    # For libraries, these directories are all referenced to the top level firmware directory, CMAKE_SOURCE_DIR
    target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR})

    target_link_libraries(${TARGET_NAME} ${lib_link})
    target_link_libraries(${TARGET_NAME} ${lib_libs})
ENDMACRO(MAKE_UDS_COMMON_LIBRARY)

MAKE_UDS_COMMON_LIBRARY(UDS_F407 CMSIS_F407 BOOTLOADER_F407)
MAKE_UDS_COMMON_LIBRARY(UDS_F732 CMSIS_F732 BOOTLOADER_F732)
