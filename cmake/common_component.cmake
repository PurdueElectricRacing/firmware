
# Create an executiable with the following features:
#   All .c files are added from the root and child directories of "component_dir"
#   All .h files in root and child directories of "component_dir" are included without prefix
#       - a file like "apps/apps.h" can be included like #include "apps.h"
#   All CMAKE libraries specified in "common_libs" are linked
#   Linked using the default L4 linker file
MACRO(COMMON_FIRMWARE_COMPONENT target_name)

    # Setup Component name based on directory
    get_target_property(_COMPONENT_NAME ${target_name} COMPONENT_NAME)
    get_target_property(_COMPONENT_DIR  ${target_name} COMPONENT_DIR)
    get_target_property(_LINKER_SCRIPT  ${target_name} LINKER_SCRIPT)
    get_target_property(_COMMON_LIBS    ${target_name} COMMON_LIBS)

    # Add Common libraries
    foreach (_LIB_NAME IN ITEMS ${_COMMON_LIBS})
        target_link_libraries(${target_name} ${_LIB_NAME})
    endforeach(_LIB_NAME)

    target_link_libraries(${target_name} common_defs) # everyone gets common defs

    # Find all .c sources in project, recursive search starting at component root
    file(GLOB_RECURSE glob_sources ${_COMPONENT_DIR}/*.c)
    target_sources(${target_name} PUBLIC ${glob_sources})

    # Find directories for '#include'
    SUBDIRLIST(${_COMPONENT_DIR} include_dirs)
    target_include_directories(${target_name} PUBLIC ${include_dirs} ${_COMPONENT_DIR})

    # Linker options
    target_link_options(${target_name} PUBLIC 
        -T${COMMON_SOURCE_DIR}/linker/${_LINKER_SCRIPT}
    )

    # Run postbuild actions like including a bootloader in the final image
    postbuild_target(${_COMPONENT_NAME} ${target_name})
ENDMACRO()