cmake_minimum_required(VERSION 3.13)

# Setup Component name based on directory
get_filename_component(COMPONENT_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
STRING(TOLOWER ${COMPONENT_NAME} COMPONENT_NAME)

set(TARGET_NAME ${COMPONENT_NAME}.elf)
add_executable(${TARGET_NAME})

# Add in bsxlite
target_link_libraries(${TARGET_NAME} ${CMAKE_CURRENT_LIST_DIR}/bsxlite/libalgobsx.a)

# Propreties are set in order to make the common component
set_target_properties(${TARGET_NAME} PROPERTIES
    COMPONENT_NAME ${COMPONENT_NAME}
    COMPONENT_DIR  ${CMAKE_CURRENT_LIST_DIR}
    LINKER_SCRIPT  "STM32F732RETX_FLASH"
    COMMON_LIBS    "CMSIS_F732;PSCHED_F732;QUEUE;PHAL_F732;libm.a;"
)
COMMON_FIRMWARE_COMPONENT(${TARGET_NAME})
