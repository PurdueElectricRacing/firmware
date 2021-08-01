# CMAKE file for building STM32CubeL4 CMSIS module


function(make_cmsis_library LIB_NAME STM32_FAMILY_NAME STM32_DEV_NAME LIB_PATH)
    add_library(${LIB_NAME} STATIC)
    string(TOLOWER ${STM32_DEV_NAME} STM32_DEV_NAME_LOWER)

    # Compile defs specific to target
    target_compile_definitions(${LIB_NAME}
        PUBLIC ${STM32_FAMILY_NAME}
        PUBLIC ${STM32_DEV_NAME}
        PUBLIC CMSIS_device_header=${STM32_DEV_NAME_LOWER}.h
    )

    set(cmsis_includes ${LIB_PATH}/Include)
    set(core_includes ${LIB_PATH}/Core/Include)
    set(stm32_family_includes ${LIB_PATH}/Device/ST/${STM32_FAMILY_NAME}/Include)

    target_include_directories(${LIB_NAME} 
        PUBLIC ${cmsis_includes} 
        PUBLIC ${core_includes} 
        PUBLIC ${stm32_family_includes}
    )

    target_sources(${LIB_NAME} 
        PUBLIC ${LIB_PATH}/Device/ST/${STM32_FAMILY_NAME}/Source/Templates/system_${STM32_FAMILY_NAME}.c
    )
endfunction()

# Create multiple libraries with different defines
make_cmsis_library(CMSIS_L432 STM32L4xx STM32L432xx ${CMAKE_SOURCE_DIR}/common/STM32CubeL4/Drivers/CMSIS)
make_cmsis_library(CMSIS_F407 STM32F4xx STM32F407xx ${CMAKE_SOURCE_DIR}/common/STM32CubeF4/Drivers/CMSIS)
