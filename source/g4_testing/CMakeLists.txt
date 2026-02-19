cmake_minimum_required(VERSION 3.13)

add_firmware_component(
    NAME g4_testing
    LINKER_SCRIPT "STM32G474RETX_FLASH"
    LIBS "CMSIS_G474;PHAL_G474;FREERTOS_G474"
)
