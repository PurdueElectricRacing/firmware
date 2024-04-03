# Helper for generating common CMake targets in the components directroy

function(postbuild_target TARGET_NAME)

    get_target_property(COMPONENT_NAME ${TARGET_NAME} COMPONENT_NAME)
    # Print out memory section usage
    # target_link_options(${TARGET_NAME} PUBLIC
    #     -Wl,--print-memory-usage
    # )

    if(BOOTLOADER_BUILD)
      set(OUTPUT_FILE_NAME BL_${COMPONENT_NAME})
    else()
      set(OUTPUT_FILE_NAME ${COMPONENT_NAME})
    endif()

    # Archive generated image and perform post-processing output
    get_target_property(COMPONENT_OUTPUT_DIR ${TARGET_NAME} OUTPUT_DIR)
    if (NOT COMPONENT_OUTPUT_DIR)
        set(COMPONENT_OUTPUT_DIR ${PROJECT_OUTPUT_DIR}/${COMPONENT_NAME})
    endif()

    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${TARGET_NAME} ${COMPONENT_OUTPUT_DIR}/${OUTPUT_FILE_NAME}.elf
        COMMENT "Archive target"
    )

    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND arm-none-eabi-objdump -xDSs ${TARGET_NAME} > ${COMPONENT_OUTPUT_DIR}/${OUTPUT_FILE_NAME}_info.txt
        COMMENT "Generating Sections & Disassembly Info..."
    )

    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND arm-none-eabi-objcopy -S -O ihex --gap-fill 0 ${TARGET_NAME} ${COMPONENT_OUTPUT_DIR}/${OUTPUT_FILE_NAME}.hex
        COMMENT "Generateing HEX file"
    )

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

MACRO(RECURSE_DIRECTORIES curdir search_term return_list)
    FILE(GLOB_RECURSE new_list RELATIVE ${curdir} ${search_term})
    SET(dir_list "")
    FOREACH(file_path ${new_list})
        GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
        SET(dir_list ${dir_list} ${dir_path})
    ENDFOREACH()
    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()
