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

import time
import matplotlib.pyplot as plt


from TelemetryDictionary import telemetrydirs as td
from Command import Command
from Command import Recorder
import Configuration
from Fps import Fps
from Controller import Controller


from Plotter import Plotter

class ControlPID(Controller):
    def __init__(self, tankparam):
        super().__init__(tankparam) 
        self.time_steps = []
        self.pv_values = []
        self.control_values = []
        self.setpoint_values = []
    
    def pid_controller(self,setpoint, pv, kp, ki, kd, previous_error, integral, dt):
        error = setpoint - pv
        integral += error * dt
        derivative = (error - previous_error) / dt
        control = kp * error + ki * integral + kd * derivative
        return control, error, integral

    def run(self):
        # Initialize command sender to simulator with specific control port
        command = Command(Configuration.ip, 4500 + self.tank)  # Replace with the your server IP
        ts = time.time()
        st = datetime.datetime.fromtimestamp(ts).strftime('%Y-%m-%d-%H-%M-%S')
        f = open(f'./data/sensor.{st}.dat', 'w')  # Log file for sensor data
        
        plotter = Plotter(500, 0, 360)

        previous_error = 0      # Init the previous error for PID
        integral = 0            # Init the integral term for PID
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
                
                control = 0.0  # Default control value
                
                if (self.mytimer > 400):
                
                    setpoint = 212                  # Desired azimuth angle
                    pv = myvalues[td['azimuth']]    # Process variable     
                    kp = 1.0                        # Proportional gain
                    ki = 0.001                      # Integral gain
                    kd = 0.05                       # Derivative gain
                    dt = 1                          # Time step

                    control, error, integral = self.pid_controller(setpoint, pv, kp, ki, kd, previous_error, integral, dt)
                    previous_error = error

                    self.time_steps.append(self.mytimer * dt)
                    self.pv_values.append(pv)
                    self.control_values.append(control)
                    self.setpoint_values.append(setpoint)
                    
                    plotter.plotdata([pv, setpoint, 0])


                # Calculate distance from origin and print it
                vec2d = (float(myvalues[td['x']]), float(myvalues[td['z']]))
                polardistance = math.sqrt(vec2d[0] ** 2 + vec2d[1] ** 2)
                print(f"Time: {myvalues[td['timer']]} Polar Distance: {polardistance}")

                # Basic control logic: move forward if close enough
                thrust = 10.0 if polardistance < 1700 else 0.0
                steering = control  # Use PID control output for steering
                turretdecl = 0.0
                turretbearing = 0.0

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
# To run: python ControlPID.py [tank_number]
# Example: python ControlPID.py 1
# TIP: You can open two terminals and run python ControlPID.py 1 in one, and python ControlPID.py 2 in the other.
if __name__ == '__main__':
    
    if len(sys.argv) < 2:
        print("Usage: python ControlPID.py [tank_number]")
        sys.exit(1)
    
    controller = ControlPID(sys.argv[1])
    controller.run()
