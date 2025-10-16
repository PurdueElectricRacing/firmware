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

    # Link with the device CMSIS core library
    target_link_libraries(${LIB_NAME} ${LINK_NAME})

    # Determine the ARM core type from the directory name
    if(${DIRNAME} MATCHES "L4" OR ${DIRNAME} MATCHES "G4" OR ${DIRNAME} MATCHES "F4")
        set(ARM_CORE_DEF "ARM_MATH_CM4")
    elseif(${DIRNAME} MATCHES "F7")
        set(ARM_CORE_DEF "ARM_MATH_CM7")
    else()
        set(ARM_CORE_DEF "ARM_MATH_CM4") # default fallback
        message(WARNING "Unknown STM32 family for ${DIRNAME}, defaulting to ARM_MATH_CM4")
    endif()

    target_compile_definitions(${LIB_NAME} PUBLIC ${ARM_CORE_DEF})

    # Disable analyzer warnings for external code
    target_compile_options(${LIB_NAME} PRIVATE -fno-analyzer)

    message(STATUS "Configured CMSIS-DSP library ${LIB_NAME} (${ARM_CORE_DEF})")
endfunction()