from typing import Tuple
import can

# Helper class for sending bootloader commands over CAN
class BootloaderCommand():
    
    APP_IDs = [
        "main_module",
        "dashboard",
        "torquevector",
        "precharge",
        "drivelinef",
        "driveliner",
        "bmsa",
        "bmsb",
        "bmsc",
        "bmsd",
        "bmse",
        "bmsf",
        "bmsg",
        "bmsh",
        "bmsj",
        "bmsj",
        "l4_testing",
    ]

    TX_CMD = {
        "BLCMD_START": 0x1,   # Request to start firmware download
        "BLCMD_FW_DATA": 0x2, # Firmware data message
        "BLCMD_CRC": 0x3,     # Final CRC-32b check of firmware
        "BLCMD_RST": 0x5      # Request for reset (from app)
    }

    RX_CMD = {
        "BLSTAT_INVALID": 0,     # Invalid operation
        "BLSTAT_BOOT": 1,        # Bootloader boot alert
        "BLSTAT_WAIT": 2,        # Waiting to get BLCMD
        "BLSTAT_METDATA_RX": 3,  # Progress update for bootloader download
        "BLSTAT_PROGRESS": 4,    # Progress update for bootloader download
        "BLSTAT_DONE": 5,        # Completed the application download with CRC pass
        "BLSTAT_JUMP_TO_APP": 6, # About to jump to application
        "BLSTAT_INVAID_APP": 7,  # Did not attempt to boot because the starting address was invalid
        "BLSTAT_UNKNOWN_CMD": 8  # Incorrect CAN command message format
    }

    ADDRESS_START = 0x08002000
    
    def __init__(self, application_name, can_db, can_rx) -> None:
        assert(application_name in self.APP_IDs)

        self.TX_MSG = can_db.get_message_by_name(f"{application_name}_bl_cmd")
        self.RX_MSG = can_db.get_message_by_name(f"{application_name}_bl_resp")
        self.can_rx = can_rx
        self.current_addr = self.ADDRESS_START

    def firmware_size_msg(self, fw_size) -> can.Message:
        data = self.TX_MSG.encode({"cmd": self.TX_CMD["BLCMD_START"], "data": fw_size})
        return can.Message(arbitration_id=self.TX_MSG.frame_id, data=data)

    def firmware_data_msg(self, word) -> can.Message:
        data = self.TX_MSG.encode({"cmd": self.TX_CMD["BLCMD_FW_DATA"], "data": word})
        self.current_addr = self.current_addr + 1; 
        return can.Message(arbitration_id=self.TX_MSG.frame_id, data=data)

    def firmware_crc_msg(self, crc) -> can.Message:
        data = self.TX_MSG.encode({"cmd": self.TX_CMD["BLCMD_CRC"], "data": crc})
        return can.Message(arbitration_id=self.TX_MSG.frame_id, data=data)

    def firmware_rst_msg(self) -> can.Message:
        data = self.TX_MSG.encode({"cmd": self.TX_CMD["BLCMD_RST"], "data": 0})
        return can.Message(arbitration_id=self.TX_MSG.frame_id, data=data)

    def get_rx_msg(self) -> Tuple:
        cmd = self.can_rx.get_signal(self.RX_MSG.frame_id, "cmd")
        data = self.can_rx.get_signal(self.RX_MSG.frame_id, "data")
        if not cmd or not data:
            return None
        else: return (cmd, data)