
# Create an executiable with the following features:
# The provided target must have the required properties outlined in the root CMakeLists.txt directory
# The resulting binary will have the following:
#   All .c files are added from the root and child directories of "_COMPONENT_DIR"
#   All .h files in root and child directories of "_COMPONENT_DIR" are included without prefix
#       - a file like "/source/dashboard/apps/apps.h" can be included like #include "apps.h"
#   All CMAKE libraries specified in "COMMON_LIBS" are linked, defaults to L432 common libs
#   Linked using the "LINKER_SCRIPT" file, defaults to L432
MACRO(COMMON_FIRMWARE_COMPONENT TARGET_NAME)

    # Setup Component name based on directory
    get_target_property(_COMPONENT_NAME ${TARGET_NAME} COMPONENT_NAME)
    get_target_property(_COMPONENT_DIR  ${TARGET_NAME} COMPONENT_DIR)
    get_target_property(_LINKER_SCRIPT  ${TARGET_NAME} LINKER_SCRIPT)
    get_target_property(_COMMON_LIBS    ${TARGET_NAME} COMMON_LIBS)

    # Default values for target props.
    if (NOT _LINKER_SCRIPT)
        message(STATUS "Missing Linker Script definition. Defaulting to STM32L432")
        set(_LINKER_SCRIPT "STM32L432KCUx_FLASH")
    endif()
    if (NOT _COMMON_LIBS)
        set(_COMMON_LIBS "CMSIS_L432;PHAL_L432;PSCHED;QUEUE;FAULTS")
    endif()

    # Add Common libraries
    foreach (_LIB_NAME IN ITEMS ${_COMMON_LIBS})
        target_link_libraries(${TARGET_NAME} ${_LIB_NAME})
    endforeach()
    target_link_libraries(${TARGET_NAME} common_defs)
    target_link_libraries(${TARGET_NAME} SYSCALLS)

    target_link_options(${TARGET_NAME} PRIVATE
        "-Wl,--whole-archive"
        "$<TARGET_FILE:SYSCALLS>"
        "-Wl,--no-whole-archive"
    )

    # Find all .c sources in project, recursive search starting at component root
    file(GLOB_RECURSE glob_sources ${_COMPONENT_DIR}/*.c)
    # Exclude if it is a test file
    list(FILTER glob_sources EXCLUDE REGEX "test_.*")
    # Exclude starter files (templates used for code generation)
    list(FILTER glob_sources EXCLUDE REGEX ".*starter.*")
    target_sources(${TARGET_NAME} PUBLIC ${glob_sources})

    # Find directories for '#include'
    SUBDIRLIST(${_COMPONENT_DIR} include_dirs)
    target_include_directories(${TARGET_NAME} PUBLIC ${include_dirs} ${_COMPONENT_DIR})

    # Linker options
    if(BOOTLOADER_BUILD)
        target_link_options(${TARGET_NAME} PUBLIC
            -T${COMMON_SOURCE_DIR}/linker/${_LINKER_SCRIPT}_APP.ld
        )
    else()
        target_link_options(${TARGET_NAME} PUBLIC
            -T${COMMON_SOURCE_DIR}/linker/${_LINKER_SCRIPT}.ld
        )
    endif()
    
    set(_STM32_FAMILY "")
    if(_COMMON_LIBS MATCHES "F7" OR _LINKER_SCRIPT MATCHES "F7")
        set(_STM32_FAMILY "F7")
    elseif(_COMMON_LIBS MATCHES "F4" OR _LINKER_SCRIPT MATCHES "F4")
        set(_STM32_FAMILY "F4")
    elseif(_COMMON_LIBS MATCHES "G4" OR _LINKER_SCRIPT MATCHES "G4")
        set(_STM32_FAMILY "G4")
    elseif(_COMMON_LIBS MATCHES "L4" OR _LINKER_SCRIPT MATCHES "L4")
        set(_STM32_FAMILY "L4")
    else()
        message(WARNING "[${TARGET_NAME}] Could not detect STM32 family, defaulting to F4")
        set(_STM32_FAMILY "F4")
    endif()

    if(_STM32_FAMILY STREQUAL "F7")
        set(_ARCH_FLAGS -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard)
        target_compile_definitions(${TARGET_NAME} PRIVATE STM32F732xx STM32F7xx)
    elseif(_STM32_FAMILY STREQUAL "F4")
        set(_ARCH_FLAGS -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
        target_compile_definitions(${TARGET_NAME} PRIVATE STM32F407xx STM32F4xx)
    elseif(_STM32_FAMILY STREQUAL "G4")
        set(_ARCH_FLAGS -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
        target_compile_definitions(${TARGET_NAME} PRIVATE STM32G474xx STM32G4xx)
    elseif(_STM32_FAMILY STREQUAL "L4")
        set(_ARCH_FLAGS -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
        target_compile_definitions(${TARGET_NAME} PRIVATE STM32L432xx STM32L4xx)
    else()
        message(WARNING "[${TARGET_NAME}] Defaulting to F4 MCU and defines.")
        set(_ARCH_FLAGS -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
        target_compile_definitions(${TARGET_NAME} PRIVATE STM32F407xx STM32F4xx)
    endif()

    target_compile_options(${TARGET_NAME} PRIVATE ${_ARCH_FLAGS})
    target_link_options(${TARGET_NAME} PRIVATE ${_ARCH_FLAGS})

    message(STATUS "[${TARGET_NAME}] STM32 family = ${_STM32_FAMILY}, applying ${_ARCH_FLAGS}")

    # Run postbuild actions like including a bootloader in the final image
    postbuild_target(${TARGET_NAME})
ENDMACRO()

# Same as above but defaults to xx_BL.ld for the linker script in bootloader builds
MACRO(COMMON_BOOTLOADER_COMPONENT TARGET_NAME)

    # Setup Component name based on directory
    get_target_property(_COMPONENT_NAME ${TARGET_NAME} COMPONENT_NAME)
    get_target_property(_COMPONENT_DIR  ${TARGET_NAME} COMPONENT_DIR)
    get_target_property(_LINKER_SCRIPT  ${TARGET_NAME} LINKER_SCRIPT)
    get_target_property(_COMMON_LIBS    ${TARGET_NAME} COMMON_LIBS)

    # Default values for target props.
    # if (NOT _LINKER_SCRIPT)
    #     set(_LINKER_SCRIPT "STM32L432KCUx_FLASH")
    # endif()
    # if (NOT _COMMON_LIBS)
    #     set(_COMMON_LIBS "CMSIS_L432;PHAL_L432;PSCHED;QUEUE;")
    # endif()

    # Add Common libraries
    foreach (_LIB_NAME IN ITEMS ${_COMMON_LIBS})
        target_link_libraries(${TARGET_NAME} ${_LIB_NAME})
    endforeach()
    target_link_libraries(${TARGET_NAME} common_defs)
    target_link_libraries(${TARGET_NAME} SYSCALLS)

    target_link_options(${TARGET_NAME} PRIVATE
        "-Wl,--whole-archive"
        "$<TARGET_FILE:SYSCALLS>"
        "-Wl,--no-whole-archive"
    )

    # Find all .c sources in project, recursive search starting at component root
    file(GLOB_RECURSE glob_sources ${_COMPONENT_DIR}/*.c)
    # Exclude starter files (templates used for code generation)
    list(FILTER glob_sources EXCLUDE REGEX ".*starter.*")
    target_sources(${TARGET_NAME} PUBLIC ${glob_sources})

    # Find directories for '#include'
    SUBDIRLIST(${_COMPONENT_DIR} include_dirs)
    target_include_directories(${TARGET_NAME} PUBLIC ${include_dirs} ${_COMPONENT_DIR})

    # Linker options
    target_link_options(${TARGET_NAME} PUBLIC
        -T${COMMON_SOURCE_DIR}/linker/${_LINKER_SCRIPT}_BL.ld
    )

    # Run postbuild actions like including a bootloader in the final image
    postbuild_target(${TARGET_NAME})
ENDMACRO()