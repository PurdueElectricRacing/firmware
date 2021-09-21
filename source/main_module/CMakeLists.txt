cmake_minimum_required(VERSION 3.13)

# Setup Component name based on directory
get_filename_component(COMPONENT_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
STRING(TOLOWER ${COMPONENT_NAME} COMPONENT_NAME)
set(TARGET_NAME ${COMPONENT_NAME}.elf)
add_executable(${TARGET_NAME})

# Add STM32_HAL Library
target_link_libraries(${TARGET_NAME} STM32_L4_HAL_LIB)
target_link_libraries(STM32_L4_HAL_LIB CMSIS_L432)
target_include_directories(STM32_L4_HAL_LIB PUBLIC hal)

# Add PER RTOS Library
target_link_libraries(${TARGET_NAME} PSCHED)
target_link_libraries(${TARGET_NAME} PHAL_L4)

# Find all .c sources in project, recursive search starting at component root
file(GLOB_RECURSE glob_sources "*.c")
target_sources(${TARGET_NAME} PRIVATE ${glob_sources})

# Find directories for '#include'
SUBDIRLIST(${CMAKE_CURRENT_LIST_DIR} include_dirs)
target_include_directories(${TARGET_NAME} PRIVATE ${include_dirs})

# Linker options
target_link_options(${TARGET_NAME} PUBLIC 
    -T${COMMON_SOURCE_DIR}/linker/STM32L432KCUx_FLASH.ld
)

# Run postbuild actions like including a bootloader in the final image
postbuild_target(${COMPONENT_NAME} ${TARGET_NAME})