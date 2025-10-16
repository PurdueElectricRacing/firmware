# CMAKE file for building STM32CubeL4 CMSIS module

# CMSIS is the building block for all other HAL/Firmware modules that interact with the hardware

define_property(TARGET PROPERTY STM32_FAMILY_NAME
    BRIEF_DOCS "MCU archetecture target" 
    FULL_DOCS  "STM32 Family such as STM32L4xx used in compiling the correct libaries for HAL and CMSIS"
)

define_property(TARGET PROPERTY STM32_DEVICE_NAME 
    BRIEF_DOCS "MCU device name, used for HAL #defines" 
    FULL_DOCS  "STM32 Device such as STM32L432xx used in compiling the correct libaries for HAL and CMSIS"
)

define_property(TARGET PROPERTY CMSIS_PATH
    BRIEF_DOCS "Path to device CMSIS files" 
    FULL_DOCS  "Path to CMSIS files"
)

# Function
#   make_cmsis_library
# Paramaters
# - LIB_NAME; Name of the library output
# - STM32_FAMILY_NAME; STM Family used to locate correct CMSIS defines
# - STM32_DEVICE_NAME; Extended device name, reference /external/STM32CubeL4/Drivers/CMSIS/Device/ST/STM32L4xx/Source/gcc for examples
# - LIB_PATH; Path to CMSIS root folder.
function(make_cmsis_library LIB_NAME STM32_FAMILY_NAME STM32_DEVICE_NAME LIB_PATH)
    add_library(${LIB_NAME} STATIC)
    string(TOLOWER ${STM32_DEVICE_NAME} STM32_DEVICE_NAME_LOWER)
    string(TOLOWER ${STM32_FAMILY_NAME} STM32_FAMILY_NAME_LOWER)

    # Target propreties for getting the specific device paramaters from the CMSIS library
    # use get_target_properties(...) to extract this info
    set_target_properties(${LIB_NAME} PROPERTIES
        STM32_FAMILY_NAME ${STM32_FAMILY_NAME}
        STM32_DEVICE_NAME ${STM32_DEVICE_NAME}
        CMSIS_PATH        ${LIB_PATH}  
    )

    # Compile defs specific to target
    # These are used to specify the peripheral mappings and device capabilities
    # for both the CMSIS and HAL libraries.
    target_compile_definitions(${LIB_NAME}
        PUBLIC ${STM32_FAMILY_NAME}
        PUBLIC ${STM32_DEVICE_NAME}
        # PRIVATE CMSIS_device_header=${STM32_DEVICE_NAME_LOWER}.h
    )

    # Include CMSIS generic, CMSIS Core, and device specific CMSIS wrapper
    set(cmsis_includes ${LIB_PATH}/Include)
    set(core_includes ${LIB_PATH}/Core/Include)
    set(stm32_family_includes ${LIB_PATH}/Device/ST/${STM32_FAMILY_NAME}/Include)

    target_include_directories(${LIB_NAME} 
        PUBLIC ${cmsis_includes} 
        PUBLIC ${core_includes} 
        PUBLIC ${stm32_family_includes}
    )

    target_sources(${LIB_NAME} 
        PRIVATE ${LIB_PATH}/Device/ST/${STM32_FAMILY_NAME}/Source/Templates/system_${STM32_FAMILY_NAME_LOWER}.c
        PRIVATE ${LIB_PATH}/Device/ST/${STM32_FAMILY_NAME}/Source/Templates/gcc/startup_${STM32_DEVICE_NAME_LOWER}.s
    )

    if(${STM32_FAMILY_NAME} MATCHES "F7")
        set(_ARCH_FLAGS -mthumb -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard)
    elseif(${STM32_FAMILY_NAME} MATCHES "F4" OR
           ${STM32_FAMILY_NAME} MATCHES "G4" OR
           ${STM32_FAMILY_NAME} MATCHES "L4")
        set(_ARCH_FLAGS -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
    else()
        message(FATAL_ERROR "Unknown STM32 family: ${STM32_FAMILY_NAME}")
    endif()

    target_compile_options(${LIB_NAME} PRIVATE ${_ARCH_FLAGS} -fno-analyzer)
    target_link_options(${LIB_NAME}    PRIVATE ${_ARCH_FLAGS})
endfunction()
