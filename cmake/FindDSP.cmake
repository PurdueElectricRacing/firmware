function(make_cmsis_dsp_library LIB_NAME DIRNAME LINK_NAME)
    add_library(${LIB_NAME} STATIC)

    set(LIB_PATH "${CMAKE_SOURCE_DIR}/external/${DIRNAME}/Drivers/CMSIS/DSP")

    if(NOT EXISTS "${LIB_PATH}")
        message(FATAL_ERROR "CMSIS-DSP not found at ${LIB_PATH}")
    endif()

    # Include directories
    target_include_directories(${LIB_NAME}
        PUBLIC ${LIB_PATH}/Include
        PUBLIC ${LIB_PATH}/PrivateInclude
    )

    # Gather all DSP source files
    file(GLOB_RECURSE dsp_sources
        "${LIB_PATH}/Source/*.[csS]"
    )

    if(dsp_sources)
        target_sources(${LIB_NAME} PRIVATE ${dsp_sources})
    else()
        message(WARNING "No CMSIS-DSP source files found in ${LIB_PATH}/Source")
    endif()

    # Link with the device CMSIS core library (CMSIS_F407, CMSIS_F732, etc.)
    target_link_libraries(${LIB_NAME} ${LINK_NAME})

    if(${DIRNAME} MATCHES "STM32CubeF7")
        set(_ARCH_FLAGS -mthumb -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard)
        set(ARM_CORE_DEF "ARM_MATH_CM7")
        target_compile_definitions(${LIB_NAME} PRIVATE STM32F732xx STM32F7xx)

    elseif(${DIRNAME} MATCHES "STM32CubeF4")
        set(_ARCH_FLAGS -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
        set(ARM_CORE_DEF "ARM_MATH_CM4")
        target_compile_definitions(${LIB_NAME} PRIVATE STM32F407xx STM32F4xx)

    elseif(${DIRNAME} MATCHES "STM32CubeG4")
        set(_ARCH_FLAGS -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
        set(ARM_CORE_DEF "ARM_MATH_CM4")
        target_compile_definitions(${LIB_NAME} PRIVATE STM32G474xx STM32G4xx)

    elseif(${DIRNAME} MATCHES "STM32CubeL4")
        set(_ARCH_FLAGS -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
        set(ARM_CORE_DEF "ARM_MATH_CM4")
        target_compile_definitions(${LIB_NAME} PRIVATE STM32L496xx STM32L4xx)

    else()
        set(_ARCH_FLAGS -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
        set(ARM_CORE_DEF "ARM_MATH_CM4")
        message(WARNING "Unknown STM32 family for ${DIRNAME}, defaulting to ARM_MATH_CM4")
    endif()

    # Apply flags
    target_compile_options(${LIB_NAME} PRIVATE ${_ARCH_FLAGS})
    target_link_options(${LIB_NAME} PRIVATE ${_ARCH_FLAGS})
    target_compile_definitions(${LIB_NAME} PUBLIC ${ARM_CORE_DEF})

    # Disable analyzer warnings for external code
    target_compile_options(${LIB_NAME} PRIVATE -fno-analyzer)

    message(STATUS "Configured CMSIS-DSP library ${LIB_NAME} (${ARM_CORE_DEF}) with flags: ${_ARCH_FLAGS}")
endfunction()
