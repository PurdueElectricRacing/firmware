cmake_minimum_required(VERSION 3.13)

# Setup Component name based on directory
get_filename_component(COMPONENT_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
STRING(TOLOWER ${COMPONENT_NAME} COMPONENT_NAME)
set(TARGET_NAME ${COMPONENT_NAME}.elf)
add_executable(${TARGET_NAME})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR})

include(${CMAKE_CURRENT_SOURCE_DIR}/../../common/can_library/can_library.cmake)
create_can_node_lib(DAQ "CMSIS_F407;FREERTOS_F407")

# Propreties are set in order to make the common component
set_target_properties(${TARGET_NAME} PROPERTIES
    COMPONENT_NAME ${COMPONENT_NAME}
    COMPONENT_DIR  ${CMAKE_CURRENT_LIST_DIR}
    LINKER_SCRIPT  "STM32F407VGTx_FLASH"
    COMMON_LIBS    "can_node_DAQ;CMSIS_F407;QUEUE;DAQ_F407;FREERTOS_F407"
)
COMMON_FIRMWARE_COMPONENT(${TARGET_NAME})
