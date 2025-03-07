import serial
import struct
import pandas as pd
import serial.tools.list_ports
import time

# Define struct format (40 floats per row)
STRUCT_FORMAT = "f" * 40  # 26 from Sheet 1 + 14 from Sheet 2

# File path (update if needed)
FILE_PATH = "TVS_5_10_24_N3.xlsx"


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


def load_excel_data(file_path, sheet_index, col_start, col_end):
    """Loads numerical data from a specific sheet and column range, starting from row 5."""
    df = pd.read_excel(
        file_path, sheet_name=sheet_index, header=None
    )  # Read sheet without headers
    df = df.iloc[4:, col_start:col_end]  # Select rows from B5 downwards (zero-indexed)
    df = df.dropna()  # Remove empty rows
    return df.values.tolist()  # Convert to list of lists (rows)


def send_combined_data(port, data1, data2, baudrate=115200, delay=0.015):
    """Merges rows from both sheets and sends as a single frame over UART."""
    with serial.Serial(port, baudrate, timeout=1) as ser:
        print(f"\nConnected to {port}. Sending merged data...")

        # Make sure both sheets have the same number of rows
        row_count = min(len(data1), len(data2))

        for i in range(row_count):
            row_merged = data1[i] + data2[i]  # Combine rows
            row_merged = row_merged[:32] + [0, 0, 0, 0] + row_merged[-4:]
            if len(row_merged) != 40:
                print(f"Skipping invalid row (wrong column count): {row_merged}")
                continue

            packed_data = struct.pack(STRUCT_FORMAT, *row_merged)
            ser.write(packed_data)
            print(f"Sent Frame {i + 1}: {row_merged}")

            time.sleep(delay)  # Small delay to prevent flooding


if __name__ == "__main__":
    selected_port = list_serial_ports()

    if selected_port:
        # Load data from both sheets
        data_sheet_1 = load_excel_data(
            FILE_PATH, sheet_index="Data", col_start=1, col_end=27
        )  # B5 to AA5
        data_sheet_2 = load_excel_data(
            FILE_PATH, sheet_index="Flag", col_start=1, col_end=14
        )  # B5 to N5

        if data_sheet_1 and data_sheet_2:
            print("\nSending combined data from both sheets...")
            send_combined_data(selected_port, data_sheet_1, data_sheet_2)
        else:
            print("Error: One or both sheets are empty or invalid.")

    else:
        print("No valid port selected. Exiting.")
