function(make_freertos_library LIB_NAME DIRNAME LINK_NAME)
    add_library(${LIB_NAME} STATIC)
    set(LIB_PATH "${CMAKE_SOURCE_DIR}/external/${DIRNAME}/Middlewares/Third_Party/FreeRTOS/Source")

    if(${DIRNAME} MATCHES "STM32CubeF7")
        set(PORTABLE_PATH "${LIB_PATH}/portable/GCC/ARM_CM7/r0p1")
        set(_ARCH_FLAGS -mthumb -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard)
        target_compile_definitions(${LIB_NAME} PRIVATE STM32F732xx STM32F7xx)
    elseif(${DIRNAME} MATCHES "STM32CubeF4")
        set(PORTABLE_PATH "${LIB_PATH}/portable/GCC/ARM_CM4F")
        set(_ARCH_FLAGS -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
        target_compile_definitions(${LIB_NAME} PRIVATE STM32F407xx STM32F4xx)
    elseif(${DIRNAME} MATCHES "STM32CubeG4")
        set(PORTABLE_PATH "${LIB_PATH}/portable/GCC/ARM_CM4F")
        set(_ARCH_FLAGS -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
        target_compile_definitions(${LIB_NAME} PRIVATE STM32G474xx STM32G4xx)
    elseif(${DIRNAME} MATCHES "STM32CubeL4")
        set(PORTABLE_PATH "${LIB_PATH}/portable/GCC/ARM_CM4F")
        set(_ARCH_FLAGS -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
        target_compile_definitions(${LIB_NAME} PRIVATE STM32L496xx STM32L4xx)
    endif()

    target_include_directories(${LIB_NAME}
        PUBLIC ${LIB_PATH}/include
        PUBLIC ${LIB_PATH}/CMSIS_RTOS_V2
        PUBLIC ${PORTABLE_PATH}
        PUBLIC ${CMAKE_SOURCE_DIR}/common/freertos
    )

    file(GLOB glob_sources
        "${LIB_PATH}/*.c"
        "${LIB_PATH}/CMSIS_RTOS_V2/*.c"
        "${PORTABLE_PATH}/*.c"
        "${LIB_PATH}/portable/MemMang/heap_4.c"
    )
    target_sources(${LIB_NAME} PRIVATE ${glob_sources})

    target_link_libraries(${LIB_NAME} ${LINK_NAME})
    target_compile_options(${LIB_NAME} PRIVATE ${_ARCH_FLAGS} -fno-analyzer)
    target_link_options(${LIB_NAME} PRIVATE ${_ARCH_FLAGS})
endfunction()

make_freertos_library(FREERTOS_LIB_F407 STM32CubeF4 CMSIS_F407)
make_freertos_library(FREERTOS_LIB_F732 STM32CubeF7 CMSIS_F732)
make_freertos_library(FREERTOS_LIB_G474 STM32CubeG4 CMSIS_G474)
make_freertos_library(FREERTOS_LIB_L496 STM32CubeL4 CMSIS_L496)
