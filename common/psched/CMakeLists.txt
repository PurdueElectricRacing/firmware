cmake_minimum_required(VERSION 3.13)

# Common sources for all PSCHED variants
file(GLOB_RECURSE glob_sources "*.c")

# Common include path
set(COMMON_INCLUDES ${CMAKE_SOURCE_DIR})

# Common function to apply architecture flags
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
        message(FATAL_ERROR "Unknown family ${family}")
    endif()

    target_compile_options(${target} PRIVATE ${_ARCH_FLAGS})
    target_link_options(${target} PRIVATE ${_ARCH_FLAGS})
endfunction()

# L432
set(TARGET_NAME PSCHED)
add_library(${TARGET_NAME})
target_sources(${TARGET_NAME} PRIVATE ${glob_sources})
target_include_directories(${TARGET_NAME} PUBLIC ${COMMON_INCLUDES})
target_link_libraries(${TARGET_NAME} CMSIS_L432)
apply_arch_flags(${TARGET_NAME} L4)

# L471
set(TARGET_NAME PSCHED_L471)
add_library(${TARGET_NAME})
target_sources(${TARGET_NAME} PRIVATE ${glob_sources})
target_include_directories(${TARGET_NAME} PUBLIC ${COMMON_INCLUDES})
target_link_libraries(${TARGET_NAME} CMSIS_L471)
apply_arch_flags(${TARGET_NAME} L4)

# L496
set(TARGET_NAME PSCHED_L496)
add_library(${TARGET_NAME})
target_sources(${TARGET_NAME} PRIVATE ${glob_sources})
target_include_directories(${TARGET_NAME} PUBLIC ${COMMON_INCLUDES})
target_link_libraries(${TARGET_NAME} CMSIS_L496)
apply_arch_flags(${TARGET_NAME} L4)

# F407
set(TARGET_NAME PSCHED_F407)
add_library(${TARGET_NAME})
target_sources(${TARGET_NAME} PRIVATE ${glob_sources})
target_include_directories(${TARGET_NAME} PUBLIC ${COMMON_INCLUDES})
target_link_libraries(${TARGET_NAME} CMSIS_F407)
apply_arch_flags(${TARGET_NAME} F4)

# F732
set(TARGET_NAME PSCHED_F732)
add_library(${TARGET_NAME})
target_sources(${TARGET_NAME} PRIVATE ${glob_sources})
target_include_directories(${TARGET_NAME} PUBLIC ${COMMON_INCLUDES})
target_link_libraries(${TARGET_NAME} CMSIS_F732)
apply_arch_flags(${TARGET_NAME} F7)

# TIM7 variant
set(TARGET_NAME PSCHED_TIM7)
add_library(${TARGET_NAME})
target_sources(${TARGET_NAME} PRIVATE ${glob_sources})
target_include_directories(${TARGET_NAME} PUBLIC ${COMMON_INCLUDES})
target_link_libraries(${TARGET_NAME} CMSIS_L432)
target_compile_definitions(${TARGET_NAME} PUBLIC PSCHED_USE_TIM7=1)
apply_arch_flags(${TARGET_NAME} L4)
