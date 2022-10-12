cmake_minimum_required(VERSION 3.13)

# Setup Component name based on directory
get_filename_component(COMPONENT_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
STRING(TOLOWER ${COMPONENT_NAME} COMPONENT_NAME)


set(L432_BOOTLOADER_NAMES dashboard driveline_f driveline_r bms_a bms_b bms_c bms_d bms_e bms_f bms_g bms_h bms_i bms_j torquevector l4_testing)
set(L496_BOOTLOADER_NAMES precharge main_module)
if (BOOTLOADER_BUILD)
    foreach(BL_NAME IN ITEMS ${L432_BOOTLOADER_NAMES})
        STRING(TOUPPER ${BL_NAME} BL_NAME_UPPER)
        # Setup Component name based on directory
        set(TARGET_NAME ${COMPONENT_NAME}_${BL_NAME}.elf)
        add_executable(${TARGET_NAME})
        target_compile_definitions(${TARGET_NAME} PUBLIC APP_ID=APP_${BL_NAME_UPPER})

        # Propreties are set in order to make the common component
        set_target_properties(${TARGET_NAME} PROPERTIES 
            COMPONENT_NAME ${COMPONENT_NAME}_${BL_NAME}
            COMPONENT_DIR  ${CMAKE_CURRENT_LIST_DIR}
            LINKER_SCRIPT  "STM32L432KCUx_FLASH"
            COMMON_LIBS    "CMSIS_L432;PSCHED;QUEUE;PHAL_L432;BOOTLOADER_COMMON_L432"
            OUTPUT_DIR     ${PROJECT_OUTPUT_DIR}/bootloaders/${COMPONENT_NAME}_${BL_NAME}
        )
        COMMON_BOOTLOADER_COMPONENT(${TARGET_NAME})
    endforeach()

    foreach(BL_NAME IN ITEMS ${L496_BOOTLOADER_NAMES})
        STRING(TOUPPER ${BL_NAME} BL_NAME_UPPER)
        # Setup Component name based on directory
        set(TARGET_NAME ${COMPONENT_NAME}_${BL_NAME}.elf)
        add_executable(${TARGET_NAME})
        target_compile_definitions(${TARGET_NAME} PUBLIC APP_ID=APP_${BL_NAME_UPPER})

        # Propreties are set in order to make the common component
        set_target_properties(${TARGET_NAME} PROPERTIES 
            COMPONENT_NAME ${COMPONENT_NAME}_${BL_NAME}
            COMPONENT_DIR  ${CMAKE_CURRENT_LIST_DIR}
            LINKER_SCRIPT  "STM32L496VGTx_FLASH"
            COMMON_LIBS    "CMSIS_L496;PSCHED_L496;QUEUE;PHAL_L496;BOOTLOADER_COMMON_L496"
            OUTPUT_DIR     ${PROJECT_OUTPUT_DIR}/bootloaders/${COMPONENT_NAME}_${BL_NAME}
        )
        COMMON_BOOTLOADER_COMPONENT(${TARGET_NAME})
    endforeach()
endif()
