from foscam.foscam import camera_factory
from roomba.create2 import Create2
from snap_and_name import take_picture 
from proximity_detect import measure, setup
import time
import threading

r = Create2()
results = None
#results = [None] * 2
def clean():
    r.start()
    r.clean()

def stop():
    r.safe()
    r.stop()

t1 = threading.Thread(target=clean)
t2 = threading.Thread(target=take_picture, args=(results,)+args, kwargs)
t3 = threading.Thread(target=stop)

#setup()

t1.start() #detect 
while True:
    d = measure()
    if d < 100:
        distance = d
        break 
#time.sleep(20)
t2.start()
t3.start()
t2.join()
print distance
print results
   
    # Main program
