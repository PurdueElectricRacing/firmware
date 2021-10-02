import can
import usb

dev = usb.core.find(idVendor=0x1D50, idProduct=0x606F)
channel = dev.product
bus_num = dev.bus
addr = dev.address
del(dev)
bus = can.Bus(bustype="gs_usb", channel=channel, bus=bus_num, address=addr, bitrate=500000)

msg = can.Message(arbitration_id=0x1400028b,
                  data=[0xC5, 0x1A, 0, 1],
                  is_extended_id=True)

msg2 = can.Message(arbitration_id=0x1400028a,
                  data=[5, 2, 3, 4],
                  is_extended_id=True)

msg3 = can.Message(arbitration_id=0x1400008c,
                  data=[0b00100101],
                  is_extended_id=True)

# bus.send_periodic(msg, 0.2)
# bus.send_periodic(msg2, 0.1)
bus.send_periodic(msg3, 0.5)

try:
    while(True):
        rx = bus.recv(3)
        if(rx.is_error_frame):
            print("Error frame: " + str(rx.error_state_indicator))
        
        print(rx)

except KeyboardInterrupt:
    bus.shutdown()
    print("done")
