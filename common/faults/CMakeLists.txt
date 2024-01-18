MACRO(MAKE_FLT_LIB lib_name lib_link hal_name)
    set(TARGET_NAME ${lib_name})
    add_library(${TARGET_NAME})

    # Find all .c sources in project
    file(GLOB_RECURSE glob_sources "*.c")
    target_sources(${TARGET_NAME} PRIVATE ${glob_sources})

    # Find directories for '#include'
    # For libraries, these directories are all referenced to the top level firmware directory, CMAKE_SOURCE_DIR
    target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_SOURCE_DIR})

    target_link_libraries(${TARGET_NAME} ${lib_link})
    target_link_libraries(${TARGET_NAME} ${hal_name})

ENDMACRO(MAKE_FLT_LIB)

MAKE_FLT_LIB(FAULTS CMSIS_L432 PHAL_L432)
MAKE_FLT_LIB(FAULTS_L496 CMSIS_L496 PHAL_L496)
MAKE_FLT_LIB(FAULTS_F407 CMSIS_F407 PHAL_F407)
MAKE_FLT_LIB(FAULTS_F732 CMSIS_F732 PHAL_F732)
