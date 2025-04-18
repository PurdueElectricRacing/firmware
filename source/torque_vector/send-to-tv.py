import serial
import struct
import serial.tools.list_ports
import time
import random

STRUCT_FORMAT = "f"
NUM_VALUES = len(STRUCT_FORMAT)


def list_serial_ports():
    """Lists available serial ports and lets the user select one."""
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


def send_serial_struct(port, baudrate=115200):
    """Sends six floats over UART."""
    with serial.Serial(port, baudrate, timeout=1) as ser:
        print(f"\nConnected to {port}. Ready to send data.")

        while True:
            mode = (
                input(
                    "\nEnter 'm' for manual input, 'r' for random values, or 'q' to quit: "
                )
                .strip()
                .lower()
            )

            if mode == "q":
                print("Exiting.")
                break
            elif mode == "m":
                try:
                    values = [
                        int(input(f"Enter num {i+1}: ")) for i in range(NUM_VALUES)
                    ]
                except ValueError:
                    print("Invalid input Enter numbers only.")
                    continue
            elif mode == "r":
                values = [random.uniform(-10, 10) for _ in range(NUM_VALUES)]
                print(f"Sending random values: {values}")
            else:
                print("Invalid choice.")
                continue

            ser.write(struct.pack(STRUCT_FORMAT, *values))
            print("Data sent")

            time.sleep(0.5)  # Small delay to avoid flooding


if __name__ == "__main__":
    selected_port = list_serial_ports()

    if selected_port:
        send_serial_struct(selected_port)
    else:
        print("No valid port selected. Exiting.")