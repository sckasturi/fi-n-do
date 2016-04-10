from collections import deque
import cv2
import cv2.cv as cv
import numpy as np 
import argparse
import imutils

#command line arguments
ap = argparse.ArgumentParser()
ap.add_argument("-v", "--video",
	help="path to the (optional) video file")
ap.add_argument("-b", "--buffer", type=int, default=32,
	help="max buffer size")
args = vars(ap.parse_args())

#Initialize some values
points = deque(maxlen=args["buffer"])
framecounter = 0
(dX, dY) = (0, 0)
#direction = ""


#Takes in webcam video
cap = cv2.VideoCapture(-1)

