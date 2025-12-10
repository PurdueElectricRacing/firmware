import re

def parse_debug_file(filepath):
    msgs = set()
    with open(filepath, 'r') as f:
        for line in f:
            match = re.search(r'0x[0-9A-Fa-f]+ : (\w+)', line)
            if match:
                msgs.add(match.group(1))
    return msgs

def parse_dbc_file(filepath):
    msgs = set()
    with open(filepath, 'r') as f:
        for line in f:
            if line.startswith("BO_ "):
                parts = line.split()
                if len(parts) >= 3:
                    # BO_ ID Name: Length Sender
                    name = parts[2].rstrip(':')
                    msgs.add(name)
    return msgs

def main():
    debug_msgs = parse_debug_file('common/can_library/debug_VCAN.txt')
    dbc_msgs = parse_dbc_file('common/daq/per_dbc_VCAN.dbc')

    print(f"Debug Msgs: {len(debug_msgs)}")
    print(f"DBC Msgs: {len(dbc_msgs)}")

    missing_in_debug = dbc_msgs - debug_msgs
    missing_in_dbc = debug_msgs - dbc_msgs

    if missing_in_debug:
        print("\nMessages in DBC but MISSING in Debug Output:")
        for m in sorted(missing_in_debug):
            print(f" - {m}")
    
    if missing_in_dbc:
        print("\nMessages in Debug Output but MISSING in DBC:")
        for m in sorted(missing_in_dbc):
            print(f" - {m}")

if __name__ == "__main__":
    main()
