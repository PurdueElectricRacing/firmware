from math import floor, ceil
import re
import time
from pathlib import Path
from pprint import pprint
from typing import List, Tuple

import can
import sys
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
    
    assert(segments[0][0] == 0x8002000)
    total_bytes = segments[0][1] - segments[0][0]
    total_words = ceil(total_bytes / 4)
    print("Waiting for node to enter bootloader mode")
    while(not bl.get_rx_msg() or bl.get_rx_msg()[0] != 2):
        pass

    time.sleep(0.1)

    print("Sending over firmware image metadata...")
    can_tx.send(bl.firmware_size_msg(total_words))
    while(not bl.get_rx_msg() or bl.get_rx_msg()[0] != 3):
        pass


    print(f"Sending {fname}...")
    print(f"Total File Size: {total_bytes} Bytes")

    num_msg = 0
    for address in range(segments[0][0], segments[0][1], 4):
        num_msg  = num_msg + 1
        bin_arr = ih.tobinarray(start=address, size=4)
        data = sum([x << ((i*8)) for i, x in enumerate(bin_arr)])
        can_tx.send(bl.firmware_data_msg(data))

        timeout = time.time() + 0.2
        rx_msg = bl.get_rx_msg()
        while(not rx_msg or rx_msg[1] != address + 4):
            if rx_msg:
                print(f"{rx_msg[0]}|{rx_msg[1]} != {address + 4} msg # {num_msg}")
            if time.time() > timeout:
                can_tx.send(bl.firmware_data_msg(data))
                timeout = time.time() + 0.2
                print("Timeout!")
            rx_msg = bl.get_rx_msg()


if __name__ == "__main__":

    if len(sys.argv) != 2:
        print("Please provide a path to a firmware image")
        exit()

    can_rx.start()
    can_tx.start()
    bl = BootloaderCommand("mainmodule", db, can_rx)

    can_tx.send(bl.reset_node_msg(""))

    fname = sys.argv[1]
    update_firmware(bl, fname)
    print("Done updating module")