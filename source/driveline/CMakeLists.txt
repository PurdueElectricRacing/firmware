cmake_minimum_required(VERSION 3.13)

# Setup Component name based on directory
get_filename_component(COMPONENT_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
STRING(TOLOWER ${COMPONENT_NAME} COMPONENT_NAME)

# Create a component for the front and rear variants of the driveline code
MAKE_COMMON_COMPONENT(${COMPONENT_NAME}_front ${CMAKE_CURRENT_LIST_DIR} "PSCHED;QUEUE;PHAL_L4" )
target_compile_definitions(${COMPONENT_NAME}_front PUBLIC FTR_DRIVELINE_REAR=1 FTR_DRIVELINE_FRONT=0)

MAKE_COMMON_COMPONENT(${COMPONENT_NAME}_rear ${CMAKE_CURRENT_LIST_DIR} "PSCHED;QUEUE;PHAL_L4")
target_compile_definitions(${COMPONENT_NAME}_rear  PUBLIC FTR_DRIVELINE_REAR=0 FTR_DRIVELINE_FRONT=1)
