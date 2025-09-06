'''
This script defines a Controller class that receives telemetry from a tank simulation,
logs sensor data, and sends basic commands to control a tank's behavior.
'''

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
        self.length = 84
        self.unpackcode = '<LLififfffffffffffffff'

        self.recorder = Recorder()  # Initialize recorder to store episodes

        self.tank = tankparam
        self.mytimer = 0
        self.fps = Fps()
        self.fps.tic()  # Start FPS timer

    def read(self):
        # Receive telemetry data and unpack it if it's the correct length
        data, address = self.sock.recvfrom(self.length)
        if len(data) > 0 and len(data) == self.length:
            return unpack(self.unpackcode, data)
        return None

    def run(self):
        # Initialize command sender to simulator with specific control port
        command = Command(Configuration.ip, 4500 + self.tank)  # Replace with the your server IP
        ts = time.time()
        st = datetime.datetime.fromtimestamp(ts).strftime('%Y-%m-%d-%H-%M-%S')
        f = open(f'./data/sensor.{st}.dat', 'w')  # Log file for sensor data

        shouldrun = True
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

                # Basic control logic: move forward if close enough
                thrust = 10.0 if polardistance < 1700 else 0.0
                steering = 0.0
                turretdecl = 0.0
                turretbearing = 0.0
                
                command.fire()

                # Send command to simulator
                command.send_command(myvalues[td['timer']], self.tank, thrust,
                                     steering, turretdecl, turretbearing)

                self.mytimer += 1

            except socket.timeout:
                # If timeout, end the episode
                print("Episode Completed")
                break

        f.close()
        print('Everything successfully closed.')

# Entry point for the script
# To run: python Controller.py [tank_number]
# Example: python Controller.py 1
# TIP: You can open two terminals and run python Controller.py 1 in one, and python Controller.py 2 in the other.
if __name__ == '__main__':
    
    if len(sys.argv) < 2:
        print("Usage: python Controller.py [tank_number]")
        sys.exit(1)
    
    controller = Controller(sys.argv[1])
    controller.run()
