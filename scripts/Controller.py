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

from TelemetryDictionary import telemetrydirs as td

# This is the port where the simulator is waiting for commands
# The structure is given in ../commandorder.h/CommandOrder
ctrlsock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
ip = '127.0.0.1'
controlport = 4501


ctrl_server_address = (ip, controlport)


def send_command(timer):

    controllingid=1
    thrust=10.0
    roll=1.0
    pitch=0.0
    yaw=0.0
    precesion=0.0
    bank=0.0
    faction=1
    #timer=1

    command = 11
    spawnid=0
    typeofisland=0
    x=0.0
    y=0.0
    z=0.0
    target=0
    bit=0
    weapon=0

    # This is the structure fron CommandOrder
    data=pack("iffffffiLiiifffi?i",
        controllingid,
        thrust,
        roll,
        pitch,
        yaw,
        precesion,
        bank,
        faction,
        timer,
        command,
        spawnid,
        typeofisland,
        x,y,z,
        target,
        bit,
        weapon)

    sent = ctrlsock.sendto(data, ctrl_server_address)

    return

data1 = 4
data2 = 2
data3 = 3

min = -400
max = 400

# Telemetry length and package form.
length = 84
unpackcode = 'Liiiffffffffffffffff'

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

    # If you plot, latency will be bigger !!!
    if len(data)>0 and len(data) == length:
        # is  a valid message struct
        new_values = unpack(unpackcode,data)

        if int(new_values[td['number']]) == tank:

            f.write( str(new_values[0]) + ',' + str(new_values[1]) + ',' + str(new_values[2]) +  ',' + str(new_values[3]) + ',' + str(new_values[4]) + ',' + str(new_values[6]) + '\n')

            x.append( float(new_values[td['bearing']]))
            y.append( float(new_values[td['x']]))
            z.append( float(new_values[td['z']]))

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
            send_command(new_values[td["timer"]])


f.close()

print ('Everything successfully closed.')
