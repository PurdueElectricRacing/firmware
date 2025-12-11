cmake_minimum_required(VERSION 3.13)

# 1. Define the path to the generated directory
set(CAN_GEN_DIR ${CMAKE_CURRENT_LIST_DIR}/generated)

# 2. Define the command to run the generator
# We use a custom target to ensure this runs before compilation
# Note: We assume build.py generates can_common.c/h and can_<NODE>.c/h in the generated folder
add_custom_command(
    OUTPUT ${CAN_GEN_DIR}/can_common.c
    COMMAND python3 ${CMAKE_CURRENT_LIST_DIR}/generation/build.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    COMMENT "Generating CAN Library..."
    VERBATIM
)

# 3. Create a target that triggers the generation
add_custom_target(can_generation DEPENDS ${CAN_GEN_DIR}/can_common.c)

# 4. Create the Common Library
# This library contains definitions shared by all nodes
# We use INTERFACE libraries so sources are compiled with the target application's flags (Arch, Defines, etc.)
add_library(can_common INTERFACE)
target_sources(can_common INTERFACE ${CAN_GEN_DIR}/can_common.c)
add_dependencies(can_common can_generation)
target_include_directories(can_common INTERFACE ${CAN_GEN_DIR})
target_include_directories(can_common INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

# 5. Create Node-Specific Libraries (Macro for convenience)
macro(create_can_node_lib NODE_NAME)
    set(LIB_NAME "can_node_${NODE_NAME}")
    
    add_library(${LIB_NAME} INTERFACE)
    
    # Sources: The generated node code AND the hardware driver
    # They are bundled together so the driver can call generated functions (like init_filters)
    target_sources(${LIB_NAME} INTERFACE 
        ${CAN_GEN_DIR}/can_${NODE_NAME}.c
        ${CMAKE_CURRENT_LIST_DIR}/can_base.c
    )
    
    # Link to common definitions
    target_link_libraries(${LIB_NAME} INTERFACE can_common)
    
    # Ensure generation happens before this lib is built
    add_dependencies(${LIB_NAME} can_generation)
endmacro()

# 6. Instantiate the libraries for your known nodes
create_can_node_lib(A_BOX)
create_can_node_lib(DAQ)
create_can_node_lib(DASHBOARD)
create_can_node_lib(MAIN)
create_can_node_lib(PDU)
create_can_node_lib(TORQUE_VECTOR)
