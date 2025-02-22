import serial
import struct
import serial.tools.list_ports

# ✅ Change this to match your C struct!
STRUCT_FORMAT = "fffffff"  # 6 floats: accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z
HEADER1 = b"\xaa\x55"  # Normal byte order
HEADER2 = b"\x55\xaa"  # Swapped byte order
BUFFER_SIZE = struct.calcsize(STRUCT_FORMAT)  # Total struct size


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
        if byte1 in (HEADER1[:1], HEADER2[:1]):  # First byte matches either header
            byte2 = ser.read(1)
            if (byte1 + byte2) in (HEADER1, HEADER2):  # Full header matched
                return True


def read_serial_struct(port, baudrate=115200):
    """Read and decode binary struct from the serial port."""
    with serial.Serial(port, baudrate, timeout=1) as ser:
        ser.reset_input_buffer()  # Clear any old data
        print(f"\nListening on {port}... Looking for sync header...\n")

        while True:
            find_header(ser)  # Wait for sync word

            data = ser.read(BUFFER_SIZE)  # Read struct data

            if len(data) == BUFFER_SIZE:
                parsed_data = struct.unpack(
                    STRUCT_FORMAT, data
                )  # Convert binary to values
                print(f"Received: {parsed_data}")


def debug_serial(port, baudrate=115200):
    """Debug function to print raw incoming serial bytes."""
    with serial.Serial(port, baudrate, timeout=1) as ser:
        print("\nRaw serial dump (first 50 bytes):")
        data = ser.read(50)  # Read some raw bytes
        print(" ".join(f"{b:02X}" for b in data))  # Print in hex


if __name__ == "__main__":
    selected_port = list_serial_ports()

    if selected_port:
        print(f"\nUsing port: {selected_port}")

        # Debug first to inspect raw bytes
        debug_serial(selected_port)

        # Then start parsing the struct
        read_serial_struct(selected_port)
    else:
        print("No valid port selected. Exiting.")
