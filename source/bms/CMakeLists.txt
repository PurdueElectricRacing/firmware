cmake_minimum_required(VERSION 3.13)


set(BMS_NAMES bms_a bms_b bms_c bms_d bms_e bms_f bms_g bms_h)

foreach(BMS_NAME IN ITEMS ${BMS_NAMES})

    set(TARGET_NAME ${BMS_NAME}.elf)
    add_executable(${TARGET_NAME})

    STRING(TOUPPER ${BMS_NAME} BMS_NAME_UPPER)
    target_compile_definitions(${TARGET_NAME} PUBLIC BMS_NODE_NAME=${BMS_NAME_UPPER})

    # Propreties are set in order to make the common component
    set_target_properties(${TARGET_NAME} PROPERTIES 
        COMPONENT_NAME ${BMS_NAME}
        COMPONENT_DIR  ${CMAKE_CURRENT_LIST_DIR}
        LINKER_SCRIPT  "STM32L432KCUx_FLASH"
        COMMON_LIBS    "CMSIS_L432;PSCHED;QUEUE;PHAL_L432"
        OUTPUT_DIR     ${PROJECT_OUTPUT_DIR}/bmses/${BMS_NAME}
    )
    COMMON_FIRMWARE_COMPONENT(${TARGET_NAME})
    MESSAGE(STATUS, "bms building")
endforeach()