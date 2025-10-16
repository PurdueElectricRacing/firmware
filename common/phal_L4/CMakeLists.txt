cmake_minimum_required(VERSION 3.13)

MACRO(MAKE_PHAL_LIBRARY lib_name lib_link)
    set(TARGET_NAME ${lib_name})
    add_library(${TARGET_NAME})

    # Find all .c sources in project
    file(GLOB_RECURSE glob_sources "*.c")
    target_sources(${TARGET_NAME} PRIVATE ${glob_sources})

    # Find directories for '#include'
    # For libraries, these directories are all referenced to the top level firmware directory, CMAKE_SOURCE_DIR
    target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR})

    target_link_libraries(${TARGET_NAME} ${lib_link})
    target_link_libraries(${TARGET_NAME} common_defs)

    if(${lib_name} MATCHES "F732" OR ${lib_name} MATCHES "F7")
        set(_STM32_FAMILY "F7")
    elseif(${lib_name} MATCHES "F407" OR ${lib_name} MATCHES "F4")
        set(_STM32_FAMILY "F4")
    elseif(${lib_name} MATCHES "G474" OR ${lib_name} MATCHES "G4")
        set(_STM32_FAMILY "G4")
    elseif(${lib_name} MATCHES "L432" OR ${lib_name} MATCHES "L4")
        set(_STM32_FAMILY "L4")
    else()
        set(_STM32_FAMILY "F4")
    endif()

    if(_STM32_FAMILY STREQUAL "F7")
        set(_ARCH_FLAGS -mthumb -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard)
        target_compile_definitions(${TARGET_NAME} PRIVATE STM32F732xx STM32F7xx)
    elseif(_STM32_FAMILY STREQUAL "F4")
        set(_ARCH_FLAGS -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
        target_compile_definitions(${TARGET_NAME} PRIVATE STM32F407xx STM32F4xx)
    elseif(_STM32_FAMILY STREQUAL "G4")
        set(_ARCH_FLAGS -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
        target_compile_definitions(${TARGET_NAME} PRIVATE STM32G474xx STM32G4xx)
    elseif(_STM32_FAMILY STREQUAL "L4")
        set(_ARCH_FLAGS -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
        if(${lib_name} MATCHES "L496")
            target_compile_definitions(${TARGET_NAME} PRIVATE STM32L496xx STM32L4xx)
        elseif(${lib_name} MATCHES "L471")
            target_compile_definitions(${TARGET_NAME} PRIVATE STM32L471xx STM32L4xx)
        else()
            target_compile_definitions(${TARGET_NAME} PRIVATE STM32L432xx STM32L4xx)
        endif()
    endif()

    target_compile_options(${TARGET_NAME} PRIVATE ${_ARCH_FLAGS})
    target_link_options(${TARGET_NAME} PRIVATE ${_ARCH_FLAGS})

    message(STATUS "[${lib_name}] STM32 family=${_STM32_FAMILY} ARCH=${_ARCH_FLAGS}")
ENDMACRO(MAKE_PHAL_LIBRARY)

MAKE_PHAL_LIBRARY(PHAL_L432 CMSIS_L432)
MAKE_PHAL_LIBRARY(PHAL_L471 CMSIS_L471)
MAKE_PHAL_LIBRARY(PHAL_L496 CMSIS_L496)
