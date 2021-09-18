cmake_minimum_required(VERSION 3.13)


get_filename_component(COMPONENT_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
STRING(TOLOWER ${COMPONENT_NAME} COMPONENT_NAME)
set(TARGET_NAME ${COMPONENT_NAME}.elf)
add_executable(${TARGET_NAME})

# Add HAL Library
target_link_libraries(${TARGET_NAME} STM32_F4_HAL_LIB)
target_link_libraries(STM32_F4_HAL_LIB CMSIS_F407)
target_include_directories(STM32_F4_HAL_LIB PUBLIC hal)

# Find all .c sources in project
file(GLOB_RECURSE glob_sources "*.c")
target_sources(${TARGET_NAME} PRIVATE ${glob_sources})

# Find directories for '#include'
SUBDIRLIST(${CMAKE_CURRENT_LIST_DIR} include_dirs)
target_include_directories(${TARGET_NAME} PRIVATE ${include_dirs})

# Linker options
target_link_options(${TARGET_NAME} PRIVATE 
    -T${COMMON_SOURCE_DIR}/linker/STM32F407VGTx_FLASH.ld
)

postbuild_target(${COMPONENT_NAME} ${TARGET_NAME})