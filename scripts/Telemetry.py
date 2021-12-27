'''

UDP Telemetry Receiver and plotter and Logger

'''

import matplotlib.pyplot as plt
import numpy as np

import serial
import time
import datetime
from struct import *

import sys, select

import socket

from TelemetryDictionary import telemetrydirs

data1 = 1
data2 = 2
data3 = 3

min = -10
max = 200

length = 60
unpackcode = 'fffffffffffffff'

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

if (len(sys.argv)>=5):
    min = int(sys.argv[4])
    max = int(sys.argv[5])

if (len(sys.argv)>=7):
    length = int(sys.argv[6])
    unpackcode = sys.argv[7]


serialconnected = False

if (not serialconnected):
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


# Sensor Recording
ts = time.time()
st = datetime.datetime.fromtimestamp(ts).strftime('%Y-%m-%d-%H-%M-%S')
f = open('./data/sensor.'+st+'.dat', 'w')


if (serialconnected):

    ser = serial.Serial(port='/dev/cu.usbmodem1421', baudrate=9600, timeout=0)

    f = open('sensor.dat', 'w')

    ser.write('X')
    time.sleep(6)

    buf = ser.readline()
    print (str(buf))

    buf = ser.readline()
    print (str(buf))

    buf = ser.readline()
    print (str(buf))

    ser.write('S')

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



if (serialconnected):
   ser.write('A7180')
address = ''
while True:
  # read
  if (serialconnected):
      ser.write('S')
      ser.write('P')
      myByte = ser.read(1)
  else:
      myByte = 'S'

  if myByte == 'S':
      if (serialconnected):
         data = ser.read(length) # 24
         myByte = ser.read(1)
      else:
         data, address = sock.recvfrom(length) # 44+26
         myByte = 'E'

      if myByte == 'E' and len(data)>0 and len(data) == length:
          # is  a valid message struct
          new_values = unpack(unpackcode,data)
          f.write( str(new_values[data1]) + ' ' + str(new_values[data2]) + ' ' + str(new_values[data3]) + '\n')

          x.append( float(new_values[data1]))
          y.append( float(new_values[data2]))
          z.append( float(new_values[data3]))

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


f.close()
if (serialconnected):
   ser.close()
print ('Everything successfully closed.')
