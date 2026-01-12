from typing import List
from parser import SystemContext, FaultModule
from utils import GENERATED_DIR, print_as_success, print_as_ok

def generate_fault_data(context: SystemContext):
    print("Generating fault library data...")
    fault_modules = context.fault_modules
    generate_fault_header(fault_modules)
    generate_fault_source(fault_modules)
    print_as_success("Successfully generated fault library data")

def generate_fault_header(fault_modules: List[FaultModule]):
    filename = GENERATED_DIR / "fault_data.h"
    total_faults = sum(len(m.faults) for m in fault_modules)

    with open(filename, 'w') as f:
        f.write("#ifndef FAULT_DATA_H\n")
        f.write("#define FAULT_DATA_H\n\n")
        f.write("#include \"common/faults/faults.h\"\n\n")

        f.write("// Total counts\n")
        for m in fault_modules:
            f.write(f"#define TOTAL_{m.macro_name}_FAULTS {len(m.faults)}\n")
        f.write(f"#define TOTAL_NUM_FAULTS {total_faults}\n\n")

        f.write("// Accessor Macros\n")
        f.write("#define GET_IDX(id) (id & 0xFFF)\n")
        f.write("#define GET_OWNER(id) (id >> 12)\n\n")

        f.write("// Fault IDs\n")
        for m in fault_modules:
            for fault in m.faults:
                f.write(f"#define ID_{fault.macro_name}_FAULT 0x{fault.id:x}\n")
        f.write("\n")

        f.write("// Latch times (ms)\n")
        for m in fault_modules:
            for fault in m.faults:
                f.write(f"#define {fault.macro_name}_LATCH_TIME {fault.time_to_latch}\n")
                f.write(f"#define {fault.macro_name}_UNLATCH_TIME {fault.time_to_unlatch}\n")
        f.write("\n")

        f.write("// Priorities\n")
        for m in fault_modules:
            for fault in m.faults:
                f.write(f"#define {fault.macro_name}_PRIORITY FAULT_{fault.priority.upper()}\n")
        f.write("\n")

        f.write("// Messages\n")
        for m in fault_modules:
            for fault in m.faults:
                f.write(f"#define {fault.macro_name}_MSG \"{fault.lcd_message}\"\n")
        f.write("\n")

        f.write("extern uint16_t faultLatchTime[TOTAL_NUM_FAULTS];\n")
        f.write("extern uint16_t faultULatchTime[TOTAL_NUM_FAULTS];\n")
        f.write("extern fault_status_t statusArray[TOTAL_NUM_FAULTS];\n")
        f.write("extern fault_attributes_t faultArray[TOTAL_NUM_FAULTS];\n\n")

        f.write("#endif\n")
    print_as_ok(f"Generated {filename.name}")

def generate_fault_source(all_modules: List[FaultModule]):
    filename = GENERATED_DIR / "fault_data.c"
    
    with open(filename, 'w') as f:
        f.write("#include \"fault_data.h\"\n\n")

        # Latch Time Array
        f.write(f"uint16_t faultLatchTime[TOTAL_NUM_FAULTS] = {{\n")
        for m in all_modules:
            for fault in m.faults:
                f.write(f"\t{fault.macro_name}_LATCH_TIME,\n")
        f.write("};\n\n")

        # Unlatch Time Array
        f.write(f"uint16_t faultULatchTime[TOTAL_NUM_FAULTS] = {{\n")
        for m in all_modules:
            for fault in m.faults:
                f.write(f"\t{fault.macro_name}_UNLATCH_TIME,\n")
        f.write("};\n\n")

        # Status Array
        f.write(f"fault_status_t statusArray[TOTAL_NUM_FAULTS] = {{\n")
        for m in all_modules:
            for fault in m.faults:
                f.write(f"\t{{false, ID_{fault.macro_name}_FAULT}},\n")
        f.write("};\n\n")

        # Attributes Array
        f.write(f"fault_attributes_t faultArray[TOTAL_NUM_FAULTS] = {{\n")
        global_idx = 0
        for m in all_modules:
            for fault in m.faults:
                f.write(f"\t{{false, false, {fault.macro_name}_PRIORITY, 0, 0, {fault.max_val}, {fault.min_val}, &statusArray[{global_idx}], 0, {fault.macro_name}_MSG}},\n")
                global_idx += 1
        f.write("};\n")

    print_as_ok(f"Generated {filename.name}")
