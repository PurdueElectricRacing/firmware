# Bootloader common library - shared between bootloader and application
set(TARGET_NAME BOOTLOADER_COMMON)
add_library(${TARGET_NAME} INTERFACE)

target_sources(${TARGET_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/bootloader_common.c)
target_include_directories(${TARGET_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
