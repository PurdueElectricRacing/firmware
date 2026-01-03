cmake_minimum_required(VERSION 3.13)

# 1. Define the path to the generated directory
set(CAN_GEN_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

# 2. Define the command to run the generator
# We use can_router.h as the sentinel file for generation
add_custom_command(
	OUTPUT ${CAN_GEN_DIR}/can_router.h
	COMMAND python3 ${CMAKE_CURRENT_LIST_DIR}/canpiler/build.py
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	COMMENT "Generating CAN Library..."
	VERBATIM
)

# 3. Create a target that triggers the generation
add_custom_target(can_generation DEPENDS ${CAN_GEN_DIR}/can_router.h)

# 4. Create the Common Headers Library
# This provides the include paths for the generated headers and the project root
add_library(can_common_headers INTERFACE)
target_include_directories(can_common_headers INTERFACE 
	${CAN_GEN_DIR}
	${CMAKE_SOURCE_DIR}
)
add_dependencies(can_common_headers can_generation)

# 5. Create Node-Specific Libraries
# Each node gets its own static library which compiles can_common.c 
# with a node-specific define.
macro(create_can_node_lib NODE_NAME)
	set(LIB_NAME "can_node_${NODE_NAME}")
	
	# We use a static library so each node has its own object file for can_common.c
	add_library(${LIB_NAME} STATIC ${CMAKE_CURRENT_LIST_DIR}/can_common.c)
	
	# Set the node-specific define so can_router.h knows which header to include
	target_compile_definitions(${LIB_NAME} PRIVATE "NODE_${NODE_NAME}")
	
	# Link to common headers and project root
	target_link_libraries(${LIB_NAME} PUBLIC can_common_headers)
	
	# Ensure generation happens before compilation
	add_dependencies(${LIB_NAME} can_generation)
endmacro()

# 6. Instantiate the libraries for your known nodes
create_can_node_lib(A_BOX)
create_can_node_lib(DAQ)
create_can_node_lib(DASHBOARD)
create_can_node_lib(MAIN)
create_can_node_lib(PDU)
create_can_node_lib(TORQUE_VECTOR)
