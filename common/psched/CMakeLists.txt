cmake_minimum_required(VERSION 3.13)

macro(add_psched_library lib_name lib_link)
    add_library(${lib_name} INTERFACE)
    target_sources(${lib_name} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/psched.c")
    target_include_directories(${lib_name} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(${lib_name} INTERFACE ${lib_link})
endmacro()

# Set up for F407
add_psched_library(PSCHED_F407 CMSIS_F407)

#Set up for F732
add_psched_library(PSCHED_F732 CMSIS_F732)

#Set up for G474
add_psched_library(PSCHED_G474 CMSIS_G474)
target_compile_definitions(PSCHED_G474 INTERFACE PSCHED_USE_TIM7=1)

