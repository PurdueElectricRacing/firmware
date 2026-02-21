include(${CMAKE_CURRENT_SOURCE_DIR}/../../common/can_library/can_library.cmake)
create_can_node_lib(DAQ "CMSIS_F407;FREERTOS_F407")

add_firmware_component(
    NAME daq
    LINKER_SCRIPT "STM32F407VGTx_FLASH"
    LIBS "can_node_DAQ;CMSIS_F407;PHAL_F407;QUEUE;FREERTOS_F407;SPMC"
)
target_include_directories(daq.elf PUBLIC ${CMAKE_SOURCE_DIR})
