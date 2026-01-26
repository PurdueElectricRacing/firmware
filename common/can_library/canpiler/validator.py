"""
validator.py

Author: Irving Wang (irvingw@purdue.edu)
"""

import sys
from jsonschema import RefResolver, Draft7Validator, ValidationError
from utils import load_json, SCHEMA_DIR, COMMON_TYPES_CONFIG_PATH, BUS_CONFIG_PATH, NODE_CONFIG_DIR, EXTERNAL_NODE_CONFIG_DIR, print_as_error, print_as_ok, print_as_warning, print_as_success

def validate_against_schema(data, schema, schema_store=None, base_uri='', filename="<unknown>") -> bool:
    """
    Validate data against schema and return success status.
    Prints all validation errors found in the file.
    """
    resolver = RefResolver(base_uri, schema, store=schema_store) if schema_store else None
    validator = Draft7Validator(schema, resolver=resolver)

    errors = sorted(validator.iter_errors(data), key=lambda e: str(e.path))
    
    if not errors:
        return True

    print_as_warning(f"Errors in {filename}:")
    for error in errors:
        path = ".".join(map(str, error.path)) or "root"
        print_as_error(f"Field '{path}': {error.message}")
    
    return False

def validate_common_types() -> bool:
    type_registry_schema = load_json(SCHEMA_DIR / 'type_registry.json')
    common_types = load_json(COMMON_TYPES_CONFIG_PATH)
    
    if validate_against_schema(common_types, type_registry_schema, filename='common_types.json'):
        print_as_ok("common_types.json")
        return True
    return False
    
def validate_bus_config(schema_store) -> bool:
    bus_schema = load_json(SCHEMA_DIR / 'bus_schema.json')
    buses = load_json(BUS_CONFIG_PATH)

    if validate_against_schema(buses, bus_schema, schema_store, filename='bus_configs.json'):
        print_as_ok("bus_config.json")
        return True
    else:
        print_as_error("Validation failed for bus_configs.json")
        return False

def validate_internal_nodes(schema_store) -> bool:
    node_schema = load_json(SCHEMA_DIR / 'node_schema.json')
    all_valid = True
    
    for node_file in sorted(NODE_CONFIG_DIR.glob('*.json')):
        node_data = load_json(node_file)
        
        if validate_against_schema(node_data, node_schema, schema_store, filename=node_file.name):
            print_as_ok(f"{node_file.name}")
        else:
            all_valid = False
    return all_valid

def validate_external_nodes(schema_store) -> bool:
    external_node_schema = load_json(SCHEMA_DIR / 'external_node_schema.json')
    all_valid = True

    for node_file in sorted(EXTERNAL_NODE_CONFIG_DIR.glob('*.json')):
        node_data = load_json(node_file)
        
        if validate_against_schema(node_data, external_node_schema, schema_store, filename=node_file.name):
            print_as_ok(f"{node_file.name}")
        else:
            all_valid = False
    return all_valid

def validate_all() -> bool:
    print("Validating configs against schema...")

    all_valid = True
    
    # Load shared schemas for references
    message_schema = load_json(SCHEMA_DIR / 'message_schema.json')
    schema_store = {
        'file:///message_schema.json': message_schema,
    }

    # Validate custom types schema
    if not validate_common_types():
        all_valid = False

    # Validate bus configs
    if not validate_bus_config(schema_store):
        all_valid = False
    
    # Validate node configs
    if not validate_internal_nodes(schema_store):
        all_valid = False
    
    if not validate_external_nodes(schema_store):
        all_valid = False
    
    if all_valid:
        print_as_success("All configs passed schema validation")
        return True
    else:
        print_as_warning("One or more configs failed schema validation")
        return False

if __name__ == "__main__":
    try:
        validate_all()
    except ValidationError as e:
        print_as_error(f"Validation error: {e}")
        sys.exit(1)
    except Exception as e:
        print_as_error(f"Unexpected error: {e}")
        sys.exit(1)
