'''
UDP Telemetry Receiving and Episode Recording

This script receives the information from the simulator (the telemetry)
picks only the data that corresponds to one of the tanks, and sends
a command to the simulator to control the tank.

This code is used to register many episodes from the game and to create a database.

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
from Fps import Fps
import Configuration

class Controller:
    def __init__(self, tankparam):
        # UDP Telemetry port on port 4500
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        tankparam = int(tankparam)
        port = 4600 + tankparam
        self.server_address = ('0.0.0.0', port)
        print ('Starting up on %s port %s' % self.server_address)

        self.sock.bind(self.server_address)
        self.sock.settimeout(10)

        self.length = 84
        self.unpackcode = '<LLififfffffffffffffff'

        self.recorder = Recorder()

        self.tank = tankparam
        
        self.mytimer = 0
        
        self.fps = Fps()
        self.fps.tic()

    def read(self):
        data, address = self.sock.recvfrom(self.length)

        # @NOTE Take care of the latency
        if len(data)>0 and len(data) == self.length:
            # is  a valid message struct
            new_values = unpack(self.unpackcode,data)
            return new_values
        
        return None
    
    def run(self):
        command = Command(Configuration.ip, 4500 + self.tank)

        shouldrun = True
        while (shouldrun):
            #try:
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

                self.fps.steptoc() 
                #print(f"Fps: {self.fps.fps}")
                
                if (int(myvalues[td['timer']])<self.mytimer):
                    self.recorder.newepisode()
                    print("New Episode")
                    self.mytimer = int(myvalues[td['timer']])-1
                    
                #print(f"Tank1: {tank1values[td['number']]}, {tank1values[td['bearing']]}, Tank 2: {tank2values[td['number']]}, {tank1values[td['bearing']]}")              
                #print (f"Time: {myvalues[td['timer']]} Health: {myvalues[td['health']]}")
                self.recorder.recordvalues(myvalues,othervalues)


                vec2d = (float(myvalues[td['x']]), float(myvalues[td['z']]))

                polardistance = math.sqrt( vec2d[0] ** 2 + vec2d[1] ** 2)

                #print(polardistance)

                thrust = 0.0
                turretbearing = 0.0
                turretdecl = 0.0
                steering=0.0

                if polardistance < 17000:
                    thrust = 10.0
                    steering = 0

                # If you want to FIRE the weapon do this:
                #command.fire()
                #   This will set the variable command.command to 11 and will fire the weapon on the simulator.
                #   This will fire only once, because the value command.command is set to zero after firing (to avoid mistakenly firing too much)

                #if (int(myvalues[td['timer']]) == 1000):
                    # Do something that you want at one specific time moment

                command.send_command(myvalues[td['timer']],self.tank,thrust,
                                     steering,
                                     turretdecl,
                                     turretbearing)    
                
                self.mytimer+=1         
                
            #except socket.timeout:
            #    print("Episode Completed")
            #    break

if __name__ == '__main__':
    controller = Controller(sys.argv[1])
    controller.run()    

