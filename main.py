from foscam.foscam import camera_factory
from roomba.create2 import Create2
from snap_and_name import take_picture 
import time
import threading

r = Create2()
def clean():
    r.start()
    r.clean()

def stop():
    r.safe()
    r.stop()

t1 = threading.Thread(target=clean)
t2 = threading.Thread(target=take_picture)
t3 = threading.Thread(target=stop)

t1.start()
time.sleep(20)
t2.start()
t3.start()
# Main program
