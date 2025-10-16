cmake_minimum_required(VERSION 3.13)

# ---- Helper: apply architecture flags ----
function(apply_arch_flags target_name family)
    if(${family} STREQUAL "F7")
        set(_ARCH_FLAGS -mthumb -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard)
        target_compile_definitions(${target_name} PUBLIC STM32F732xx STM32F7xx)
    elseif(${family} STREQUAL "F4")
        set(_ARCH_FLAGS -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
        target_compile_definitions(${target_name} PUBLIC STM32F407xx STM32F4xx)
    elseif(${family} STREQUAL "L4")
        set(_ARCH_FLAGS -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
        target_compile_definitions(${target_name} PUBLIC STM32L4xx)
    elseif(${family} STREQUAL "G4")
        set(_ARCH_FLAGS -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
        target_compile_definitions(${target_name} PUBLIC STM32G4xx)
    else()
        message(FATAL_ERROR "Unknown architecture family: ${family} for target ${target_name}")
    endif()

    target_compile_options(${target_name} PRIVATE ${_ARCH_FLAGS})
    target_link_options(${target_name} PRIVATE ${_ARCH_FLAGS})
endfunction()

# DAQ
MACRO(MAKE_DAQ_LIB lib_name lib_link hal_name arch_family)
    set(TARGET_NAME ${lib_name})
    add_library(${TARGET_NAME})

    target_sources(${TARGET_NAME} PRIVATE "daq_base.c")
    target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR})

    target_link_libraries(${TARGET_NAME} QUEUE)
    target_link_libraries(${TARGET_NAME} ${lib_link})
    target_link_libraries(${TARGET_NAME} ${hal_name})

    apply_arch_flags(${TARGET_NAME} ${arch_family})
ENDMACRO()

MAKE_DAQ_LIB(DAQ CMSIS_L432 PHAL_L432 L4)
MAKE_DAQ_LIB(DAQ_L496 CMSIS_L496 PHAL_L496 L4)
MAKE_DAQ_LIB(DAQ_F407 CMSIS_F407 PHAL_F407 F4)
MAKE_DAQ_LIB(DAQ_F732 CMSIS_F732 PHAL_F732 F7)

# CAN_PARSE
MACRO(MAKE_CAN_PARSE_LIB lib_name lib_link hal_name sched_name arch_family)
    set(TARGET_NAME ${lib_name})
    add_library(${TARGET_NAME})

    target_sources(${TARGET_NAME} PRIVATE "can_parse_base.c")
    target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR})

    target_link_libraries(${TARGET_NAME} QUEUE)
    target_link_libraries(${TARGET_NAME} ${lib_link})
    target_link_libraries(${TARGET_NAME} ${hal_name})
    target_link_libraries(${TARGET_NAME} ${sched_name})

    apply_arch_flags(${TARGET_NAME} ${arch_family})
ENDMACRO()

MAKE_CAN_PARSE_LIB(CAN_PARSE CMSIS_L432 PHAL_L432 PSCHED L4)
MAKE_CAN_PARSE_LIB(CAN_PARSE_L496 CMSIS_L496 PHAL_L496 PSCHED_L496 L4)
MAKE_CAN_PARSE_LIB(CAN_PARSE_F407 CMSIS_F407 PHAL_F407 PSCHED_F407 F4)
MAKE_CAN_PARSE_LIB(CAN_PARSE_F732 CMSIS_F732 PHAL_F732 PSCHED_F732 F7)
