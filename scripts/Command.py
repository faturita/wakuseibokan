import socket
from struct import *
import datetime, time
from TelemetryDictionary import telemetrydirs as td
import sys

import math
import numpy as np

DEFAULT_HEATUP = 75 # #change if value too high or low

class Command:
    def __init__(self, ip='127.0.0.1', controlport=4501):
        self.ctrlsock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.ctrl_server_address = (ip, controlport)

        self.command = 0

        self.__heatup = 100

    def __str__(self):
        return ''

    def __repr__(self):
        return self.__str__()
    
    def fire(self):
        self.command = 11

    @property
    def heatup(self):
        return self.__heatup

    def send_command(self,timer, controllingid, thrust, steering, turretdeclination, turretbearing):
        spawnid=0
        typeofisland=0
        x=0.0
        y=0.0
        z=0.0
        target=0
        bit=0
        weapon=0
        yaw=0
        bank=0
        faction=controllingid

        # Thrust is the speed of the tank. >0 is forward, <0 is backwards.
        # Steering controls the direction of the tank. >0 is right, <0 is left.
        # turretdeclination is the pitch movement, the control of the turret. >0 is up, <0 is down, 90 is straight up.
        # TurretBearing is the rotation of the turret. >0 is right, <0 is left.

        #check if the tank is overheated to prevent firing if heatup is not 0
        # (this is not longer necessary, because the simulator is processing that info)
        #if (self.__heatup > 0 and self.command == 11):
        #    self.command = 0
            
        
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
            self.command,    # 0 or 11
            spawnid,
            typeofisland,
            x,y,z,
            target,
            weapon,
            timer)
        
        # Check the size of the struct due to cross-platform compatibility issues.
        #print(calcsize("<i6fiiiifffiiI"))   This  must be 68 to be cross platform.

        sent = self.ctrlsock.sendto(data, self.ctrl_server_address)

        if (self.__heatup > 0):
            self.__heatup -= 1

        if (self.command == 11):
            self.__heatup = DEFAULT_HEATUP  


        self.command = 0
    
class Recorder:
    def __init__(self):
        self.newepisode()
        
    def newepisode(self):
        ts = time.time()
        st = datetime.datetime.fromtimestamp(ts).strftime('%Y-%m-%d-%H-%M-%S')
        self.f = open('./data/episode.'+st+'.dat', 'w')
        self.f.write('timer,x1,z1,b1,h1,p1,x2,z2,b2,h2,p2\n')
        self.f.flush()

    def record(self, timer, x1,z1,b1,h1,p1,x2,z2,b2,h2,p2):
        self.f.write( str(timer) + ',' + 
                str(x1) + ',' + 
                str(z1) +  ',' + 
                str(b1) + ',' + 
                str(h1) + ',' + 
                str(p1) + ',' +
                str(x2) + ',' +
                str(z2) + ',' +
                str(b2) + ',' +
                str(h2) + ',' +
                str(p2) + '\n')
        self.f.flush();  

    def recordvalues(self, tank1values, tank2values):
        self.record(tank1values[td['timer']],
                    tank1values[td['x']],
                    tank1values[td['z']],
                    tank1values[td['bearing']],
                    tank1values[td['health']],
                    tank1values[td['power']],
                    tank2values[td['x']],
                    tank2values[td['z']],
                    tank2values[td['bearing']],
                    tank2values[td['health']],
                    tank2values[td['power']])

