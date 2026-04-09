# Bootloader build configuration
# Each node gets its own bootloader binary

# G4 Bootloader nodes
set(G4_BL_NODES
    MAIN_MODULE
    DASHBOARD
    A_BOX
    TORQUE_VECTOR
    DRIVELINE
    G4_TESTING
)

# F4 Bootloader nodes
set(F4_BL_NODES
    PDU
)

# Create bootloader for each G4 node
foreach(NODE ${G4_BL_NODES})
    # Create canpiler node library for bootloader
    create_can_node_lib(BL_${NODE} "CMSIS_G474;FREERTOS_G474")

    # Build bootloader binary
    add_firmware_component(
        NAME bootloader_${NODE}
        LINKER_SCRIPT "STM32G474RETX_FLASH"
        LIBS "can_node_BL_${NODE};CMSIS_G474;PHAL_G474;BOOTLOADER_COMMON"
        IS_BOOTLOADER
    )

    # Set APP_ID for this node
    string(TOUPPER ${NODE} NODE_UPPER)
    target_compile_definitions(bootloader_${NODE}.elf PRIVATE APP_ID=APP_${NODE_UPPER})
endforeach()

# Create bootloader for each F4 node
foreach(NODE ${F4_BL_NODES})
    # Create canpiler node library for bootloader
    create_can_node_lib(BL_${NODE} "CMSIS_F407;FREERTOS_F407")

    # Build bootloader binary
    add_firmware_component(
        NAME bootloader_${NODE}
        LINKER_SCRIPT "STM32F407VGTx_FLASH"
        LIBS "can_node_BL_${NODE};CMSIS_F407;PHAL_F407;BOOTLOADER_COMMON"
        IS_BOOTLOADER
    )

    # Set APP_ID for this node
    string(TOUPPER ${NODE} NODE_UPPER)
    target_compile_definitions(bootloader_${NODE}.elf PRIVATE APP_ID=APP_${NODE_UPPER})
endforeach()

# Dummy target for building all bootloaders
add_custom_target(bootloader.elf DEPENDS
    bootloader_MAIN_MODULE.elf
    bootloader_DASHBOARD.elf
    bootloader_A_BOX.elf
    bootloader_TORQUE_VECTOR.elf
    bootloader_DRIVELINE.elf
    bootloader_G4_TESTING.elf
    bootloader_PDU.elf
)
