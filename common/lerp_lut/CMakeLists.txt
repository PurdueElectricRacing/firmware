set(TARGET_NAME LERP_LUT)
add_library(${TARGET_NAME})

target_sources(${TARGET_NAME} PRIVATE "lerp_lut.c")

target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
