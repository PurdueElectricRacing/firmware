# CMAKE file for building STM32 CMSIS modules
# CMSIS is the building block for all other HAL/Firmware modules that interact with the hardware

# Function: make_cmsis_library
# Parameters:
# - LIB_NAME: Name of the library output
# - STM32_FAMILY_NAME: STM Family used to locate correct CMSIS defines (e.g. STM32F4xx)
# - STM32_DEVICE_NAME: Extended device name (e.g. STM32F407xx)
# - LIB_PATH: Path to CMSIS root folder.
function(make_cmsis_library LIB_NAME STM32_FAMILY_NAME STM32_DEVICE_NAME LIB_PATH)
    add_library(${LIB_NAME} STATIC)
    
    string(TOLOWER ${STM32_DEVICE_NAME} STM32_DEVICE_NAME_LOWER)
    string(TOLOWER ${STM32_FAMILY_NAME} STM32_FAMILY_NAME_LOWER)

    # Get and apply CPU flags (mcpu, fpu, float-abi)
    get_cpu_flags(${STM32_FAMILY_NAME} CPU_FLAGS)
    target_compile_options(${LIB_NAME} PUBLIC ${CPU_FLAGS})
    target_link_options(${LIB_NAME} PUBLIC ${CPU_FLAGS})

    # Compile definitions specific to target
    target_compile_definitions(${LIB_NAME}
        PUBLIC ${STM32_FAMILY_NAME}
        PUBLIC ${STM32_DEVICE_NAME}
    )

    # Include CMSIS generic, CMSIS Core, and device-specific CMSIS wrapper
    target_include_directories(${LIB_NAME} 
        PUBLIC ${LIB_PATH}/Core/Include
        PUBLIC ${LIB_PATH}/Device/ST/${STM32_FAMILY_NAME}/Include
    )

    # Add core system files and assembly startup files
    target_sources(${LIB_NAME} 
        PRIVATE ${LIB_PATH}/Device/ST/${STM32_FAMILY_NAME}/Source/Templates/system_${STM32_FAMILY_NAME_LOWER}.c
        PRIVATE ${LIB_PATH}/Device/ST/${STM32_FAMILY_NAME}/Source/Templates/gcc/startup_${STM32_DEVICE_NAME_LOWER}.s
    )

    # Disable analyzer flags for external CMSIS code
    target_compile_options(${LIB_NAME} PRIVATE -fno-analyzer)
endfunction()
