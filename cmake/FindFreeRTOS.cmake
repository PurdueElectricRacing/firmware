
function(make_freertos_library LIB_NAME DIRNAME LINK_NAME)
    add_library(${LIB_NAME} STATIC)
    set(LIB_PATH "${CMAKE_SOURCE_DIR}/external/${DIRNAME}/Middlewares/Third_Party/FreeRTOS/Source")

    target_include_directories(${LIB_NAME}
        PUBLIC ${LIB_PATH}/include
        PUBLIC ${LIB_PATH}/CMSIS_RTOS_V2
        PUBLIC ${LIB_PATH}/portable/GCC/ARM_CM4F
        PUBLIC ${CMAKE_SOURCE_DIR}/common/freertos
    )

    file(GLOB glob_sources "${LIB_PATH}/*.c" "${LIB_PATH}/CMSIS_RTOS_V2/*.c" "${LIB_PATH}/portable/GCC/ARM_CM4F/*.c" "${LIB_PATH}/portable/MemMang/heap_4.c")
    target_sources(${LIB_NAME}
        PUBLIC ${glob_sources}
    )
    target_link_libraries(${LIB_NAME} ${LINK_NAME})

endfunction()

# Create multiple libraries with different defines

make_freertos_library(FREERTOS_LIB_F407 STM32CubeF4 CMSIS_F407)
make_freertos_library(FREERTOS_LIB_F732 STM32CubeF7 CMSIS_F732)
make_freertos_library(FREERTOS_LIB_G474 STM32CubeG4 CMSIS_G474)
