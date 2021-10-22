import can
import usb
import cantools

dev = usb.core.find(idVendor=0x1D50, idProduct=0x606F)
channel = dev.product
bus_num = dev.bus
addr = dev.address
del(dev)
bus = can.Bus(bustype="gs_usb", channel=channel, bus=bus_num, address=addr, bitrate=500000)

db = cantools.db.load_file('./common/daq/per_dbc.dbc')

test_var_response_msg = db.get_message_by_name('daq_response_TEST_NODE')
test_var_command_msg = db.get_message_by_name('daq_command_TEST_NODE')

for i in range(4):
    bus.recv(0.03)

message = can.Message(arbitration_id=test_var_command_msg.frame_id, 
                      data=[0x04], is_extended_id=True)

bus.send(message)

while True:
    rx = bus.recv(5)
    if(rx.arbitration_id == test_var_response_msg.frame_id):
        print(f"before: {rx}")
        break

message = can.Message(arbitration_id=test_var_command_msg.frame_id, 
                      data=[0x05, 0xEF, 0x0D], is_extended_id=True)

bus.send(message)

message = can.Message(arbitration_id=test_var_command_msg.frame_id, 
                      data=[0x04], is_extended_id=True)

bus.send(message)

while True:
    rx = bus.recv(5)
    if(rx.arbitration_id == test_var_response_msg.frame_id):
        print(f"after: {rx}")
        break

quit()
message = can.Message(arbitration_id=test_var_command_msg.frame_id, 
                      data=[0x04], is_extended_id=True)
# bus.send_periodic(msg, 0.2)
# bus.send_periodic(msg2, 0.1)
bus.send_periodic(message, 0.005)

try:
    while(True):
        rx = bus.recv(3)
        #if(rx.is_error_frame):
        #    print("Error frame: " + str(rx.error_state_indicator))
        if(rx.arbitration_id == test_var_response_msg.frame_id):
            print(rx)

except KeyboardInterrupt:
    bus.shutdown()
    print("done")
