# CMAKE file for building STM32CubeXX CMSIS module

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

    # Disable analyzer flags for external STM32 HAL libraries
    target_compile_options(${LIB_NAME} PRIVATE -fno-analyzer)
endfunction()