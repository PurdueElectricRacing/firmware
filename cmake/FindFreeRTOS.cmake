# CMAKE file for building STM32CubeL4 CMSIS module

set(LIB_PATH "${CMAKE_SOURCE_DIR}/common/STM32CubeL4/Middlewares/Third_Party/FreeRTOS/Source")

function(make_freertos_library LIB_NAME)
    add_library(${LIB_NAME} STATIC)

    target_include_directories(${LIB_NAME} 
        PUBLIC ${LIB_PATH}/include
        PUBLIC ${LIB_PATH}/CMSIS_RTOS_V2
        PUBLIC ${LIB_PATH}/portable/GCC/ARM_CM4F
    )

    file(GLOB glob_sources "${LIB_PATH}/*.c" "${LIB_PATH}/CMSIS_RTOS/*.c" "${LIB_PATH}/portable/GCC/ARM_CM4F/*.c")
    target_sources(${LIB_NAME} 
        PUBLIC ${glob_sources}
    )
endfunction()

# Create multiple libraries with different defines
make_freertos_library(FREERTOS_LIB)
