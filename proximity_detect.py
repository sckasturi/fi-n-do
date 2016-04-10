import RPi.GPIO as GPIO
import time


trig_pin = 2
echo_pin = 3

print "Distance Measurement In Progress"

def setup():
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(trig_pin,GPIO.OUT)
    GPIO.setup(echo_pin,GPIO.IN)
    GPIO.output(trig_pin, False)
    print "Waiting For Sensor To Settle"
    time.sleep(2)
    print "Let's start measuring distances!"

def measure():
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(trig_pin,GPIO.OUT)
    GPIO.setup(echo_pin,GPIO.IN)
    GPIO.output(trig_pin, False)
    GPIO.output(trig_pin, True)
    time.sleep(0.00001)
    GPIO.output(trig_pin, False)
    pulse_start = time.time()
    while GPIO.input(echo_pin) == 0:
        if time.time() - pulse_start >  3:
            return "No distance recorded."
    #print time.time() - pulse_start
    pulse_end = time.time()
    pulse_duration = pulse_end - pulse_start
    distance = round(pulse_duration*17150, 2)
    return distance

for i in range(100):
    print measure()
#    time.sleep(5)

GPIO.cleanup()
