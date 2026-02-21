
# Function to add a firmware component
# Usage:
# add_firmware_component(
#     NAME <name>
#     LINKER_SCRIPT <script_base_name>
#     LIBS <list_of_libs>
#     [IS_BOOTLOADER]
# )
function(add_firmware_component)
    set(options IS_BOOTLOADER)
    set(oneValueArgs NAME LINKER_SCRIPT OUTPUT_DIR)
    set(multiValueArgs LIBS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(TARGET_NAME ${ARG_NAME}.elf)
    add_executable(${TARGET_NAME})

    # Link common libs
    target_link_libraries(${TARGET_NAME} PRIVATE common_defs SYSCALLS ${ARG_LIBS})

    # Sources: include all .c files in current directory and subdirectories
    file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
    list(FILTER SOURCES EXCLUDE REGEX "test_.*|.*starter.*")
    target_sources(${TARGET_NAME} PRIVATE ${SOURCES})

    # Includes: current directory and all subdirectories
    target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    RECURSE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} "*.h" INCLUDE_DIRS)
    foreach(DIR ${INCLUDE_DIRS})
        target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${DIR})
    endforeach()

    # Linker script logic
    if(BOOTLOADER_BUILD AND NOT ARG_IS_BOOTLOADER)
        set(LS_SUFFIX "_APP.ld")
    elseif(ARG_IS_BOOTLOADER)
        set(LS_SUFFIX "_BL.ld")
    else()
        set(LS_SUFFIX ".ld")
    endif()
    
    target_link_options(${TARGET_NAME} PRIVATE -T${COMMON_SOURCE_DIR}/linker/${ARG_LINKER_SCRIPT}${LS_SUFFIX})

    # Post-build actions
    postbuild_target(${TARGET_NAME} ${ARG_NAME} "${ARG_OUTPUT_DIR}")
endfunction()
