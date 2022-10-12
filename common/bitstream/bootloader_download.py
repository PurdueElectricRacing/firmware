from math import floor, ceil
import re
from tabnanny import check
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
import os
dbc_path = Path(__file__).parent.parent.joinpath(Path("daq/per_dbc.dbc"))
# Setup CAN bus and DBC
db = cantools.database.load_file(dbc_path)
dev = usb.core.find(idVendor=0x1D50, idProduct=0x606F)
bus = None
if dev:
    channel = dev.product
    bus_num = dev.bus
    addr = dev.address
    del(dev)
    bus = can.Bus(bustype="gs_usb", channel=channel, bus=bus_num, address=addr, bitrate=500000)
    while(bus.recv(0)): pass

# Setup Rx and Tx threads
# can_rx = CANRxThread(bus, db)
# can_tx = CANTxThread(bus)

def sleep_ns(ns):
    exit = time.time_ns() + ns
    while(time.time_ns() < exit):
        pass


# CRC-32b calculation
CRC_POLY = 0x04C11DB7
def crc_update(data, prev):
    crc = prev ^ data
    idx = 0
    while (idx < 32):
        if (crc & 0x80000000): crc = ((crc << 1) ^ CRC_POLY) & 0xFFFFFFFF
        else: crc = (crc << 1) & 0xFFFFFFFF
        idx += 1
    return crc


def update_firmware(bl: BootloaderCommand, fname) -> None:
    start = time.time()

    ih = IntelHex()
    ih.fromfile(fname, format="hex")
    segments = ih.segments()

    for seg in segments:
        print(f"Segment: 0x{seg[0]:02X} : 0x{seg[1]:02X}")
    
    assert(segments[0][0] == 0x8002000)
    total_bytes = segments[0][1] - segments[0][0]
    total_words = ceil(total_bytes / 4)
    print("Waiting for node to enter bootloader mode")

    # Request reset
    msg = bl.firmware_rst_msg()
    bus.send(msg)
    
    can_rx = None
    while can_rx == None or can_rx['cmd'] != bl.RX_CMD['BLSTAT_WAIT']:
        msg = bus.recv()
        if msg.arbitration_id == rx_msg_def.frame_id:
            can_rx = db.decode_message(msg.arbitration_id, msg.data)

    print("Sending over firmware image metadata...")
    msg = bl.firmware_size_msg(total_words)

    bus.send(msg)

    can_rx = None
    while not can_rx or can_rx['cmd'] != bl.RX_CMD['BLSTAT_METDATA_RX']:
        msg = bus.recv()
        if msg.arbitration_id == rx_msg_def.frame_id:
            can_rx = db.decode_message(msg.arbitration_id, msg.data)
            print(can_rx)
    
    time.sleep(0.1)

    print(f"Sending {fname}...")
    print(f"Total File Size: {total_bytes} Bytes")

    num_msg = 0
    crc = 0xFFFFFFFF
    for address in range(segments[0][0], segments[0][1], 4):
        num_msg  = num_msg + 1
        bin_arr = ih.tobinarray(start=address, size=4)

        data = sum([x << ((i*8)) for i, x in enumerate(bin_arr)])
        crc = crc_update(data, crc)

        can_tx = bl.firmware_data_msg(data)
        bus.send(can_tx)

        can_rx = None
        while not can_rx or can_rx['data'] != address + 4 or can_rx['cmd'] != bl.RX_CMD['BLSTAT_PROGRESS']:
            msg = bus.recv()
            if msg.arbitration_id == rx_msg_def.frame_id:
                can_rx = db.decode_message(msg.arbitration_id, msg.data)   
                if can_rx['cmd'] != bl.RX_CMD['BLSTAT_PROGRESS']:
                    print(f"Received invalid command: {can_rx['cmd']}")
                elif can_rx['data'] != address + 4:
                    print(f"{can_rx['cmd']}|{can_rx['data']} != {address + 4} msg # {num_msg}")
                else:
                    # print(f"{can_rx['cmd']}|Sent address {can_rx['data']} msg # {num_msg}")
                    pass
                if can_rx['data'] > address + 4:
                    print("Node data overrun!")
                    return

    # Send CRC
    print("Sending CRC checksum")
    # print("%08x" % (crc & 0xFFFFFFFF))
    can_tx = bl.firmware_crc_msg(crc & 0xFFFFFFFF)
    bus.send(can_tx)

    # Receive firmware download success / fail
    can_rx = None
    while not can_rx or can_rx['cmd'] == bl.RX_CMD['BLSTAT_UNKNOWN_CMD']:
        msg = bus.recv()
        if msg.arbitration_id == rx_msg_def.frame_id:
            can_rx = db.decode_message(msg.arbitration_id, msg.data)   

    if (can_rx['cmd'] == bl.RX_CMD['BLSTAT_DONE']):
        print("Firmware download successful, CRC matched!")
    else:
        print("ERROR: Firmware download failed!!")
    
    print(f"Total time: {time.time() - start}")


rx_msg_def = None
if __name__ == "__main__":

    if len(sys.argv) != 2:
        print("Please provide a path to a firmware image")
        exit()

    bl = BootloaderCommand("l4_testing", db, None)
    rx_msg_def = db.get_message_by_name("l4_testing_bl_resp")
    # bus.set_filters([{"can_id": 0x0404E2BC, "can_mask": 0xFFFFFFFF},
    #                  {"can_id": 0x0409C4BE, "can_mask": 0xFFFFFFFF}])
    # while(1):
    #     msg = bus.recv()
    #     try:
    #         can_rx = db.decode_message(msg.arbitration_id, msg.data)
    #         print(can_rx)
    #     except Exception:
    #         continue

    

    # can_tx.send(bl.reset_node_msg(""))

    fname = sys.argv[1]
    print(Path(fname))
    update_firmware(bl, fname)
    print("Done updating module")