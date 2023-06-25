'''

UDP Send Test

'''

import matplotlib.pyplot as plt
import numpy as np

import time
import datetime
from struct import *

import sys, select

import socket

#from TelemetryDictionary import telemetrydirs

# This is the port where the simulator is waiting for commands.
ctrlsock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
ip = '127.0.0.1'
port = 5400

if (len(sys.argv)>=2):
    ip = sys.argv[1]
    port = int(sys.argv[2])

server_address = (ip, port)


sent = ctrlsock.sendto(b'1', server_address)

print('Package sent...')

print ('Everything successfully closed.')

quit()
