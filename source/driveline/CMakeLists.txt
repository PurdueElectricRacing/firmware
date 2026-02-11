cmake_minimum_required(VERSION 3.13)

# Create the CAN node library for this component (shared by both sides)
create_can_node_lib(DRIVELINE "CMSIS_G474;FREERTOS_G474")

foreach(SIDE front rear)
    set(TARGET_NAME ${SIDE}_driveline.elf)
    set(COMPONENT_NAME ${SIDE}_driveline)
    add_executable(${TARGET_NAME})

    # Pass the location define to the compiler
    string(TOUPPER ${SIDE} SIDE_UPPER)
    target_compile_definitions(${TARGET_NAME} PRIVATE IS_${SIDE_UPPER}_DRIVELINE)

    # Properties are set in order to make the common component
    set_target_properties(${TARGET_NAME} PROPERTIES
        COMPONENT_NAME ${COMPONENT_NAME}
        COMPONENT_DIR  ${CMAKE_CURRENT_LIST_DIR}
        LINKER_SCRIPT  "STM32G474RETX_FLASH"
        COMMON_LIBS    "can_node_DRIVELINE;CMSIS_G474;FREERTOS_G474;PHAL_G474;"
    )
    COMMON_FIRMWARE_COMPONENT(${TARGET_NAME})
endforeach()

# Add a dummy target for 'per_build.py -t driveline' to build both
add_custom_target(driveline.elf DEPENDS front_driveline.elf rear_driveline.elf)
