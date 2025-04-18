import serial
import struct
import serial.tools.list_ports
import time
import random
from scipy.io import loadmat
import numpy as np

# EDIT THIS TO MATCH serial_rx in main.h on TV
SEND_STRUCT_FORMAT = "fff2f2ff3ffffffff3f2ffffffffffffffffffff5f5f5f"

# EDIT THIS TO MATCH serial_tx in main.h on TV
RECEIVE_STRUCT_FORMAT = "<5f5f5ff10ffff2f2ff3ffffffff3f2ffffff2fff2f2ff2ffffffff"

NUM_VALUES = 57
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
    not_done = True
    while not_done:
        bytes = ser.read(1)
        #print(f"BYTE: {bytes}")
        # print(f"FIRST BYTE: {byte1}")
        if bytes == b'\xAA':
            bytes2 = ser.read(1)
            if bytes2 == b'\x55':
                not_done = False

def send_serial_struct(port, baudrate=115200):
    with serial.Serial(port, baudrate, timeout=1) as ser:
        print(f"\nConnected to {port}. Ready to send data.")
        
        # load .mat file, convert data to an array
        matfile = loadmat(r"C:\Users\TAK\Documents\GitHub\SimscapeModel\T1_Pysical_Testbench\VCU COM Testing\random_testing_data_matrix.mat")
        data_np = matfile["randDataMat"]
        data_list = data_np.astype(np.half).tolist()
        num_samples = len(data_list)

        for i in range(10):
            print(f"\n=== Sample {i+1} of {num_samples} ===")
            # format and send data
            sample = data_list[i]
            packet = struct.pack(SEND_STRUCT_FORMAT, *sample)
            print("Sending data...")
            ser.write(packet)
            print(f"Data sent (size: {len(sample)})")
            print(sample)
            #time.sleep(1)
            # recieve data
            #print("Waiting for response...")
            response = receive_response(ser, port)
            print(f"Response recieved (size: {len(response)})")
            print(response)

            # write out response
            #input()

def receive_response(ser, port):
    #ser.reset_input_buffer()
    print(f"\nListening on {port}... Looking for sync header...\n")

    while True:
        # expecting 290 bytes
        find_header(ser)
        # find header find 2, still need 288

        data = ser.read(BUFFER_SIZE)
        # data lenght should be 288
        print(f"length of data: {len(data)}")
        print(f"BUFFER SIZE: {BUFFER_SIZE}")
        if len(data) == BUFFER_SIZE:
            parsed_data = struct.unpack(
                RECEIVE_STRUCT_FORMAT, data
            )
            print(f"Received: {parsed_data}")


    # data = ser.read(BUFFER_SIZE)

    # if len(data) == BUFFER_SIZE:
    #     parsed_data = struct.unpack(RECEIVE_STRUCT_FORMAT, data)
    #     return parsed_data
    # else:
    #     print(f"Error: Invalid data received. Length: {len(data)}")


def main():
    selected_port = list_serial_ports()

    if selected_port:
        print(f"\nUsing port: {selected_port}")

        send_serial_struct(selected_port)
    else:
        print("No valid port selected. Exiting.")

if __name__ == "__main__":
    main()
