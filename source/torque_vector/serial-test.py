import serial
import struct

# Sync header (detect both byte orders)
HEADER1 = b"\xaa\x55"  # Normal order
HEADER2 = b"\x55\xaa"  # Swapped order

# Data format: Two vectors (accel + gyro), each with (x, y, z) floats
VECTOR_3D_FORMAT = "fff"  # Three floats per vector
BUFFER_FORMAT = VECTOR_3D_FORMAT * 2  # Two vectors (accel + gyro)
BUFFER_SIZE = struct.calcsize(BUFFER_FORMAT)  # Total expected size


def find_header(ser):
    """Find the start of a transmission by detecting the sync word."""
    while True:
        byte1 = ser.read(1)  # Read one byte
        if byte1 == b"\xaa" or byte1 == b"\x55":  # Possible start of header
            byte2 = ser.read(1)
            if (byte1 + byte2) in (HEADER1, HEADER2):
                return True  # Found sync header!


def read_vectors(port, baudrate=115200):
    """Read and decode vector data from the serial port."""
    with serial.Serial(port, baudrate, timeout=1) as ser:
        ser.reset_input_buffer()  # Clear any old data
        print(f"Listening on {port}... Looking for sync header...")

        while True:
            find_header(ser)  # Wait for sync word

            data = ser.read(BUFFER_SIZE)  # Read the expected struct size

            if len(data) == BUFFER_SIZE:
                # Unpack binary data into six floats (accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z)
                accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z = struct.unpack(
                    BUFFER_FORMAT, data
                )

                print(
                    f"Accel: ({accel_x:.3f}, {accel_y:.3f}, {accel_z:.3f}) | "
                    f"Gyro: ({gyro_x:.3f}, {gyro_y:.3f}, {gyro_z:.3f})"
                )


def debug_serial(port, baudrate=115200):
    """Debug function to print raw incoming serial bytes."""
    with serial.Serial(port, baudrate, timeout=1) as ser:
        print("Raw serial dump (first 50 bytes):")
        data = ser.read(50)  # Read some raw bytes
        print(" ".join(f"{b:02X}" for b in data))  # Print in hex


if __name__ == "__main__":
    port = "/dev/cu.usbserial-D30J7IC4"  # Your specific serial port

    # Debug first to see raw bytes
    debug_serial(port)

    # Then start reading properly
    read_vectors(port)
