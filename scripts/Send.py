'''
Wakuseibokan - Command UDP Client

The command data type is commandorder.h@ControlStructure.  This is what
the scenario is expecting to be used to control the tank.

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
port = 4501

if (len(sys.argv)>=2):
    ip = sys.argv[1]
    port = int(sys.argv[2])

ctrl_server_address = (ip, port)

def send_command(timer):

    controllingid=1
    thrust=10.0
    steering=1.0
    turretdeclination=10.0
    yaw=0.0
    turretbearing=0.0
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
    weapon=0

    # This is the structure fron CommandOrder
    data=pack("<i6fiiiifffiiI",         
        controllingid,
        thrust,
        steering,
        turretdeclination,
        yaw,
        turretbearing,
        bank,
        faction,
        command,    # 0 or 11
        spawnid,
        typeofisland,
        x,y,z,
        target,
        weapon,
        timer)

    sent = ctrlsock.sendto(data, ctrl_server_address)

    return

send_command(100)

print('Package sent...')

print ('Everything successfully closed.')

quit()
