cmake_minimum_required(VERSION 3.10)

# CMAKE file for building STM32CubeL4 module

add_library(stm32l4_cmsis SHARED)

set(LIB_PATH "../common/STM32CubeL4")
set(cmsis_includes "${LIB_PATH}/Drivers/CMSIS/Include")
set(core_includes "${LIB_PATH}/Drivers/CMSIS/Core/Include")
set(stm32l4_includes "${LIB_PATH}/Drivers/CMSIS/Device/ST/STM32L4xx/Include")

target_include_directories(stm32l4_cmsis ${cmsis_includes} ${cmsis_includes} ${stm32l4_includes})
target_sources(stm32l4_cmsis "${LIB_PATH}/Drivers/CMSIS/Device/ST/STM32L4xx/system_stm32l4xx.c")