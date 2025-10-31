#ifndef ADBMS_FAULTS
#define ADBMS_FAULTS

typedef enum
{
    BMS_ERROR_CAN = 0,  // CAN RX/TX
    BMS_ERROR_ELCON,
    BMS_ERROR_CHARGER_PORT,

    BMS_ERROR_SID,          // Device ID
    BMS_ERROR_RXPEC,        // RX PEC mismatch
    BMS_ERROR_CONFIG,       // Config TX failed
    BMS_ERROR_POLL_TIMEOUT, // Poll Timeout

    BMS_ERROR_VPV,       // V+ to V−
    BMS_ERROR_VMV,       // S1N to V−
    BMS_ERROR_VA_UV,     // Analog power undervoltage
    BMS_ERROR_VA_OV,     // Analog power overvoltage
    BMS_ERROR_VD_UV,     // Digital power undervoltage
    BMS_ERROR_VD_OV,     // Digital power overvoltage
    BMS_ERROR_VREG,      // Regulated Power
    BMS_ERROR_VREF2,     // Vref2 for thermistors
    BMS_ERROR_ITMP_UT,   // Internal die temperature under temperature
    BMS_ERROR_ITMP_OT,   // Internal die temperature over temperature
    BMS_ERROR_PACK_WEAK, // Under/Overvoltage module

    BMS_ERROR_CELL_OW,    // Cell open-wire
    BMS_ERROR_CELL_UV,    // Cell undervoltage
    BMS_ERROR_CELL_OV,    // Cell overvoltage
    BMS_ERROR_CELL_REDUN, // Cell redundant measurement
    BMS_ERROR_CELL_WEAK,  // Under/Overvoltage cell in module

    BMS_ERROR_AUX_OW,     // AUX open-wire
    BMS_ERROR_AUX_UT,     // AUX under temperature
    BMS_ERROR_AUX_OT,     // AUX over temperature
    BMS_ERROR_AUX_REDUN,  // AUX over temperature

    BMS_ERROR_COUNT,
} adbms_fault_t;


#endif