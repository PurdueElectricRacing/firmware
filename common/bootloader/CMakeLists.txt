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

    if(${lib_name} MATCHES "F7")
        set(_ARCH_FLAGS -mthumb -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard)
    elseif(${lib_name} MATCHES "F4" OR ${lib_name} MATCHES "L4" OR ${lib_name} MATCHES "G4")
        set(_ARCH_FLAGS -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
    endif()
    target_compile_options(${TARGET_NAME} PRIVATE ${_ARCH_FLAGS})
    target_link_options(${TARGET_NAME} PRIVATE ${_ARCH_FLAGS})
ENDMACRO(MAKE_BOOTLOADER_COMMON_LIBRARY)

MAKE_BOOTLOADER_COMMON_LIBRARY(BOOTLOADER_COMMON_L432 CMSIS_L432)
MAKE_BOOTLOADER_COMMON_LIBRARY(BOOTLOADER_COMMON_L496 CMSIS_L496)
MAKE_BOOTLOADER_COMMON_LIBRARY(BOOTLOADER_COMMON_F407 CMSIS_F407)
MAKE_BOOTLOADER_COMMON_LIBRARY(BOOTLOADER_COMMON_F732 CMSIS_F732)
