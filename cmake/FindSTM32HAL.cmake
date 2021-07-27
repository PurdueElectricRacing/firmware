# CMAKE file for building STM32CubeL4 CMSIS module

set(LIB_PATH "${CMAKE_SOURCE_DIR}/common/STM32CubeL4/Drivers/STM32L4xx_HAL_Driver")

function(make_stm32_hal_library LIB_NAME)
    add_library(${LIB_NAME} STATIC)

    set(stm32_hal_includes "${LIB_PATH}/Inc")

    target_include_directories(${LIB_NAME} 
        PUBLIC ${stm32_hal_includes}
    )

    file(GLOB glob_sources "${LIB_PATH}/Src/stm32l4xx_hal_can.c")
    target_sources(${LIB_NAME} 
        PUBLIC ${glob_sources}
    )
endfunction()

# Create multiple libraries with different defines
make_stm32_hal_library(STM32_HAL_LIB)
