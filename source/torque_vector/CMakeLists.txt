cmake_minimum_required(VERSION 3.13)

create_can_node_lib(TORQUE_VECTOR CMSIS_F732)

add_firmware_component(
    NAME torque_vector
    LINKER_SCRIPT "STM32F732RETX_FLASH"
    LIBS "CMSIS_F732;PSCHED_F732;QUEUE;can_node_TORQUE_VECTOR;PHAL_F732;libm.a;BOOTLOADER_COMMON_F732"
)
