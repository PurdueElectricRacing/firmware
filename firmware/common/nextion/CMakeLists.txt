set(TARGET_NAME NEXTION)
add_library(${TARGET_NAME} INTERFACE)

target_sources(${TARGET_NAME} INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/nextion.c"
)

target_include_directories(${TARGET_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${TARGET_NAME} INTERFACE
    STRBUF
)