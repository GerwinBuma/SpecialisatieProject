import array

import usb.core
import usb.util

# search for our device by product and vendor ID
dev = usb.core.find(idVendor=0x4B4, idProduct=0x8051)

#raise error if device is not found
if dev is None:
    raise ValueError('Device not found')

# set the active configuration (basically, start the device)
dev.set_configuration()

# get interface 0, alternate setting 0
intf = dev.get_active_configuration()[(0, 0)]

# find the first (and in this case only) OUT endpoint in our interface
epOut = usb.util.find_descriptor(
        intf,
        custom_match= \
        lambda e: \
            usb.util.endpoint_direction(e.bEndpointAddress) == \
            usb.util.ENDPOINT_OUT)

# find the first (and in this case only) IN endpoint in our interface
epIn = usb.util.find_descriptor(
    intf,
    custom_match= \
    lambda e: \
        usb.util.endpoint_direction(e.bEndpointAddress) == \
        usb.util.ENDPOINT_IN)

# make sure our endpoints were found
assert epOut is not None
assert epIn is not None

print("Message: ")
t = input() # get the user input
i = len(t)
epOut.write(t) # send it
print("Received: " + str(epIn.read(i))) # receive the echo