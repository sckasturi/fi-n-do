#!/usr/bin/env python

import create2
import time

r = create2.Create2()

count = 0

def callback(sensors):
    global count 
    count += 1
    if sensors.bump_left or sensors.bump_right:
        print "Pardon me!"


r.start()
r.clean()
r.sensor_stream((7,), callback)

while True:
    time.sleep(6000)

    
