"""
build.py

Author: Irving Wang (irvingw@purdue.edu)
"""

import sys
from validator import validate_all
from parser import parse_all, load_custom_types, create_system_context
from linker import link_all
from mapper import map_hardware
from dbcgen import generate_dbcs
from codegen import generate_headers
from faultgen import generate_fault_data
from load_calc import calculate_bus_load
from utils import load_json, BUS_CONFIG_PATH, GENERATED_DIR, print_as_success

def build():
    if not validate_all():
        sys.exit(1)
    
    # Clean generated directory to remove stale files
    if GENERATED_DIR.exists():
        for f in GENERATED_DIR.glob("*"):
            if f.is_file():
                f.unlink()
    GENERATED_DIR.mkdir(exist_ok=True)

    try:
        nodes = parse_all()
        
        # Load bus configs and custom types
        bus_configs = load_json(BUS_CONFIG_PATH)
        custom_types = load_custom_types()

        # Fault system middleware (B) - enriches nodes and types in one pass
        from faultgen import augment_system_with_faults
        augment_system_with_faults(nodes, bus_configs, custom_types)

        link_all(nodes)
            
        # Mapper needs dict of configs
        busses = {b['name']: b for b in bus_configs['busses']}
        mappings = map_hardware(nodes, busses)
        
        # Create the unified context (A) - derives fault modules automatically from nodes
        context = create_system_context(nodes, mappings, busses, custom_types)
        
    except ValueError as e:
        print(f"Error: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"Unexpected Error: {e}")
        sys.exit(1)

    # All generators now consume the context
    generate_headers(context)
    if context.fault_modules:
        generate_fault_data(context)
    generate_dbcs(context)

    # Final Analysis
    calculate_bus_load(context)
    
    print_as_success("CAN library generation complete")

if __name__ == "__main__":
    build()
