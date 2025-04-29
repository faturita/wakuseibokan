'''
Seek and Destroying Script

This script receives the information from the simulator (the telemetry)
identify where the other tank is located, the bearing and the distance, 
and sends the command to the tank to move its trajectory to redirect towards
the other tank for shooting it when it is close enough.

Real time Data Science

'''
import socket
from struct import *
import datetime, time
from TelemetryDictionary import telemetrydirs as td
import sys

import math
import numpy as np

from Command import Command
from Command import Recorder
import Configuration

FIRE = 11

def getAzimuth(x1, z1, x2, z2): 
    dx = (x2 - x1) 
    dz = (z2 - z1) 

    val = np.arctan2(dz, dx) * 180.0/np.pi

    if (val>=90):
        val -= 90
    else:
        val += 270

    return val

def getAzimuthRadians(x1, z1, x2, z2):
    x = getAzimuth(x1, z1, x2, z2)* (np.pi/180.0) * (-1)

    if (getAzimuth(x1, z1, x2, z2)>180 and getAzimuth(x1, z1, x2, z2)<360):
        x = getAzimuth(x1, z1, x2, z2)-360
        x = x * (np.pi/180.0) * (-1)    

    return x

quadrant  = 0
sp = 0

def getContinuosAzimuthRadians(x1,z1,x2,z2):
    global quadrant
    global sp

    x = getAzimuthRadians(x1,z1,x2,z2)

    currquadrant = 0
    if (x1<0 and z1>0):
        currquadrant = 1
    if (x1<0 and z1<0):
        currquadrant = 2
    if (x1>0 and z1<0):
        currquadrant = 3
    if (x1>0 and z1>0):
        currquadrant = 4

    if (quadrant == 3 and currquadrant == 2):
        sp += 1
        x = x + sp * 2 * np.pi
    if (quadrant == 2 and currquadrant == 3):
        sp -= 1
        x = x + sp * 2 * np.pi

    quadrant = currquadrant

    return x


def aim(values1, values2):
    angle = getAzimuth(values1[td['x']], values1[td['z']], values2[td['x']], values2[td['z']])
    bearing1 = values1[td['bearing']]

    angle2 = angle - bearing1

    return angle2

class Controller:
    def __init__(self, tankparam):
        # UDP Telemetry port on port 4500
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        tankparam = int(tankparam)
        port = 4601
        if (tankparam == 1):
            port = 4601
        elif (tankparam == 2):
            port = 4602
        self.server_address = ('0.0.0.0', port)
        print ('Starting up on %s port %s' % self.server_address)

        self.sock.bind(self.server_address)
        self.sock.settimeout(5)

        self.length = 80
        self.unpackcode = '<Lififfffffffffffffff'

        self.recorder = Recorder()

        self.tank = tankparam

    def read(self):
        data, address = self.sock.recvfrom(self.length)

        #print(f"Fps: {fps.fps}")

        # Take care of the latency
        if len(data)>0 and len(data) == self.length:
            # is  a valid message struct
            new_values = unpack(self.unpackcode,data)
            return new_values
        
        return None
    
    def run(self):
        if (self.tank == 1):
            command = Command(Configuration.ip, 4501)
        elif (self.tank == 2):
            command = Command(Configuration.ip, 4502)
             

        while (True):
            try:
                tank1values = self.read()
                if int(tank1values[td['number']]) != 1:
                    continue

                tank2values = self.read()
                if int(tank2values[td['number']]) != 2:
                    continue  


                if (self.tank == 1):
                    myvalues =  tank1values
                    othervalues = tank2values
                else:
                    myvalues = tank2values
                    othervalues = tank1values

                #print(f"Tank1: {tank1values[td['number']]}, {tank1values[td['bearing']]}, Tank 2: {tank2values[td['number']]}, {tank1values[td['bearing']]}")              
                #print (f"Health: {myvalues[td['health']]}")
                self.recorder.recordvalues(myvalues,othervalues)
                
                
                #m -------


                vec2d = (float(myvalues[td['x']]), float(myvalues[td['z']]))
                polardistance = math.sqrt( vec2d[0] ** 2 + vec2d[1] ** 2)

                vec2dtotarget = (float(othervalues[td['x']])-float(myvalues[td['x']]), float(othervalues[td['z']])-float(myvalues[td['z']]))
                targetdistance = math.sqrt( vec2dtotarget[0] ** 2 + vec2dtotarget[1] ** 2)

                #print(polardistance , targetdistance)

                thrust = 0.0
                if polardistance < 1700:
                    thrust = 10.0
                    steering = 0

                turretbearing = aim(myvalues,othervalues)
                
                turretdeclination = np.random.uniform(-0.4, 0.4)

                if (turretbearing > 0.0):
                    steering = 1.0
                    thrust = 10.0
                elif (turretbearing < 0.0):
                    steering = -1.0
                    thrust = 10.0


                if polardistance >= 1700:
                    thrust = 0.0   
                    steering = 0


                if targetdistance < 200:
                    command.command = FIRE
                    thrust = 0.0
                    
                if targetdistance < 200:
                    thrust = 0.0
                
                    
                # ----

                command.send_command(myvalues[td['timer']],self.tank,thrust,
                                     steering,
                                     turretdeclination,
                                     turretbearing)             
                
            except socket.timeout:
                print("Episode Completed")
                break


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python SeekAndDestroy.py [tank_number]")
        sys.exit(1)
        
    controller = Controller(sys.argv[1])
    controller.run() 
  