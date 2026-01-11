import sys
from validator import validate_all
from parser import parse_all, parse_faults
from linker import link_all
from mapper import map_hardware
from dbcgen import generate_debug, generate_dbcs
from codegen import generate_headers
from utils import load_json, BUS_CONFIG_PATH

def build():
    if not validate_all():
        sys.exit(1)
    
    try:
        nodes = parse_all()
        fault_modules = parse_faults()
        link_all(nodes)
        
        # Load bus configs for mapper and codegen
        bus_configs = load_json(BUS_CONFIG_PATH)
        busses = {b['name']: b for b in bus_configs['busses']}
        
        mappings = map_hardware(nodes, busses)
        
    except ValueError as e:
        print(f"Error: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"Unexpected Error: {e}")
        sys.exit(1)

    generate_debug(nodes)
    generate_headers(nodes, mappings, fault_modules)
    generate_dbcs(nodes)

if __name__ == "__main__":
    build()
