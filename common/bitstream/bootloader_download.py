from math import floor, ceil
import re
import time
from pathlib import Path
from pprint import pprint
from typing import List, Tuple

import can
import can.interfaces.gs_usb
import cantools
import gs_usb
import usb
from intelhex import IntelHex
from can_process import CANRxThread, CANTxThread
from bootloader_commands import BootloaderCommand

# Setup CAN bus and DBC
db = cantools.database.load_file(Path("../daq/per_dbc.dbc"))
dev = usb.core.find(idVendor=0x1D50, idProduct=0x606F)
bus = None
if dev:
    channel = dev.product
    bus_num = dev.bus
    addr = dev.address
    del(dev)
    bus = can.ThreadSafeBus(bustype="gs_usb", channel=channel, bus=bus_num, address=addr, bitrate=500000)
    while(bus.recv(0)): pass

# Setup Rx and Tx threads
can_rx = CANRxThread(bus, db)
can_tx = CANTxThread(bus)

def sleep_ns(ns):
    exit = time.time_ns() + ns
    while(time.time_ns() < exit):
        pass

def update_firmware(bl: BootloaderCommand, fname) -> None:
    ih = IntelHex()
    ih.fromfile(fname, format="hex")
    segments = ih.segments()

    for seg in segments:
        print(f"Segment: 0x{seg[0]:02X} : 0x{seg[1]:02X}")
    
    assert(len(segments) == 1)
    total_bytes = len(ih)
    total_words = ceil(total_bytes / 4)

    while(not bl.get_rx_msg() or bl.get_rx_msg()[0] != 2):
        pass
    print("Found ")
    can_tx.send(bl.firmware_size_msg(total_words))
    while(bl.get_rx_msg()[0] != 3):
        pass

    print()

    print(f"Sending {fname}...")
    print(f"Total File Size: {total_bytes} Bytes")

    num_msg = 0
    for address in range(segments[0][0], segments[0][1], 4):
        bin_arr = ih.tobinarray(start=address, size=4)
        data = sum([x << ((i*8)) for i, x in enumerate(bin_arr)])
        can_tx.send(bl.firmware_data_msg(data))
        num_msg  = num_msg + 1
        while(bl.get_rx_msg()[1] != address):
            print(f"{bl.get_rx_msg()[0]}|{bl.get_rx_msg()[1]} != {address}")


if __name__ == "__main__":
    can_rx.start()
    can_tx.start()


    bl = BootloaderCommand("torquevector", db, can_rx)

    update_firmware(bl, "torque_vector.hex")
    
    # print(can_rx.get_all_signals())