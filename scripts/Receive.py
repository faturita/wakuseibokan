'''

UDP Server

'''

import matplotlib.pyplot as plt
import numpy as np

import time
import datetime
from struct import *

import sys, select

import socket


ip = '0.0.0.0'
port = 5400

if (len(sys.argv)>=2):
    ip = sys.argv[1]
    port = sys.argv[2]

# UDP SERVER Socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server_address = (ip, port)
print ('Starting up on %s port %s' % server_address)

sock.bind(server_address)


address = ''
while True:
    # read
    length = 256
    data, address = sock.recvfrom(length)

    print (f'Data Received:{data}')

print ('Everything successfully closed.')
