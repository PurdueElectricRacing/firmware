
# Create an executiable with the following features:
#   All .c files are added from the root and child directories of "component_dir"
#   All .h files in root and child directories of "component_dir" are included without prefix
#       - a file like "apps/apps.h" can be included like #include "apps.h"
#   All CMAKE libraries specified in "common_libs" are linked
#   Linked using the default L4 linker file
MACRO(MAKE_COMMON_COMPONENT component_name component_dir common_libs)

    # Setup Component name based on directory
    set(_TARGET_NAME ${component_name}.elf)
    add_executable(${_TARGET_NAME})

    # Add Common libraries
    foreach (_LIB_NAME IN ITEMS ${common_libs})
        target_link_libraries(${_TARGET_NAME} ${_LIB_NAME})
    endforeach(_LIB_NAME)

    target_link_libraries(${_TARGET_NAME} common_defs) # everyone gets common defs

    # Find all .c sources in project, recursive search starting at component root
    file(GLOB_RECURSE glob_sources ${component_dir}/*.c)
    target_sources(${_TARGET_NAME} PUBLIC ${glob_sources})

    # Find directories for '#include'
    SUBDIRLIST(${component_dir} include_dirs)
    target_include_directories(${_TARGET_NAME} PUBLIC ${include_dirs})

    # Linker options
    target_link_options(${_TARGET_NAME} PUBLIC 
        -T${COMMON_SOURCE_DIR}/linker/STM32L432KCUx_FLASH.ld
    )

    # Run postbuild actions like including a bootloader in the final image
    postbuild_target(${component_name} ${_TARGET_NAME})
ENDMACRO()

MACRO(MAKE_BOOTLOADER _TARGET_NAME component_dir common_libs linker_script)

    

    
ENDMACRO()