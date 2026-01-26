set(TARGET_NAME STRBUF)
add_library(${TARGET_NAME})

target_sources(${TARGET_NAME} PRIVATE "strbuf.c")

target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
