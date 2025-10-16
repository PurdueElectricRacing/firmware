macro(MAKE_FREERTOS_WRAPPER LIB_NAME FREERTOS_CORE)
    set(TARGET_NAME ${LIB_NAME})
    add_library(${TARGET_NAME} STATIC)

    file(GLOB_RECURSE glob_sources "*.c")
    target_sources(${TARGET_NAME} PRIVATE ${glob_sources})

    target_include_directories(${TARGET_NAME} PUBLIC
        ${CMAKE_SOURCE_DIR}/common/freertos
        ${CMAKE_SOURCE_DIR}
    )

    target_link_libraries(${TARGET_NAME} PUBLIC ${FREERTOS_CORE})

    if(${LIB_NAME} MATCHES "F732" OR ${LIB_NAME} MATCHES "F7")
        set(_ARCH_FLAGS -mthumb -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard)
        target_compile_definitions(${TARGET_NAME} PRIVATE STM32F732xx STM32F7xx)
    elseif(${LIB_NAME} MATCHES "F407" OR ${LIB_NAME} MATCHES "F4")
        set(_ARCH_FLAGS -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
        target_compile_definitions(${TARGET_NAME} PRIVATE STM32F407xx STM32F4xx)
    elseif(${LIB_NAME} MATCHES "G474" OR ${LIB_NAME} MATCHES "G4")
        set(_ARCH_FLAGS -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
        target_compile_definitions(${TARGET_NAME} PRIVATE STM32G474xx STM32G4xx)
    elseif(${LIB_NAME} MATCHES "L496" OR ${LIB_NAME} MATCHES "L4")
        set(_ARCH_FLAGS -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard)
        target_compile_definitions(${TARGET_NAME} PRIVATE STM32L496xx STM32L4xx)
    endif()

    target_compile_options(${TARGET_NAME} PRIVATE ${_ARCH_FLAGS})
    target_link_options(${TARGET_NAME} PRIVATE ${_ARCH_FLAGS})
endmacro()

MAKE_FREERTOS_WRAPPER(FREERTOS_F407 FREERTOS_LIB_F407)
MAKE_FREERTOS_WRAPPER(FREERTOS_F732 FREERTOS_LIB_F732)
MAKE_FREERTOS_WRAPPER(FREERTOS_G474 FREERTOS_LIB_G474)
MAKE_FREERTOS_WRAPPER(FREERTOS_L496 FREERTOS_LIB_L496)
