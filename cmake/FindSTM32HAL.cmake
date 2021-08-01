# CMAKE file for building STM32CubeL4 CMSIS module

function(make_stm32_hal_library LIB_NAME LIB_PATH)
    add_library(${LIB_NAME} STATIC)

    set(stm32_hal_includes ${LIB_PATH}/Inc)

    target_include_directories(${LIB_NAME} 
        PUBLIC ${stm32_hal_includes}
    )

    file(GLOB glob_sources ${LIB_PATH}/Src/*.c)
    list(FILTER glob_sources EXCLUDE REGEX "template\.c") # Remove template.c files from library
    target_sources(${LIB_NAME} 
        PUBLIC ${glob_sources}
    )
endfunction()

# Create multiple libraries with different defines
make_stm32_hal_library(STM32_L4_HAL_LIB ${CMAKE_SOURCE_DIR}/common/STM32CubeL4/Drivers/STM32L4xx_HAL_Driver)
make_stm32_hal_library(STM32_F4_HAL_LIB ${CMAKE_SOURCE_DIR}/common/STM32CubeF4/Drivers/STM32F4xx_HAL_Driver)
