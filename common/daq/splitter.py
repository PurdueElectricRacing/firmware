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

#split_by_node("can_config.json", "split_nodes")

import os
import json
from glob import glob

NODE_CONFIG_DIR = "node_configs"

def process_message(msg: dict) -> dict:
    # Remove legacy fields
    msg.pop("msg_pgn", None)
    msg.pop("msg_hlp", None)

    # Set msg_priority = 3 unless override present
    if "msg_priority" not in msg and "msg_id_override" not in msg:
        msg["msg_priority"] = 3

    # Ensure msg_period exists
    if "msg_period" not in msg:
        msg["msg_period"] = 0

    return msg

def process_node_config(path: str):
    with open(path, "r") as f:
        data = json.load(f)

    changed = False

    for bus in data.get("busses", []):
        for node in bus.get("nodes", []):
            for direction in ("tx", "rx"):
                if direction in node:
                    new_msgs = []
                    for msg in node[direction]:
                        new_msg = process_message(msg)
                        new_msgs.append(new_msg)
                    node[direction] = new_msgs
                    changed = True

    if changed:
        with open(path, "w") as f:
            json.dump(data, f, indent=2)
        print(f"Updated: {path}")
    else:
        print(f"No changes: {path}")

def main():
    for json_file in glob(os.path.join(NODE_CONFIG_DIR, "*.json")):
        process_node_config(json_file)

if __name__ == "__main__":
    main()
