include(../../common/can_library/can_library.cmake)
create_can_node_lib(MAIN_MODULE CMSIS_F407)

add_firmware_component(
    NAME main_module
    LINKER_SCRIPT "STM32F407VGTx_FLASH"
    LIBS "CMSIS_F407;PSCHED_F407;QUEUE;PHAL_F407;AMK;BOOTLOADER_COMMON_F407;can_node_MAIN_MODULE"
)
