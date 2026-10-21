# Function to add a firmware component
# Usage:
# add_firmware_component(
#     NAME <name>
#     LINKER_SCRIPT <script_base_name>
#     LIBS <list_of_libs>
#     [DEFINES <list_of_defines>]
#     [OUTPUT_DIR <output_directory>]
#     [BUILD_BOOTLOADABLE | IS_BOOTLOADER]
# )
#
# Bootloader:
# - By default: builds a standalone image by default
# - BUILD_BOOTLOADABLE: also builds bootloadable_<name>
# - IS_BOOTLOADER: builds a resident bootloader
function(add_firmware_component)
    set(options IS_BOOTLOADER BUILD_BOOTLOADABLE)
    set(oneValueArgs NAME LINKER_SCRIPT OUTPUT_DIR)
    set(multiValueArgs LIBS DEFINES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(ARG_IS_BOOTLOADER AND ARG_BUILD_BOOTLOADABLE)
        message(FATAL_ERROR "A resident bootloader cannot have a bootloadable variant")
    endif()

    # Collect sources and header directories
    file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
    list(FILTER SOURCES EXCLUDE REGEX "test_.*|.*starter.*")
    RECURSE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} "*.h" INCLUDE_DIRS)
    list(TRANSFORM INCLUDE_DIRS PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

    # Add the optional bootloadable variant
    set(IMAGE_NAMES ${ARG_NAME})
    if(ARG_BUILD_BOOTLOADABLE)
        list(APPEND IMAGE_NAMES bootloadable_${ARG_NAME})
    endif()

    foreach(IMAGE_NAME IN LISTS IMAGE_NAMES)
        set(TARGET_NAME ${IMAGE_NAME}.elf)
        add_executable(${TARGET_NAME} ${SOURCES})
        target_link_libraries(${TARGET_NAME} PRIVATE SYSCALLS ${ARG_LIBS})
        target_include_directories(${TARGET_NAME} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_SOURCE_DIR}
            ${INCLUDE_DIRS}
        )
        target_compile_definitions(${TARGET_NAME} PRIVATE ${ARG_DEFINES})

        # Select the linker layout and bootloader defines
        if(ARG_IS_BOOTLOADER)
            set(LS_SUFFIX "_BL.ld")
            target_compile_definitions(${TARGET_NAME} PRIVATE BOOTLOADER_FIRMWARE=1)
        elseif(IMAGE_NAME STREQUAL "bootloadable_${ARG_NAME}")
            set(LS_SUFFIX "_APP.ld")
            target_compile_definitions(${TARGET_NAME} PRIVATE BOOTLOADER_ENABLED=1)
        else()
            set(LS_SUFFIX ".ld")
        endif()

        set(MAP_FILE ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.map)
        target_link_options(${TARGET_NAME} PRIVATE
            -T${SUPPORT_DIR}/linker/${ARG_LINKER_SCRIPT}${LS_SUFFIX}
            "-Wl,-Map=${MAP_FILE}"
        )
        # Archive ELF, HEX, BIN, and map files
        postbuild_target(${TARGET_NAME} ${IMAGE_NAME} "${ARG_OUTPUT_DIR}" ${MAP_FILE})
    endforeach()
endfunction()
