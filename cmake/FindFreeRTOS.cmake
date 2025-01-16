# CMAKE file for building STM32CubeF4 FreeRTOS module

set(LIB_PATH "${CMAKE_SOURCE_DIR}/common/STM32CubeF4/Middlewares/Third_Party/FreeRTOS/Source")

function(make_freertos_library LIB_NAME)
    add_library(${LIB_NAME} STATIC)

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
    target_link_libraries(${LIB_NAME} CMSIS_F407)

endfunction()

# Create multiple libraries with different defines
make_freertos_library(FREERTOS_LIB)
