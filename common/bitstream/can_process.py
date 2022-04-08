from typing import Any
import cantools
from pathlib import Path
import threading
import queue

class CANRxThread(threading.Thread):
    def __init__(self, bus, database) -> None:
        threading.Thread.__init__(self)
        self.daemon = True
        self.bus = bus
        self.rx_lock = threading.Lock()
        self.rx_messages = {}
        self.db = database

    def get_all_signals(self):
        with self.rx_lock:
            return self.rx_messages

    def __getitem__(self, key):
        return self.get_signal(key)

    def get_signal(self, message_id, sig_name: str) -> Any:
        with self.rx_lock:
            if message_id in self.rx_messages and sig_name in self.rx_messages[message_id]:
                return self.rx_messages[message_id][sig_name]
            else:
                return None

    def run(self):
        while(self.is_alive()):
            msg = self.bus.recv()
            try:
                can_rx = self.db.decode_message(msg.arbitration_id, msg.data)
                with self.rx_lock:
                    for sig_name in can_rx:
                        if not msg.arbitration_id in self.rx_messages:
                            self.rx_messages[msg.arbitration_id] = {}
                        self.rx_messages[msg.arbitration_id][sig_name] = can_rx[sig_name]
            
            except KeyError:
                print(f"Unknown message with ID {msg.arbitration_id} and Data {msg.data}")
                continue

            


class CANTxThread(threading.Thread):
    def __init__(self, bus) -> None:
        threading.Thread.__init__(self)
        self.daemon = True
        self.bus = bus
        self.tx_q = queue.Queue()

    def send(self, msg):
        self.tx_q.put(msg)

    def queue_occ(self):
        if self.tx_q.empty():
            return 0
        return self.tx_q.qsize()

    def run(self):
        while(self.is_alive()):
            item = self.tx_q.get(block=True)
            self.bus.send(item)