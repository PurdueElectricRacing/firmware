from math import floor, ceil
import re
from tabnanny import check
import time
from pathlib import Path
from pprint import pprint
from typing import List, Tuple
import threading

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
    bus = can.ThreadSafeBus(bustype="gs_usb", channel=channel, bus=bus_num, address=addr, bitrate=500000)
    while(bus.recv(0)): pass

class CANRxThread(threading.Thread):
    def __init__(self, bus, fnc) -> None:
        threading.Thread.__init__(self)
        self.daemon = True
        self.bus = bus
        self.fnc = fnc
        self.alive = False

    def run(self):
        self.alive = True
        while self.isAlive() and self.alive:
            msg = self.bus.recv(0.25)
            if (msg and msg.arbitration_id & 0x3F == 60): self.fnc(msg)

class Bootloader():
    """ Widget that handles bootloader firmware downloads """

    bl_msg_ending = "_bl_resp"

    reset_reasons = ["RESET_REASON_INVALID",
                     "RESET_REASON_BUTTON",
                     "RESET_REASON_DOWNLOAD_FW",
                     "RESET_REASON_APP_WATCHDOG",
                     "RESET_REASON_POR",
                     "RESET_REASON_BAD_FIRMWARE"]
    
    FLASH_STATE = {
        "WAIT_FOR_BL_MODE": 0,
        "WAIT_FOR_META_RESP": 1,
        "STREAMING_DATA": 2,
        "WAIT_FOR_STATUS": 3
    }

    def __init__(self, bus: can.Bus, firmware_path, db):

        self.bus = bus
        self.ih = None # Intel Hex
        self.db = db

        self.segments = None
        self.total_bytes = 0
        self.total_double_words = 0
        self.flash_active = False
        self.crc = 0xFFFFFFFF
        self.curr_addr = 0x00
        self.flash_state = self.FLASH_STATE['WAIT_FOR_BL_MODE']
        self.flash_timeout_timer = threading.Timer(2, self.flashTimeout)
        self.flash_start_time = 0

        self.firmware_path = firmware_path
        self.hex_loc = ""
        self.bl = None
        self.rxThread = CANRxThread(self.bus, self.handleNewBlMsg)
        self.flashReset(0)

    def selectNode(self, node):
        """ Updates the selected bootloader node """
        self.hex_loc = ""
        self.selected_node = node
        self.bl = BootloaderCommand(self.selected_node, self.db)
        if (not self.bl):
            return None
        # Try to find HEX file associated with node
        node_path = self.firmware_path + f"/output/{self.selected_node}/BL_{self.selected_node}.hex"
        self.verifyHex(node_path)
    
    def verifyHex(self, path):
        if (path != "" and os.path.exists(path)):
            self.hex_loc = path

            self.ih = IntelHex()
            self.ih.fromfile(path, format="hex")
            self.segments = self.ih.segments()

            for seg in self.segments:
                self._logInfo(f"Segment: 0x{seg[0]:02X} : 0x{seg[1]:02X}") 
            if (self.segments[0][0] < 0x8002000):
                self.ui.currFileLbl.setText("Please select file")
                self._logInfo(f"Invalid start address, ensure the hex is of type BL_ and starts at 0x8002000 (L series) or 0x8004000 (F series)")
                self.hex_loc = ""
                self.segments = None
                return
            self.total_bytes = self.segments[0][1] - self.segments[0][0]
            self.total_double_words = ceil(self.total_bytes / 8)

        else:
            self._logInfo(f"Unable to find hex at {path}")
    
    def requestFlash(self):
        if (not self.bl): return
        if (not self.segments):
            self._logInfo(f"Cannot flash {self.selected_node}, invalid hex")
            return
        self.flashReset(0)
        msg = self.bl.firmware_rst_msg()
        self.bus.send(msg)
        self.flash_active = True
        self.crc = 0xFFFFFFFF
        self._logInfo(f"Flashing {self.selected_node}")

        self.restartFlashTimeout(3)
        self.flash_start_time = time.time()

        self.rxThread.start()
        self.flashTxUpdate()
    
    def handleNewBlMsg(self, msg: can.Message):
        if (not self.bl): return
        # Is message for selected node?
        if (msg.arbitration_id == self.bl.RX_MSG.frame_id):
            can_rx = self.bl.decode_msg(msg)
            if (can_rx['cmd'] == self.bl.RX_CMD['BLSTAT_BOOT']):
                self._logInfo(f"{list(self.bl.RX_CMD.keys())[can_rx['cmd']]}: {self.reset_reasons[can_rx['data']]}")
            if (self.flash_active):
                self.flashRxUpdate(can_rx['cmd'], can_rx['data'])
    
    def restartFlashTimeout(self, seconds):
        self.flash_timeout_timer = threading.Timer(2, self.flashTimeout)
        self.flash_timeout_timer.start()
    
    def flashRxUpdate(self, cmd, data):
        """ Only called if message is from current node, and flash is active """
        if (cmd == self.bl.RX_CMD['BLSTAT_INVALID']):
            self._logInfo(f"BL Error: {self.bl.BL_ERROR[data]}")
            self.flashReset(0)
            return
        if (self.flash_state == self.FLASH_STATE['WAIT_FOR_BL_MODE']):
            if (cmd == self.bl.RX_CMD['BLSTAT_WAIT']):
                self.flash_timeout_timer.cancel() # Entered bl mode, stop timeout timer
                self._logInfo("Sending firmware image metadata ...")
                msg = self.bl.firmware_size_msg(self.total_double_words * 2) # words
                self.bus.send(msg)
                self.flash_state = self.FLASH_STATE['WAIT_FOR_META_RESP']
                self.restartFlashTimeout(2)
        elif (self.flash_state == self.FLASH_STATE['WAIT_FOR_META_RESP']):
            if (cmd == self.bl.RX_CMD['BLSTAT_METDATA_RX']):
                self.flash_timeout_timer.cancel()
                self._logInfo(f"Sending {self.hex_loc}...")
                self._logInfo(f"Total File Size: {self.total_bytes} Bytes")
                self.restartFlashTimeout(2)
                self.curr_addr = self.segments[0][0]
                self.flash_state = self.FLASH_STATE['STREAMING_DATA']
        elif (self.flash_state == self.FLASH_STATE['STREAMING_DATA']):
            if (cmd == self.bl.RX_CMD['BLSTAT_PROGRESS']):
                self.flash_timeout_timer.cancel()
                self.restartFlashTimeout(2)
        elif (self.flash_state == self.FLASH_STATE['WAIT_FOR_STATUS']):
            if (cmd != self.bl.RX_CMD['BLSTAT_PROGRESS']):
                self.flash_timeout_timer.cancel()
                if (cmd == self.bl.RX_CMD['BLSTAT_DONE']):
                    self._logInfo("Firmware download successful, CRC matched")
                    self.flashReset(100)
                else:
                    self._logInfo("ERROR: Firmware download failed!!")
                    self.flashReset(0)
                self._logInfo("Total time: %.2f seconds" % (time.time() - self.flash_start_time))
    
    def flashTxUpdate(self):
        while (self.flash_active):
            end_time = time.perf_counter_ns() + 1000000
            if (self.flash_state == self.FLASH_STATE['STREAMING_DATA']):
                if (self.curr_addr < self.segments[0][1]):
                    self.sendSegmentDoubleWord(self.curr_addr)
                    self.curr_addr += 8
                else:
                    self._logInfo("Sending CRC checksum")
                    can_tx = self.bl.firmware_crc_msg(self.crc & 0xFFFFFFFF)
                    self.bus.send(can_tx)
                    self.flash_state = self.FLASH_STATE['WAIT_FOR_STATUS']
            while (time.perf_counter_ns() < end_time):
                pass
    
    def sendSegmentDoubleWord(self, addr):
        """ Only call if flash in progress """
        bin_arr = self.ih.tobinarray(start=addr, size=4)
        data1 = sum([x << ((i*8)) for i, x in enumerate(bin_arr)])
        self.crc = self.crc_update(data1, self.crc)

        if (addr + 4 >= self.segments[0][1]):
            data2 = 0
        else:
            bin_arr = self.ih.tobinarray(start=(addr + 4), size=4)
            data2 = sum([x << ((i*8)) for i, x in enumerate(bin_arr)])
        self.crc = self.crc_update(data2, self.crc)

        can_tx = self.bl.firmware_data_msg((data2 << 32) | data1)
        self.bus.send(can_tx)
    
    def flashTimeout(self):
        if (self.flash_active):
            if (self.flash_state <= self.FLASH_STATE['WAIT_FOR_BL_MODE']):
                self._logInfo("ERROR: Timed out waiting for node to enter bootloader state")
                self.flashReset(0)
            else:
                self._logInfo("ERROR: Timed out during flash")
                self.flashReset(0)
        else:
            self.flash_timeout_timer.cancel() # triggered unexpectedly

    def flashReset(self, prog):
        self.flash_active = False
        if (self.rxThread.isAlive()): 
            self.rxThread.alive = False
        self.flash_timeout_timer.cancel()
        self.flash_state = self.FLASH_STATE['WAIT_FOR_BL_MODE']

    def _logInfo(self, msg):
        print(msg)

    # CRC-32b calculation
    CRC_POLY = 0x04C11DB7
    def crc_update(self, data, prev):
        crc = prev ^ data
        idx = 0
        while (idx < 32):
            if (crc & 0x80000000): crc = ((crc << 1) ^ self.CRC_POLY) & 0xFFFFFFFF
            else: crc = (crc << 1) & 0xFFFFFFFF
            idx += 1
        return crc

rx_msg_def = None
if __name__ == "__main__":

    if len(sys.argv) != 3:
        print("Usage: [module name]")
        exit()

    # fname = sys.argv[2]
    # print(Path(fname))
    fname = "."

    bl = Bootloader(bus, fname, db)
    bl.selectNode(sys.argv[1])
    bl.requestFlash()

    print("Done updating module")

    bus.shutdown()
    usb.util.dispose_resources(bus.gs_usb.gs_usb)
            