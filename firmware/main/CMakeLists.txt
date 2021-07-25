cmake_minimum_required(VERSION 3.10)


add_executable(main main.c startup_stm32l432xx.s)

target_compile_options(main PRIVATE
        -mcpu=cortex-m4
        -mthumb
        -mfpu=fpv4-sp-d16
        -mfloat-abi=hard

        -fdata-sections
        -ffunction-sections

        -Wall
)

target_link_options(main PRIVATE 
    -T${CMAKE_SOURCE_DIR}/main/STM32L432KCUx_FLASH.ld
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
    --specs=nano.specs
    -lc
    -lm
    -lnosys
)

target_link_libraries(main INTERFACE stm32l4_cmsis)