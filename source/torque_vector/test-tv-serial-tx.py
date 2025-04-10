import serial
import struct
import serial.tools.list_ports

#Ensure this struct matches the one on TV
STRUCT_FORMAT = "c"
BUFFER_SIZE = struct.calcsize(STRUCT_FORMAT)

if __name__ == "__main__":
    selected_port = list_serial_ports()
    if selected_port:
        print(f"\nUsing port: {selected_port}")
        read_serial_struct(selected_port)
    else:
        print("No valid port selected. Exiting.")

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
        else:
            print("Invalid selection.")
            return None
    except ValueError:
        print("Please enter a valid number.")
        return None

def read_serial_struct(port, baudrate=115200):
    with serial.Serial(port, baudrate, timeout=1) as ser:
        ser.reset_input_buffer()
        print(f"\nListening on {port}... Looking for sync header...\n")
        while True:
            data = ser.read(BUFFER_SIZE)
            if len(data) == BUFFER_SIZE:
                parsed_data = struct.unpack(
                    STRUCT_FORMAT, data
                )
                print(f"Received: {parsed_data}")
