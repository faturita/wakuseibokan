'''

UDP Telemetry Receiver and plotter and Logger

'''

import matplotlib.pyplot as plt
import numpy as np

import serial
import time
import datetime
from struct import *
from Fps import Fps

import sys, select

import socket

from TelemetryDictionary import telemetrydirs as td

data1 = 1
data2 = 2
data3 = 3

min = -10
max = 200

length = 84
unpackcode = '<LLififfffffffffffffff'
tankparam = 1

if (len(sys.argv)<2):
    print('Provide the tank number')
    quit()

tankparam = int(sys.argv[1])

if (len(sys.argv)>=3):
    print ("Reading which data to shown")
    try:
        data1 = int(sys.argv[2])
        data2 = int(sys.argv[3])
        data3 = int(sys.argv[4])
    except:
        data1 = td[sys.argv[2]]
        data2 = td[sys.argv[3]]
        data3 = td[sys.argv[4]]

if (len(sys.argv)>=6):
    min = int(sys.argv[5])
    max = int(sys.argv[6])

if (len(sys.argv)>=8):
    length = int(sys.argv[7])
    unpackcode = sys.argv[8]

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
tankparam = int(tankparam)
port = 4600 + tankparam  # Listening port based on tank number
server_address = ('0.0.0.0', port)
print ('Starting up on %s port %s' % server_address)

sock.bind(server_address)

def gimmesomething(ser):
    while True:
        line = ser.readline()
        if (len(line)>0):
            break
    return line


# Sensor Recording
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

shouldrun = True
fps = Fps()
fps.tic()  # Start FPS timer

def read():
    # Receive telemetry data and unpack it if it's the correct length
    dataframe, address = sock.recvfrom(length)
    if len(dataframe) > 0 and len(dataframe) == length:
        return unpack(unpackcode, dataframe)
    return None

while shouldrun:

    try:
        
        # Read both tanks' telemetry packets
        tank1values = read()
        if int(tank1values[td['number']]) != 1:
            continue

        tank2values = read()
        if int(tank2values[td['number']]) != 2:
            continue

        # Determine which tank is "ours"
        if tankparam == 1:
            myvalues = tank1values
            othervalues = tank2values
        else:
            myvalues = tank2values
            othervalues = tank1values
            
        fps.steptoc()
        print(f"Fps: {fps.fps}")
        f.write( str(myvalues[data1]) + ' ' + str(myvalues[data2]) + ' ' + str(myvalues[data3]) + '\n')

        x.append( float(myvalues[data1]))
        y.append( float(myvalues[data2]))
        z.append( float(myvalues[data3]))

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
    except socket.timeout:
        # If timeout, end the episode
        print("Episode Completed")
        shouldrun = False
        break


f.close()
print ('Everything successfully closed.')
