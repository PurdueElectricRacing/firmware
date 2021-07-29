# Helper for generating common CMake targets in the components directroy

function(postbuild_target COMPONENT_NAME)

    # Archive generated image and perform post-processing output
    set(COMPONENT_OUTPUT_DIR ${PROJECT_OUTPUT_DIR}/${COMPONENT_NAME})
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${TARGET_NAME} ${COMPONENT_OUTPUT_DIR}/${TARGET_NAME}
        COMMENT "Archive target"
    )

    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND arm-none-eabi-objdump -xDSs ${COMPONENT_OUTPUT_DIR}/${TARGET_NAME} > ${COMPONENT_OUTPUT_DIR}/${COMPONENT_NAME}_info.txt
        COMMENT "Generating Sections & Disassembly Info..."
    )

    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND arm-none-eabi-size ${TARGET_NAME} 
        COMMENT "Binary Output Size"
    )

endfunction()