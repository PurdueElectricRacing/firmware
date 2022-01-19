from typing import List, Tuple
import can
import can.interfaces.gs_usb
import gs_usb
import usb
import time
import cantools
from pathlib import Path
from pprint import pprint
import re

db = cantools.database.load_file(Path("../daq/per_dbc.dbc"))

dev = usb.core.find(idVendor=0x1D50, idProduct=0x606F)
bus = None
if dev:
    channel = dev.product
    bus_num = dev.bus
    addr = dev.address
    del(dev)
    bus = can.Bus(bustype="gs_usb", channel=channel, bus=bus_num, address=addr, bitrate=500000)
    while(bus.recv(0)): pass

data_message = db.get_message_by_name("bitstream_data")

def send_bitstream_request(fsize: int) -> None:
    req_message = db.get_message_by_name("bitstream_request")
    data = req_message.encode({"download_request": 1, "download_size": fsize})
    message = can.Message(arbitration_id=req_message.frame_id, data=data)
    bus.send(message)

def send_bitstream_data(bits: List) -> None:
    data_message = db.get_message_by_name("bitstream_data")
    data = data_message.encode({f"d{i}": bits[i] for i in range(8)})
    message = can.Message(arbitration_id=data_message.frame_id, data=data)
    bus.send(message, timeout=0.01)

mcs_regex = re.compile(":(?P<length>[0-9A-F]{2})(?P<address>[0-9A-F]{4})(?P<type>[0-9A-F]{2})(?P<data>[0-9A-F]*)(?P<cksum>[0-9A-F]{2})")
def parse_mcs_line(mcs_line: str) -> Tuple[int, int, bytearray]:
    extract = mcs_regex.match(mcs_line)
    record_type = int(extract.group("type"), 16)
    length = int(extract.group("length"), 16)
    data = extract.group("data")
    return (record_type, length, data)

def flash_bitstream(fname: str) -> None:
    file_length = 0
    with open(fname) as f:
        for line in f.readlines():
            (record_type, length, data) = parse_mcs_line(line)
            if record_type == 0:
                bytes = [int(data[2*i:2*i + 2], 16) if len(data[2*i:2*i + 2]) else None for i in range(16)]
                file_length = file_length + length
    
    print(f"File Length bytes: {file_length}")
    send_bitstream_request(int(file_length / 2))
    while(True):
        msg = bus.recv()
        try:
            can_rx = db.decode_message(msg.arbitration_id, msg.data)
            if "flash_erase" in can_rx and can_rx["flash_erase"] == 1:
                break
        except KeyError:
            pprint(f"Unknown message: {msg}")
    print("Sending data")

    sent = 0
    with open(fname) as f:
        for line in f.readlines():
            (record_type, length, data) = parse_mcs_line(line)
            if record_type == 0:
                bytes = [int(data[2*i:2*i + 2], 16) if len(data[2*i:2*i + 2]) else 0 for i in range(16)]
                sent = sent + 16
                send_bitstream_data(bytes[0:8])
                time.sleep(0.0015)
                send_bitstream_data(bytes[8:16])
                time.sleep(0.0015)
                pprint(f"{round((sent/file_length) * 100, 2)}% complete...")
        
        
if __name__ == "__main__":

    flash_bitstream("clk_gen.hex")
    print("Done!")
    while True:
        msg = bus.recv()
        try:
            can_rx = db.decode_message(msg.arbitration_id, msg.data)
            pprint(can_rx)
        except KeyError:
            pprint(f"Unknown message: {msg}")
