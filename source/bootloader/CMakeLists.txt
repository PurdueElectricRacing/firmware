# Setup Component name based on directory
get_filename_component(COMPONENT_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
STRING(TOLOWER ${COMPONENT_NAME} COMPONENT_NAME)


set(F407_BOOTLOADER_NAMES dashboard main_module pdu a_box f4_testing daq)
set(F732_BOOTLOADER_NAMES torquevector f7_testing)

if (BOOTLOADER_BUILD)
    foreach(BL_NAME IN ITEMS ${F407_BOOTLOADER_NAMES})
        STRING(TOUPPER ${BL_NAME} BL_NAME_UPPER)
        add_firmware_component(
            NAME ${COMPONENT_NAME}_${BL_NAME}
            LINKER_SCRIPT "STM32F407VGTx_FLASH"
            LIBS "CMSIS_F407;PSCHED_F407;QUEUE;PHAL_F407;BOOTLOADER_COMMON_F407"
            OUTPUT_DIR ${PROJECT_OUTPUT_DIR}/bootloaders/${COMPONENT_NAME}_${BL_NAME}
            IS_BOOTLOADER
        )
        target_compile_definitions(${COMPONENT_NAME}_${BL_NAME}.elf PUBLIC APP_ID=APP_${BL_NAME_UPPER})
    endforeach()

    foreach(BL_NAME IN ITEMS ${F732_BOOTLOADER_NAMES})
        STRING(TOUPPER ${BL_NAME} BL_NAME_UPPER)
        add_firmware_component(
            NAME ${COMPONENT_NAME}_${BL_NAME}
            LINKER_SCRIPT "STM32F732RETX_FLASH"
            LIBS "CMSIS_F732;PSCHED_F732;QUEUE;PHAL_F732;BOOTLOADER_COMMON_F732"
            OUTPUT_DIR ${PROJECT_OUTPUT_DIR}/bootloaders/${COMPONENT_NAME}_${BL_NAME}
            IS_BOOTLOADER
        )
        target_compile_definitions(${COMPONENT_NAME}_${BL_NAME}.elf PUBLIC APP_ID=APP_${BL_NAME_UPPER})
    endforeach()
endif()
