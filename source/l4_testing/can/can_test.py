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

msg3 = can.Message(arbitration_id=0x1400058a,
                  data=[5, 2, 3, 4],
                  is_extended_id=True)
try:
    bus.send(msg)
    print("Message sent on {}".format(bus.channel_info))
except can.CanError:
    print("Message NOT sent")

# bus.send_periodic(msg, 0.2)
# bus.send_periodic(msg2, 0.1)
bus.send_periodic(msg3, 0.5)

try:
    while(True):
        print(bus.recv(3))

except KeyboardInterrupt:
    bus.shutdown()
    print("done")
