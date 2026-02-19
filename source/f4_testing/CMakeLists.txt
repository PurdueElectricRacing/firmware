cmake_minimum_required(VERSION 3.13)

add_firmware_component(
    NAME f4_testing
    LINKER_SCRIPT "STM32F407VGTx_FLASH"
    LIBS "CMSIS_F407;PSCHED_F407;PHAL_F407;FREERTOS_F407"
)
