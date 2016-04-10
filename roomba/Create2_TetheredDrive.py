#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 28 February 2015

###########################################################################
# Copyright (c) 2015 iRobot Corporation
# http://www.irobot.com/
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#   Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
#   Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in
#   the documentation and/or other materials provided with the
#   distribution.
#
#   Neither the name of iRobot Corporation nor the names
#   of its contributors may be used to endorse or promote products
#   derived from this software without specific prior written
#   permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
###########################################################################

from Tkinter import *
import tkMessageBox
import tkSimpleDialog

import struct

try:
    import serial
except ImportError:
    tkMessageBox.showerror('Import error', 'Please install pyserial.')
    raise

connection = None

TEXTWIDTH = 40 # window width, in characters
TEXTHEIGHT = 16 # window height, in lines

VELOCITYCHANGE = 200
ROTATIONCHANGE = 300

helpText = """\
Supported Keys:
P\tPassive
S\tSafe
F\tFull
C\tClean
D\tDock
R\tReset
Space\tBeep
Arrows\tMotion

If nothing happens after you connect, try pressing 'P' and then 'S' to get into safe mode.
"""

# sendCommandASCII takes a string of whitespace-separated, ASCII-encoded base 10 values to send
def sendCommandASCII(command):
    cmd = ""
    for v in command.split():
        cmd += chr(int(v))

    sendCommandRaw(cmd)

# sendCommandRaw takes a string interpreted as a byte array
def sendCommandRaw(command):
    global connection

    try:
        if connection is not None:
            connection.write(command)
        else:
            tkMessageBox.showerror('Not connected!', 'Not connected to a robot!')
            print "Not connected."
    except serial.SerialException:
        print "Lost connection"
        tkMessageBox.showinfo('Uh-oh', "Lost connection to the robot!")
        connection = None

    print ' '.join([ str(ord(c)) for c in command ])
    text.insert(END, ' '.join([ str(ord(c)) for c in command ]))
    text.insert(END, '\n')
    text.see(END)
    
    
# A handler for keyboard events. Feel free to add more!
def callbackKey(event):
    k = event.keysym.upper()
    motionChange = False

    if event.type == '2': # KeyPress; need to figure out how to get constant
        if k == 'P':   # Passive
            sendCommandASCII('128')
        elif k == 'S': # Safe
            sendCommandASCII('131')
        elif k == 'F': # Full
            sendCommandASCII('132')
        elif k == 'C': # Clean
            sendCommandASCII('135')
        elif k == 'D': # Dock
            sendCommandASCII('143')
        elif k == 'SPACE': # Beep
            sendCommandASCII('140 3 1 64 16 141 3')
        elif k == 'R': # Reset
            sendCommandASCII('7')
        elif k == 'UP':
            callbackKey.up = True
            motionChange = True
        elif k == 'DOWN':
            callbackKey.down = True
            motionChange = True
        elif k == 'LEFT':
            callbackKey.left = True
            motionChange = True
        elif k == 'RIGHT':
            callbackKey.right = True
            motionChange = True
        else:
            print repr(k), "not handled"
    elif event.type == '3': # KeyRelease; need to figure out how to get constant
        if k == 'UP':
            callbackKey.up = False
            motionChange = True
        elif k == 'DOWN':
            callbackKey.down = False
            motionChange = True
        elif k == 'LEFT':
            callbackKey.left = False
            motionChange = True
        elif k == 'RIGHT':
            callbackKey.right = False
            motionChange = True

    if motionChange == True:
        velocity = 0
        velocity += VELOCITYCHANGE if callbackKey.up is True else 0
        velocity -= VELOCITYCHANGE if callbackKey.down is True else 0
        rotation = 0
        rotation += ROTATIONCHANGE if callbackKey.left is True else 0
        rotation -= ROTATIONCHANGE if callbackKey.right is True else 0

        # compute left and right wheel velocities
        vr = velocity + (rotation/2)
        vl = velocity - (rotation/2)

        # create drive command
        cmd = struct.pack(">Bhh", 145, vr, vl)
        if cmd != callbackKey.lastDriveCommand:
            sendCommandRaw(cmd)
            callbackKey.lastDriveCommand = cmd
# static variables for keyboard callback -- I know, this is icky
callbackKey.up = False
callbackKey.down = False
callbackKey.left = False
callbackKey.right = False
callbackKey.lastDriveCommand = ''

def onConnect():
    global connection
    
    port = tkSimpleDialog.askstring('Port?', 'Enter COM port to open.')
    print "Trying " + port + "... "
    try:
        connection = serial.Serial(port, baudrate=115200, timeout=1)
        print "Connected!"
        tkMessageBox.showinfo('Connected', "Connection succeeded!")
    except serial.SerialException:
        print "Failed."
        tkMessageBox.showinfo('Failed', "Sorry, couldn't connect to " + port)


def onHelp():
    tkMessageBox.showinfo('Help', helpText)

def onQuit():
    if tkMessageBox.askyesno('Really?', 'Are you sure you want to quit?'):
        root.destroy()

# set up the GUI
root = Tk()

menu = Menu(root)
menu.add_command(label="Connect", command=onConnect)
menu.add_command(label="Help", command=onHelp)
menu.add_command(label="Quit", command=onQuit)
root.config(menu=menu)

text = Text(root, height = TEXTHEIGHT, width = TEXTWIDTH, wrap = WORD)
scroll = Scrollbar(root, command=text.yview)
text.configure(yscrollcommand=scroll.set)
text.pack(side=LEFT, fill=BOTH, expand=True)
scroll.pack(side=RIGHT, fill=Y)

text.insert(END, helpText)

#frame = Frame(root, width=WIDTH, height=HEIGHT)
#frame.pack()

root.bind("<Key>", callbackKey)
root.bind("<KeyRelease>", callbackKey)

root.mainloop()
