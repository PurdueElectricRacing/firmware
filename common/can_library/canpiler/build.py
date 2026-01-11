import sys
from validator import validate_all
from parser import parse_all, parse_faults, SystemContext, BusView
from linker import link_all
from mapper import map_hardware
from dbcgen import generate_dbcs
from codegen import generate_headers
from faultgen import generate_fault_data
from utils import load_json, BUS_CONFIG_PATH, GENERATED_DIR

def create_context(nodes, mappings, fault_modules, bus_configs):
    """Aggregates all system data into a single context object for generators."""
    context = SystemContext(
        nodes=nodes, 
        fault_modules=fault_modules, 
        mappings=mappings,
        bus_configs=bus_configs
    )
    
    # Identify all busses across all nodes
    all_bus_names = set()
    for node in nodes:
        all_bus_names.update(node.busses)
    
    for bus_name in sorted(all_bus_names):
        view = BusView(name=bus_name)
        
        for node in nodes:
            if bus_name in node.busses:
                view.nodes.add(node.name)
                for msg in node.busses[bus_name].tx_messages:
                    view.messages.append(msg)
                    view.sender_map[msg.name] = node.name
        
        # Deterministic sorting for downstream generators
        view.messages.sort(key=lambda x: (x.final_id or 0, x.name))
        context.busses[bus_name] = view
        
    return context

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
        fault_modules = parse_faults()
        link_all(nodes)
        
        # Load bus configs for mapper
        bus_configs = load_json(BUS_CONFIG_PATH)
        busses = {b['name']: b for b in bus_configs['busses']}
        
        mappings = map_hardware(nodes, busses)
        
        # Create the unified context
        context = create_context(nodes, mappings, fault_modules, busses)
        
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

if __name__ == "__main__":
    build()
