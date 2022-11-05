#include "faults.h"
1

bool fault_lib_enable;

bool setFault(fault_struct_t *faultToFind, int valueToCompare) {
    if (!fault_lib_enable)
    {

    }
    else return false;
}

void killFaultLibrary() {
    fault_lib_enable = false;
}

void initFaultLibrary() {
    fault_lib_enable = true;
}