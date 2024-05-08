from typing import Tuple
import can

# Helper class for sending bootloader commands over CAN
class BootloaderCommand():
    
    APP_IDs = [
        "main_module",
        "dashboard",
        "torquevector",
        "a_box",
        "pdu",
        "l4_testing",
        "f4_testing",
        "f7_testing",
        "daq",
        "firmware"
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

    BL_ERROR = {
        0 : "BLERROR_CRC_FAIL",
        1 : "BLERROR_LOCKED",
        2 : "BLERROR_LOW_ADDR",
        3 : "BLERROR_ADDR_BOUND",
        4 : "BLERROR_FLASH",
    }

    def __init__(self, application_name, can_db) -> None:
        if (application_name not in self.APP_IDs):
            print(f"App name \"{application_name}\" not recoginzed")
            print(f"Possible apps: {self.APP_IDs}")
            return None

        self.TX_MSG = can_db.get_message_by_name(f"{application_name}_bl_cmd")
        self.RX_MSG = can_db.get_message_by_name(f"{application_name}_bl_resp")
        self.DATA_MSG = can_db.get_message_by_name(f"bitstream_data")

    def firmware_size_msg(self, fw_size) -> can.Message:
        data = self.TX_MSG.encode({"cmd": self.TX_CMD["BLCMD_START"], "data": fw_size})
        return can.Message(arbitration_id=self.TX_MSG.frame_id, data=data)

    def firmware_data_msg(self, double_word) -> can.Message:
        data = double_word.to_bytes(8, 'little')
        return can.Message(arbitration_id=self.DATA_MSG.frame_id, data=data)

    def firmware_crc_msg(self, crc) -> can.Message:
        data = self.TX_MSG.encode({"cmd": self.TX_CMD["BLCMD_CRC"], "data": crc})
        return can.Message(arbitration_id=self.TX_MSG.frame_id, data=data)

    def firmware_rst_msg(self) -> can.Message:
        data = self.TX_MSG.encode({"cmd": self.TX_CMD["BLCMD_RST"], "data": 0})
        return can.Message(arbitration_id=self.TX_MSG.frame_id, data=data)
    
    def decode_msg(self, msg: can.Message):
        return self.RX_MSG.decode(msg.data)