from math import floor
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

# Setup CAN bus and DBC
db = cantools.database.load_file(Path("../daq/per_dbc.dbc"))
data_message = db.get_message_by_name("bitstream_data")
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

def send_bitstream_request(fsize: int) -> None:
    req_message = db.get_message_by_name("bitstream_request")
    data = req_message.encode({"download_request": 1, "download_size": fsize})
    message = can.Message(arbitration_id=req_message.frame_id, data=data)
    can_tx.send(message)

def send_bitstream_data(bits: List) -> None:
    data_message = db.get_message_by_name("bitstream_data")
    data = data_message.encode({f"d{i}": bits[i] for i in range(8)})
    message = can.Message(arbitration_id=data_message.frame_id, data=data)
    can_tx.send(message)

def sleep_ns(ns):
    exit = time.time_ns() + ns
    while(time.time_ns() < exit):
        pass

def flash_bitstream(fname: str) -> None:
    
    ih = IntelHex()
    ih.fromfile(fname, format="hex")
    segments = ih.segments()
    
    # Generated file must be one continuous section
    assert len(segments) == 1
    assert segments[0][0] == 0

    total_bytes = segments[0][1]
    print(f"Sending {fname}")
    print(f"Total File Size: {total_bytes} Bytes")

    while(can_rx["flash_erase"] != 1):
        time.sleep(0.1)
        send_bitstream_request(total_bytes)

    start_time = time.time()
    print("Sending data...")

    for i in range(0, total_bytes, 8):
        sleep_ns(400000)
        send_bitstream_data([ih[i+j] for j in range(8)])
        progress = i / total_bytes

        # Check that last page was programmed first
        # while(can_rx["page"] != floor((i) / 256)):
        #     print(f"waiting for page == {floor((i) / 256)} currently: {can_rx['page']} {round( progress * 100, 2)}%")
    print("Waiting...")
    while(can_rx["flash_success"] != 1):
        pass

    total_time = time.time() - start_time
    print(f"Done! Trasfered {total_bytes} Bytes in {total_time} seconds ({total_bytes / total_time} Bps)")

if __name__ == "__main__":
    can_rx.start()
    can_tx.start()

    flash_bitstream("clk_gen.hex")
    print(can_rx.get_all_signals())