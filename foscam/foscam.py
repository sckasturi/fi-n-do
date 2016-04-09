#!/usr/bin/env python

import requests

class HDFoscam:
    """Class for requesting snapshots from a HD Foscam camera"""
    def __init__(self, ip="10.99.99.50", port=88, user="user", password="password"):
        self.ip = ip
        self.port = port
        self.user = user
        self.password = password

    def snapshot(self):
        url = "http://%s:%s/cgi-bin/CGIProxy.fcgi" % (self.ip, self.port)
        params = {'cmd': 'snapPicture2', 'usr': self.user, 'pwd': self.password}
        r = requests.get(url, params=params)
        return r.content

class MJPEGFoscam:
    """Class for requesting snapshots from a MJPEG (VGA) Foscam camera"""
    def __init__(self, ip="10.99.99.50", port=80, user="admin", password=""):
        self.ip = ip
        self.port = port
        self.user = user
        self.password = password

    def snapshot(self):
        url = "http://%s:%s/snapshot.cgi" % (self.ip, self.port)
        params = {'user': self.user, 'pwd': self.password }
        r = requests.get(url, params=params)
        return r.content

def camera_factory(ip="10.99.99.50", port=None, user=None, password=None):
    """Attempt to auto-detect whether a given camera is an HD or MJPEG camera, and return the correct object"""    
    # Guess that this is a MJPEG camera, and request a snapshot
    mjpeg_port = port or 80
    mjpeg_user = user or "admin"
    mjpeg_pass = password or ""
    mjpeg_url = "http://%s:%s/snapshot.cgi" % (ip, mjpeg_port)
    mjpeg_params = {'user': mjpeg_user, 'pwd': mjpeg_pass}
    try:        
        r = requests.get(mjpeg_url, params=mjpeg_params, timeout=1)
        if r.status_code == 200:
            return MJPEGFoscam(ip, mjpeg_port, mjpeg_user, mjpeg_pass)
    except:
        pass
    # That didn't work.  Try as HD
    hd_port = port or 88
    hd_user = user or "user"
    hd_pass = password or "password"
    hd_url = "http://%s:%s/cgi-bin/CGIProxy.fcgi" % (ip, hd_port)
    hd_params = {'cmd': 'snapPicture2', 'usr': hd_user, 'pwd': hd_pass}
    try:
        r = requests.get(hd_url, params=hd_params, timeout=1)
        if r.status_code == 200:
            return HDFoscam(ip, hd_port, hd_user, hd_pass)
    except:
        pass
    raise Exception("Could not determine camera type")
