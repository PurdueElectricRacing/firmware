from typing import Tuple
import can

# Helper class for sending bootloader commands over CAN
class BootloaderCommand():
    
    APP_IDs = [
        "mainmodule",
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
    ]

    TX_CMD = {
        "BLCMD_INFO":  0x1,
        "BLCMD_ADD_ADDRESS": 0x2,
        "BLCMD_FW_DATA": 0x3
    }

    ADDRESS_START = 0x08002000
    
    def __init__(self, application_name, can_db, can_rx) -> None:
        assert(application_name in self.APP_IDs)

        self.TX_MSG = can_db.get_message_by_name(f"{application_name}_bl_cmd")
        self.RX_MSG = can_db.get_message_by_name(f"{application_name}_bl_resp")
        self.RESET_MSG = can_db.get_message_by_name("bootloader_request_reset")
        self.can_rx = can_rx
        self.current_addr = self.ADDRESS_START

        print(self.TX_MSG.frame_id)

    def firmware_size_msg(self, fw_size) -> can.Message:
        data = self.TX_MSG.encode({"cmd": self.TX_CMD["BLCMD_INFO"], "data": fw_size})
        return can.Message(arbitration_id=self.TX_MSG.frame_id, data=data)

    def seek_address_msg(self, address):
        assert(self.current_addr <= address)
        if self.current_addr != address:
            data = self.TX_MSG.encode({"cmd": self.TX_CMD["BLCMD_ADD_ADDRESS"], "data": address - self.current_addr})
            self.current_addr = address
            return can.Message(arbitration_id=self.TX_MSG.frame_id, data=data)
        else:
            return None

    def firmware_data_msg(self, word) -> can.Message:
        data = self.TX_MSG.encode({"cmd": self.TX_CMD["BLCMD_FW_DATA"], "data": word})
        self.current_addr = self.current_addr + 1; 
        return can.Message(arbitration_id=self.TX_MSG.frame_id, data=data)

    def get_rx_msg(self) -> Tuple:
        cmd = self.can_rx.get_signal(self.RX_MSG.frame_id, "cmd")
        data = self.can_rx.get_signal(self.RX_MSG.frame_id, "data")
        if not cmd or not data:
            return None
        else: return (cmd, data)
    
    def reset_node_msg(self, name) -> None:
        return can.Message(arbitration_id=self.RESET_MSG.frame_id, data=bytes([0,0,0,0,0,0,0,0]))
