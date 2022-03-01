cmake_minimum_required(VERSION 3.13)

# Setup Component name based on directory
get_filename_component(COMPONENT_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
STRING(TOLOWER ${COMPONENT_NAME} COMPONENT_NAME)


# Setup Component name based on directory
set(_TARGET_NAME ${COMPONENT_NAME}.elf)
add_executable(${_TARGET_NAME})
target_compile_definitions(${_TARGET_NAME} PUBLIC APP_ID=APP_MAINMODULE)


# MAKE_BOOTLOADER(${COMPONENT_NAME} ${CMAKE_CURRENT_LIST_DIR} "CMSIS_L432;PSCHED;QUEUE;PHAL_L432" "STM32L432KCUx_FLASH_BL.ld")

# Add Common libraries
target_link_libraries(${_TARGET_NAME} CMSIS_L432 PSCHED QUEUE PHAL_L432)
target_link_libraries(${_TARGET_NAME} common_defs)

# Find all .c sources in project, recursive search starting at component root
file(GLOB_RECURSE glob_sources ${CMAKE_CURRENT_LIST_DIR}/*.c)
target_sources(${_TARGET_NAME} PUBLIC ${glob_sources})

# Find directories for '#include'
SUBDIRLIST(${CMAKE_CURRENT_LIST_DIR} include_dirs)
target_include_directories(${_TARGET_NAME} PUBLIC ${include_dirs} ${CMAKE_CURRENT_LIST_DIR} )

# Linker options
target_link_options(${_TARGET_NAME} PUBLIC 
    -T${COMMON_SOURCE_DIR}/linker/STM32L432KCUx_FLASH_BL.ld
)

# Run postbuild actions like including a bootloader in the final image
postbuild_target(${COMPONENT_NAME} ${_TARGET_NAME})