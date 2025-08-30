import threading
import time
import os
import psutil

import socket
from struct import *
import datetime, time
import sys
import math
import numpy as np

from TelemetryDictionary import telemetrydirs as td
from Command import Command
from Command import Recorder
import Configuration
from Fps import Fps


data = dict()
data['me'] = dict()
data['other'] = dict()
data['me']['number'] = 1
data['other']['number'] = 2
data['me']['timer'] = 0
data['other']['timer'] = 0

data['me']['thrust'] = 0.0
data['me']['steering'] = 0.0
data['me']['turretdeclination'] = 0.0
data['me']['turretbearing'] = 0.0
data['me']['polardistance'] = 0.0


shouldrun = True

class Controller:
    def __init__(self, tankparam):
        # Create a UDP socket for receiving telemetry
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        tankparam = int(tankparam)
        port = 4600 + tankparam  # Listening port based on tank number
        self.server_address = ('0.0.0.0', port)
        print ('Starting up on %s port %s' % self.server_address)

        # Bind the socket to the chosen address and set a timeout
        self.sock.bind(self.server_address)
        self.sock.settimeout(10)

        # Telemetry packet length and unpacking format
        self.length = 80
        self.unpackcode = '<Lififfffffffffffffff'

        self.recorder = Recorder()  # Initialize recorder to store episodes

        self.tank = tankparam
        self.mytimer = 0
        self.fps = Fps()
        self.fps.tic()  # Start FPS timer

    def read(self):
        # Receive telemetry data and unpack it if it's the correct length
        dataframe, address = self.sock.recvfrom(self.length)
        if len(dataframe) > 0 and len(dataframe) == self.length:
            return unpack(self.unpackcode, dataframe)
        return None

    def run(self, core_id, name):
        # Initialize command sender to simulator with specific control port
        command = Command(Configuration.ip, 4500 + self.tank)  # Replace with the your server IP
        ts = time.time()
        st = datetime.datetime.fromtimestamp(ts).strftime('%Y-%m-%d-%H-%M-%S')
        f = open(f'./data/sensor.{st}.dat', 'w')  # Log file for sensor data
        
        global shouldrun
        global data

        while shouldrun:
            try:
                # Read both tanks' telemetry packets
                tank1values = self.read()
                if int(tank1values[td['number']]) != 1:
                    continue

                tank2values = self.read()
                if int(tank2values[td['number']]) != 2:
                    continue

                # Determine which tank is "ours"
                if self.tank == 1:
                    myvalues = tank1values
                    othervalues = tank2values
                else:
                    myvalues = tank2values
                    othervalues = tank1values

                # Update FPS and print to console
                self.fps.steptoc()
                #print(f"Fps: {self.fps.fps}")

                # Detect if a new episode has started
                if int(myvalues[td['timer']]) < self.mytimer:
                    self.recorder.newepisode()
                    print("New Episode")
                    self.mytimer = int(myvalues[td['timer']]) - 1

                # Record values for both tanks
                self.recorder.recordvalues(myvalues, othervalues)

                # Write select telemetry values to file
                f.write(','.join([str(myvalues[0]), str(myvalues[1]), str(myvalues[2]),
                                  str(myvalues[3]), str(myvalues[4]), str(myvalues[6])]) + '\n')
                f.flush()

                # Calculate distance from origin and print it
                vec2d = (float(myvalues[td['x']]), float(myvalues[td['z']]))
                polardistance = math.sqrt(vec2d[0] ** 2 + vec2d[1] ** 2)
                print(f"Time: {myvalues[td['timer']]} Polar Distance: {polardistance}")
                
                data['me']['polardistance'] = polardistance
                
                # Yield control to other processes.
                time.sleep(0.0)
                
                # Decide with higher priority what this loop decides to do...
                thrust = 0.0 if data['me']['polardistance'] > 1700 else data['me']['thrust']
                steering = data['me']['steering']
                turretdeclination = data['me']['turretdeclination']
                turretbearing = data['me']['turretbearing'] 
                
                # Send command to simulator
                command.send_command(myvalues[td['timer']], self.tank, thrust,
                                     steering, turretdeclination, turretbearing)

                self.mytimer += 1
                
                data['me']['timer'] = self.mytimer

            except socket.timeout:
                # If timeout, end the episode
                print("Episode Completed")
                shouldrun = False
                break

        f.close()
        print('Everything successfully closed.')

class Cortical:
    def __init__(self, tankparam):
        tankparam = int(tankparam)

        self.tank = tankparam
        self.mytimer = 0
        self.fps = Fps()
        self.fps.tic()  # Start FPS timer

    def run(self, core_id, name):

        global shouldrun
        global data
        while shouldrun:
            try:
                # Update FPS and print to console
                self.fps.steptoc()
                #print(f"Fps: {self.fps.fps}")

                self.mytimer += 1
                
                print(f"Processing {data['me']['timer']}")
                
                time.sleep(0.0)
                
                data['me']['thrust'] = 10.0 

            except socket.timeout:
                # If timeout, end the episode
                print("Episode Completed")
                break


# Entry point for the script
# To run: python Subsumption.py [tank_number]
if __name__ == '__main__':
    
    if len(sys.argv) < 2:
        print("Usage: python Subsumption.py [tank_number]")
        sys.exit(1)
    
    controller = Controller(sys.argv[1])
    cortical = Cortical(sys.argv[1])
    

    # Create two threads and assign them to different cores
    thread1 = threading.Thread(target=controller.run, args=(0, "Controller"))
    thread2 = threading.Thread(target=cortical.run, args=(1, "Cortical"))

    thread1.start()
    thread2.start()


    thread1.join()  
    thread2.join()



