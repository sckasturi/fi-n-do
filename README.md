## Inspiration

Blind people cannot identify what is in front of them.

## What it does

It is a robot that moves in front of a blind person and tells them to stop when it detects an object in front of them. It then snaps a picture and sends the picture to the CloudSight API, which identifies the object and sends back the name. It then says the name of the object into the headphones.

## How we built it

We built the device using a Raspberry Pi to power the whole thing. The Laboratory for Telecommunication Sciences loaned us a Roomba + Camera system that we later added another webcam to and a proximity sensor. The rear-facing webcam uses OpenCV, while the front facing camera uses CloudSight for recognition. The Roomba is powered using iRobot Create, and the whole thing is brought together by a lot of Python and heart emoticon

## Challenges we ran into

- Integration OpenCV into the main program
- Making sure that the robot stopped when it detected an object
- Slow WiFi

## Accomplishments that we're proud of

- Integrating a ton of data into one device
- Finding a good image identification API

## What we learned

- How to use OpenCV
- How to use Roomba
- How to multithread
- How to use Foscam

## What's Next?

- Faster Image Recognition
- Better Footstep Detection
