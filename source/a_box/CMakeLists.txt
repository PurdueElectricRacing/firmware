cmake_minimum_required(VERSION 3.13)

# Setup Component name based on directory
get_filename_component(COMPONENT_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
STRING(TOLOWER ${COMPONENT_NAME} COMPONENT_NAME)

set(TARGET_NAME ${COMPONENT_NAME}.elf)
add_executable(${TARGET_NAME})

# Create the CAN node library for this component
create_can_node_lib(A_BOX "CMSIS_F407;FREERTOS_F407")

# Propreties are set in order to make the common component
set_target_properties(${TARGET_NAME} PROPERTIES
    COMPONENT_NAME ${COMPONENT_NAME}
    COMPONENT_DIR  ${CMAKE_CURRENT_LIST_DIR}
    LINKER_SCRIPT  "STM32F407VGTx_FLASH"
    COMMON_LIBS    "CMSIS_F407;PSCHED_F407;QUEUE;can_node_A_BOX;PHAL_F407;FAULTS_F407;m;BOOTLOADER_COMMON_F407"
)
COMMON_FIRMWARE_COMPONENT(${TARGET_NAME})
