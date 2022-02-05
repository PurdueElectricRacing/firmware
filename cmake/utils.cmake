# Helper for generating common CMake targets in the components directroy

function(postbuild_target COMPONENT_NAME TARGET_NAME)

    # Print out memory section usage
    target_link_options(${TARGET_NAME} PUBLIC
        -Wl,--print-memory-usage
    )

    # Archive generated image and perform post-processing output
    set(COMPONENT_OUTPUT_DIR ${PROJECT_OUTPUT_DIR}/${COMPONENT_NAME})
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${TARGET_NAME} ${COMPONENT_OUTPUT_DIR}/${TARGET_NAME}
        COMMENT "Archive target"
    )

    find_program(HAS_OBJDUMP arm-none-eabi-objdump)
    if (HAS_HAS_OBJDUMP)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND arm-none-eabi-objdump -xDSs ${COMPONENT_OUTPUT_DIR}/${TARGET_NAME} > ${COMPONENT_OUTPUT_DIR}/${COMPONENT_NAME}_info.txt
        COMMENT "Generating Sections & Disassembly Info..."
    )
    endif()

    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND cmake -E echo 
        COMMENT "Formatting"
    )

    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND arm-none-eabi-size ${TARGET_NAME} 
        COMMENT "Binary Output Size"
    )

endfunction()

MACRO(SUBDIRLIST curdir result)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()