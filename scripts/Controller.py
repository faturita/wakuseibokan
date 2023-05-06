'''

UDP Telemetry Receiver and controller

'''

import matplotlib.pyplot as plt
import numpy as np

import time
import datetime
from struct import *

import sys, select

import socket

from TelemetryDictionary import telemetrydirs

# This is the port where the simulator is waiting for commands.
ctrlsock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
ip = '127.0.0.1'
controlport = 5400


ctrl_server_address = (ip, controlport)


def send_command(id, roll, thrust,pitch,precesion,openfire):
    #id = 1
    #roll = 1.0
    #thrust = 10.0
    #pitch = 0
    #precesion = 0
    #openfire = 1

    data=pack("iffffi",id,roll,thrust,pitch, precesion, openfire)

    sent = ctrlsock.sendto(data, ctrl_server_address)

    return

data1 = 4
data2 = 2
data3 = 3

min = -400
max = 400

# Telemetry length and package form.
length = 76
unpackcode = 'iiiffffffffffffffff'

if (len(sys.argv)>=2):
    print ("Reading which data to shown")
    try:
        data1 = int(sys.argv[1])
        data2 = int(sys.argv[2])
        data3 = int(sys.argv[3])
    except:
        data1 = telemetrydirs[sys.argv[1]]
        data2 = telemetrydirs[sys.argv[2]]
        data3 = telemetrydirs[sys.argv[3]]
        pass

if (len(sys.argv)>=5):
    min = int(sys.argv[4])
    max = int(sys.argv[5])

if (len(sys.argv)>=7):
    length = int(sys.argv[6])
    unpackcode = sys.argv[7]



# UDP Telemetry port on port 4500
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server_address = ('0.0.0.0', 4500)
print ('Starting up on %s port %s' % server_address)

sock.bind(server_address)

def gimmesomething(ser):
    while True:
        line = ser.readline()
        if (len(line)>0):
            break
    return line


# Sensor Recording
ts = time.time()
st = datetime.datetime.fromtimestamp(ts).strftime('%Y-%m-%d-%H-%M-%S')
f = open('./data/sensor.'+st+'.dat', 'w')


# You probably won't need this if you're embedding things in a tkinter plot...
plt.ion()

x = []
y = []
z = []

fig = plt.figure()
ax = fig.add_subplot(111)

line1, = ax.plot(x,'r', label='X') # Returns a tuple of line objects, thus the comma
line2, = ax.plot(y,'g', label='Y')
line3, = ax.plot(z,'b', label='Z')

ax.axis([0, 500, min, max])


plcounter = 0

plotx = []


counter = 0

address = ''

tank=1

while True:
    # read

    data, address = sock.recvfrom(length)


    if len(data)>0 and len(data) == length:
        # is  a valid message struct
        new_values = unpack(unpackcode,data)

        if int(new_values[telemetrydirs['number']]) == tank:

            f.write( str(new_values[0]) + ',' + str(new_values[1]) + ',' + str(new_values[2]) +  ',' + str(new_values[3]) + ',' + str(new_values[4]) + ',' + str(new_values[6]) + '\n')

            x.append( float(new_values[telemetrydirs['bearing']]))
            y.append( float(new_values[telemetrydirs['x']]))
            z.append( float(new_values[telemetrydirs['z']]))

            plotx.append( plcounter )

            line1.set_ydata(x)
            line2.set_ydata(y)
            line3.set_ydata(z)

            line1.set_xdata(plotx)
            line2.set_xdata(plotx)
            line3.set_xdata(plotx)

            fig.canvas.draw()
            plt.pause(0.0000000001)

            plcounter = plcounter+1

            if plcounter > 500:
                plcounter = 0
                plotx[:] = []
                x[:] = []
                y[:] = []
                z[:] = []

            # Analyze the data to determine what to do.
            # Do something
            #send_command(1,1.0,10.0,0,0,1)


f.close()

print ('Everything successfully closed.')
