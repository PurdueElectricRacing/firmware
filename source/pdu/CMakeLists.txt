cmake_minimum_required(VERSION 3.13)

include(${CMAKE_CURRENT_LIST_DIR}/../../common/can_library/can_library.cmake)
create_can_node_lib(PDU CMSIS_F407)

add_firmware_component(
    NAME pdu
    LINKER_SCRIPT "STM32F407VGTx_FLASH"
    LIBS "can_node_PDU;CMSIS_F407;PSCHED_F407;PHAL_F407;QUEUE;BOOTLOADER_COMMON_F407"
)