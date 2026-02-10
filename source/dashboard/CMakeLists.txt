cmake_minimum_required(VERSION 3.13)

# Setup Component name based on directory
get_filename_component(COMPONENT_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
STRING(TOLOWER ${COMPONENT_NAME} COMPONENT_NAME)

set(TARGET_NAME ${COMPONENT_NAME}.elf)
add_executable(${TARGET_NAME})

include(../../common/can_library/can_library.cmake)
create_can_node_lib(DASHBOARD "CMSIS_G474;FREERTOS_G474")

# Propreties are set in order to make the common component
set_target_properties(${TARGET_NAME} PROPERTIES
    COMPONENT_NAME ${COMPONENT_NAME}
    COMPONENT_DIR  ${CMAKE_CURRENT_LIST_DIR}
    LINKER_SCRIPT  "STM32F407VGTx_FLASH"
    COMMON_LIBS    "can_node_DASHBOARD;CMSIS_G474;FREERTOS_G474;QUEUE;PHAL_G474;STRBUF"
)

COMMON_FIRMWARE_COMPONENT(${TARGET_NAME})
