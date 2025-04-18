import serial
import struct
import serial.tools.list_ports
import time
import random

# EDIT THIS TO MATCH serial_rx in main.h on TV
SEND_STRUCT_FORMAT = SEND_STRUCT_FORMAT = "f" #"fff2f2ff3ffffffff3f2ffffffffffffffffffff5f5f5f"

# EDIT THIS TO MATCH serial_tx in main.h on TV
RECEIVE_STRUCT_FORMAT = "<5f5f5ff10ffff2f2ff3ffffffff3f2ffffff2fff2f2ff2ffffffff"

NUM_VALUES = 1 #len(SEND_STRUCT_FORMAT)
HEADER1 = b"\xaa\x55"
HEADER2 = b"\x55\xaa"
BUFFER_SIZE = struct.calcsize(RECEIVE_STRUCT_FORMAT)

def list_serial_ports():
    ports = list(serial.tools.list_ports.comports())
    if not ports:
        print("No serial ports found.")
        return None

    print("\nAvailable Serial Ports:")
    for i, port in enumerate(ports):
        print(f"{i + 1}: {port.device} - {port.description}")

    choice = input("\nSelect a port (enter number): ")
    try:
        index = int(choice) - 1
        if 0 <= index < len(ports):
            return ports[index].device
    except ValueError:
        pass
    print("Invalid selection.")
    return None

def find_header(ser):
    while True:
        byte1 = ser.read(1)
        if byte1 in (HEADER1[:1], HEADER2[:1]):
            byte2 = ser.read(1)
            if (byte1 + byte2) in (HEADER1, HEADER2):
                return True

def send_serial_struct(port, baudrate=115200):
    with serial.Serial(port, baudrate, timeout=1) as ser:
        print(f"\nConnected to {port}. Ready to send data.")

        while True:
            mode = input("\nEnter 'm' for manual input, 'r' for random values, or 'q' to quit: ").strip().lower()

            if mode == "q":
                print("Exiting.")
                break
            elif mode == "m":
                try:
                    values = [float(input(f"Enter float {i+1}: ")) for i in range(NUM_VALUES)]
                except ValueError:
                    print("Invalid input. Enter numbers only.")
                    continue
            elif mode == "r":
                values = [random.uniform(-10, 10) for _ in range(NUM_VALUES)]
                print(f"Sending random values: {values}")
            else:
                print("Invalid choice.")
                continue

            ser.write(struct.pack(SEND_STRUCT_FORMAT, *values))
            print("Data sent")

            print("\nWaiting for response...")
            receive_response(ser)

            repeat = input("\nDo you want to send more data? (y/n): ").strip().lower()
            if repeat != "y":
                print("Exiting.")
                break

def receive_response(ser):
    data = ser.read(BUFFER_SIZE)

    if len(data) == BUFFER_SIZE:
        parsed_data = struct.unpack(RECEIVE_STRUCT_FORMAT, data)
        print(f"Received: {parsed_data}")
    else:
        print(f"Error: Invalid data received. Length: {len(data)}")

def main():
    selected_port = list_serial_ports()

    if selected_port:
        print(f"\nUsing port: {selected_port}")

        send_serial_struct(selected_port)
    else:
        print("No valid port selected. Exiting.")

if __name__ == "__main__":
    main()
