import os
import json
from collections import defaultdict
from copy import deepcopy

def split_by_node(input_json_path, output_dir):
    """
    Splits a CAN config JSON into one file per node.
    Each file contains all bus contexts where that node appears.
    """
    with open(input_json_path) as f:
        data = json.load(f)

    os.makedirs(output_dir, exist_ok=True)

    node_index = defaultdict(list)

    for bus in data.get("busses", []):
        for node in bus.get("nodes", []):
            node_name = node["node_name"]

            node_bus = {
                "bus_name": bus["bus_name"],
                "bus_speed": bus.get("bus_speed"),
                "nodes": [deepcopy(node)]
            }

            node_index[node_name].append(node_bus)

    for node_name, bus_list in node_index.items():
        node_config = {
            "$schema": data.get("$schema"),
            "busses": bus_list
        }
        file_path = os.path.join(output_dir, f"{node_name}.json")
        with open(file_path, "w") as out:
            json.dump(node_config, out, indent=2)

# Example usage
split_by_node("can_config.json", "split_nodes")
