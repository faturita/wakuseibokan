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

# 96
length = 84+3*4 # 84 bytes for the telemetry struct + 3 floats for the landing position
unpackcode = '<LLififffffffffffffffffff'
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
port = 4601 # Listening port based on tank number
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
filename = './data/sensor.'+st+'.dat'
f = open(filename, 'w')

print('Logging to file: '+filename)

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

maxRange = 1000

ax.axis([0, maxRange, min, max])


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
        
        # Read telemetry packets
        values = read()
            
        fps.steptoc()

        f.write( str(values[data1]) + ' ' + str(values[data2]) + ' ' + str(values[data3]) + '\n')
        print ('%f %f %f' % (values[data1], values[data2], values[data3]) )
        x.append( float(values[data1]))
        y.append( float(values[data2]))
        z.append( float(values[data3]))

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

        if plcounter > maxRange:
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
