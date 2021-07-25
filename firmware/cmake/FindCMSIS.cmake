cmake_minimum_required(VERSION 3.10)

# CMAKE file for building STM32CubeL4 CMSIS module

set(LIB_PATH "${CMAKE_SOURCE_DIR}/common/STM32CubeL4")

function(make_cmsis_library LIB_NAME STM32_DEV_NAME)
    add_library(${LIB_NAME} STATIC)
    target_compile_definitions(${LIB_NAME}
        PUBLIC -D${STM32_DEV_NAME}
    )

    set(cmsis_includes "${LIB_PATH}/Drivers/CMSIS/Include")
    set(core_includes "${LIB_PATH}/Drivers/CMSIS/Core/Include")
    set(stm32l4_includes "${LIB_PATH}/Drivers/CMSIS/Device/ST/STM32L4xx/Include")

    target_include_directories(${LIB_NAME} 
        PUBLIC ${cmsis_includes} 
        PUBLIC ${core_includes} 
        PUBLIC ${stm32l4_includes}
    )

    target_sources(${LIB_NAME} 
        PUBLIC "${LIB_PATH}/Drivers/CMSIS/Device/ST/STM32L4xx/Source/Templates/system_stm32l4xx.c"
    )
endfunction()

# Create multiple libraries with different defines
make_cmsis_library(CMSIS_L432 STM32L432xx)
