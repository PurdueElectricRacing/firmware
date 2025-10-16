cmake_minimum_required(VERSION 3.13)

# Helper for architecture flags
function(apply_arch_flags target family)
    if(${family} STREQUAL "F7")
        set(_ARCH_FLAGS -mthumb -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard)
        target_compile_definitions(${target} PUBLIC STM32F732xx STM32F7xx)
    elseif(${family} STREQUAL "F4")
        set(_ARCH_FLAGS -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
        target_compile_definitions(${target} PUBLIC STM32F407xx STM32F4xx)
    elseif(${family} STREQUAL "L4")
        set(_ARCH_FLAGS -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
        target_compile_definitions(${target} PUBLIC STM32L4xx)
    else()
        message(FATAL_ERROR "Unknown MCU family: ${family}")
    endif()

    target_compile_options(${target} PRIVATE ${_ARCH_FLAGS})
    target_link_options(${target} PRIVATE ${_ARCH_FLAGS})
endfunction()

# Fault library macro
MACRO(MAKE_FLT_LIB lib_name lib_link hal_name arch_family)
    set(TARGET_NAME ${lib_name})
    add_library(${TARGET_NAME})

    file(GLOB_RECURSE glob_sources "*.c")
    target_sources(${TARGET_NAME} PRIVATE ${glob_sources})
    target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR})

    target_link_libraries(${TARGET_NAME} ${lib_link})
    target_link_libraries(${TARGET_NAME} ${hal_name})

    apply_arch_flags(${TARGET_NAME} ${arch_family})
ENDMACRO()

MAKE_FLT_LIB(FAULTS CMSIS_L432 PHAL_L432 L4)
MAKE_FLT_LIB(FAULTS_L496 CMSIS_L496 PHAL_L496 L4)
MAKE_FLT_LIB(FAULTS_F407 CMSIS_F407 PHAL_F407 F4)
MAKE_FLT_LIB(FAULTS_F732 CMSIS_F732 PHAL_F732 F7)
