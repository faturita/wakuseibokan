'''
Wakuseibokan - Telemetry UDP Server

This is a simple UDP server that receives a telemetry package, in the format
specified by TelemetryDictionary.py (which is the same as the one in
networking/telemetry.cpp@ModelRecord (it should be the same).

The teletry information is sent by the server to the IPs specified in
conf/telemetry.endpoints.ini

The Simulator sends per tick one RECORD per tank.  So this script receives
first the record for tank number 1 and then the record for tank number 2.

'''

import matplotlib.pyplot as plt
import numpy as np

import time
import datetime
from struct import *

import sys, select

import socket


ip = '0.0.0.0'
port = 4501

if (len(sys.argv)>=2):
    ip = sys.argv[1]
    port = int(sys.argv[2])

# UDP SERVER Socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server_address = (ip, port)
print ('Starting up on %s port %s' % server_address)

sock.bind(server_address)

# This is the length and format of the struct ModelRecord that is sent by the server.
# Check https://docs.python.org/3/library/struct.html
length = 68
unpackcode = '<i6fiiiifffiiI'

address = ''
while True:
    # read

    data, address = sock.recvfrom(length)

    print (f'Data Received:{data} Length {len(data)} (Struct size {calcsize(unpackcode)}) from {address}')

    new_values = unpack(unpackcode,data)

    print(f"Timer {new_values[16]}")

print ('Everything successfully closed.')
