import usb.core
import usb.util
import time as time

import matplotlib.pyplot as plt

def getSampleRate(maxSamples):
    samples = 0
    timeBegin = timeclass.perf_counter()
    while samples < maxSamples:
        print(epIn.read(64))
        samples += 64
    timeEnd = timeclass.perf_counter()
    print("Bytes per second: ", 1 / ((timeEnd - timeBegin) / maxSamples))

# search for our device by product and vendor ID
dev = usb.core.find(idVendor=0x04b4, idProduct=0x8085)

#dev = usb.core.find(find_all=True)
#for d in dev:
#    print(d)
#exit()


if dev is None:
    raise ValueError('Device not found')

#ep = dev[0].interfaces()[0].endpoints()[0]

#raise error if device is not found


# set the active configuration (basically, start the device)
dev.set_configuration()

# get interface 0, alternate setting 0
intf = dev.get_active_configuration()[(0, 0)]

# find the first (and in this case only) OUT endpoint in our interface
#epOut = usb.util.find_descriptor(
#        intf,
#        custom_match= \
#        lambda e: \
#            usb.util.endpoint_direction(e.bEndpointAddress) == \
#            usb.util.ENDPOINT_OUT)

# find the first (and in this case only) IN endpoint in our interface
epIn = usb.util.find_descriptor(
    intf,
    custom_match= \
    lambda e: \
        usb.util.endpoint_direction(e.bEndpointAddress) == \
        usb.util.ENDPOINT_IN)

# make sure our endpoints were found
#assert epOut is not None
assert epIn is not None

plot_data = []
plot_data_x = []

samples = 0
maxSamples = 100

# Stress test to make sure buffers never flood
#while True:
#    data = epIn.read(64)

data = epIn.read(64) # Always skip first packet
while samples < maxSamples:
    timeBegin = time.perf_counter()
    data = epIn.read(64) # contains 32 ADC samples
    #plot_data = plot_data + (data.tolist()) #todo: this method is to slow, will cause Psoc to overrun
    i = 0
    while i < len(data):
        plot_data.append(int.from_bytes(data[i + 1].to_bytes(1, byteorder="big") + data[i].to_bytes(1, byteorder="big"), byteorder="big"))
        i += 2

    #plot_data.append(d)
    samples += (len(data)/2)
    timeEnd = time.perf_counter()

print(timeEnd-timeBegin)

# Convert 8 bits to 16 bits numbers
#i = 0
#plot_data_real = []
#while i<len(plot_data):
    #plot_data_real.append(int.from_bytes(plot_data[i+1].to_bytes(1, byteorder="big") + plot_data[i].to_bytes(1, byteorder="big"), byteorder="big"))
    #i+=2

fig, ax = plt.subplots()
ax.plot(plot_data, marker='o')

plt.show()


