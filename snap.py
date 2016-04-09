from foscam.foscam import camera_factory
import time

camera = camera_factory()
snap = camera.snapshot()
path = "snaps/snap%d.jpg" % time.time()
with open(path, "w") as outfile:
	outfile.write(snap)

status = api.wait(response['token'], timeout=30)
print str(status["name"])
