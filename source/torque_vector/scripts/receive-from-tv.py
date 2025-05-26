import serial
import struct
import serial.tools.list_ports

# Change this to match your C struct
STRUCT_FORMAT = "<5f5f5ff10ff2ff2ff3ff10f3f2ff3f2f3f2ff2ff2ff2fffff"
HEADER1 = b"\xaa\x55"
HEADER2 = b"\x55\xaa"
BUFFER_SIZE = struct.calcsize(STRUCT_FORMAT)


def list_serial_ports():
    """Lists all available serial ports and allows the user to select one."""
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


def find_header(ser):
    """Find either sync header (`0xAA55` or `0x55AA`)."""
    while True:
        byte1 = ser.read(1)
        if byte1 in (HEADER1[:1], HEADER2[:1]):
            byte2 = ser.read(1)
            if (byte1 + byte2) in (HEADER1, HEADER2):
                return True


def read_serial_struct(port, baudrate=115200):
    """Read and decode binary struct from the serial port."""
    with serial.Serial(port, baudrate, timeout=1) as ser:
        ser.reset_input_buffer()
        print(f"\nListening on {port}... Looking for sync header...\n")

        while True:
            find_header(ser)

            data = ser.read(BUFFER_SIZE)

            if len(data) == BUFFER_SIZE:
                parsed_data = struct.unpack(
                    STRUCT_FORMAT, data
                )
                print(f"Received: {parsed_data}")


def debug_serial(port, baudrate=115200):
    """Debug function to print raw incoming serial bytes."""
    with serial.Serial(port, baudrate, timeout=1) as ser:
        print("\nRaw serial dump (first 50 bytes):")
        data = ser.read(50)
        print(" ".join(f"{b:02X}" for b in data))


if __name__ == "__main__":
    selected_port = list_serial_ports()

    if selected_port:
        print(f"\nUsing port: {selected_port}")

        debug_serial(selected_port)

        read_serial_struct(selected_port)
    else:
        print("No valid port selected. Exiting.")