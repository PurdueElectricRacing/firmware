import sys
from validator import validate_all
from parser import parse_all
from linker import link_all
from dbcgen import generate_debug
from codegen import generate_headers

def build():
    if not validate_all():
        sys.exit(1)
    
    try:
        nodes = parse_all()
        
        link_all(nodes)
        
    except ValueError as e:
        print(f"Error: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"Unexpected Error: {e}")
        sys.exit(1)

    generate_debug(nodes)
    generate_headers(nodes)
    # TODO dbcgen

if __name__ == "__main__":
    build()
