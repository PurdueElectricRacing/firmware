from typing import List, Tuple
import cantools
import can
import struct
from pathlib import Path
from pprint import pprint
import pdb
import re

db = cantools.database.load_file(Path("../daq/per_dbc.dbc"))
bus = can.interface.Bus('test', bustype='virtual')
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
    bus.send(message)

mcs_regex = re.compile(":(?P<length>[0-9A-F]{2})(?P<address>[0-9A-F]{4})(?P<type>[0-9A-F]{2})(?P<data>[0-9A-F]*)(?P<cksum>[0-9A-F]{2})")
def parse_mcs_line(mcs_line: str) -> Tuple[int, int, bytearray]:
    extract = mcs_regex.match(mcs_line)
    record_type = int(extract.group("type"), 16)
    length = int(extract.group("length"), 16)
    data = extract.group("data")
    return (record_type, length, data)


with open("clk_gen.hex") as f:
    for line in f.readlines():
        (record_type, length, data) = parse_mcs_line(line)
        if record_type == 0:
            bytes = [int(data[2*i:2*i + 2], 16) if len(data[2*i:2*i + 2]) else 0 for i in range(16) ]
            send_bitstream_data(bytes[0:8])
            send_bitstream_data(bytes[8:16])
        
