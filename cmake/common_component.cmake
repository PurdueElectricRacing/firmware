
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
        set(_LINKER_SCRIPT "STM32L432KCUx_FLASH")
    endif()
    if (NOT _COMMON_LIBS)
        set(_COMMON_LIBS "CMSIS_L432;PHAL_L432;PSCHED;QUEUE;FAULTS")
    endif()

    # Add Common libraries
    foreach (_LIB_NAME IN ITEMS ${_COMMON_LIBS})
        target_link_libraries(${TARGET_NAME} ${_LIB_NAME})
    endforeach(_LIB_NAME)

    target_link_libraries(${TARGET_NAME} common_defs) # everyone gets common defs

    # Find all .c sources in project, recursive search starting at component root
    file(GLOB_RECURSE glob_sources ${_COMPONENT_DIR}/*.c)
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
    endforeach(_LIB_NAME)

    target_link_libraries(${TARGET_NAME} common_defs) # everyone gets common defs

    # Find all .c sources in project, recursive search starting at component root
    file(GLOB_RECURSE glob_sources ${_COMPONENT_DIR}/*.c)
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