from foscam.foscam import camera_factory
import time
import cloudsight
import pyttsx
camera = camera_factory()

snap = camera.snapshot()
path = "img/snap%d.jpg" % time.time()
with open(path, "w") as outfile:
	outfile.write(snap)

auth = cloudsight.SimpleAuth('0XG_2yQHzZgMHyjVVVskNQ')
api = cloudsight.API(auth)
with open(path, 'rb') as f:
    response = api.image_request(f, path, {
        'image_request[locale]': 'en-US',
        })
status = api.wait(response['token'], timeout=30)
name = str(status["name"])
print name
engine = pyttsx.init()
engine.setProperty('rate', 140)
engine.say(name)
engine.runAndWait()
