#!/usr/bin/env python

import binascii
import bitstring
import serial
import struct
import threading


class Create2:
    """Python class for interfacing with a locally-connected iRobot Create 2 robot"""

    def __init__(self, dev = "/dev/ttyUSB0", baud=115200):
        """Open the connection with the device"""
        self.dev = dev
        self.baud = baud
        self.ser = serial.Serial(dev, baud, writeTimeout=1)
        self._power_color = 0 # 0 = green, 255 = red
        self._power_intensity = 0
        self.led_bits = bitstring.BitArray(8) # All leds off by default
        self.sensors = self.Sensors()
        self.start()
        
    def start(self):
        """Starts the connection with the robot.  Must be called before any other command, and after a """
        """call to reset() or stop().  Puts the robot into Passive mode."""
        try:
            self.ser.write("\x80")
        except:
            raise IOError("Cannot communicate with the robot.  Please press the power button and try again.")
        
    def reset(self):
        """Reset the robot, as if the battery had been pulled.  Must call start() to resume communications"""
        self.ser.write("\x07")
    
    def stop(self):
        """Stop listening to commands.  Does not stop a cleaning cycle in progress!"""
        self.ser.write("\xad")

    def safe(self):
        """Put the robot into Safe mode.  Allows full control over the robot, but with safety sensors engaged"""
        self.ser.write("\x83")

    def full(self):
        """Put the robot into Full mode.  Allows full control, and does not respond to safety sensors.  Use with caution!"""
        self.ser.write("\x84")

    def clean(self):
        """Start a Roomba cleaning cycle.  The robot will attempt to cover the current room uniformly.  Sets mode to Passive."""
        self.ser.write("\x87")

    def max(self):
        """Start a Roomba Max cleaning cycle.  Will cruise until the battery is dead, attempting to cover the current room uniformly.  Sets mode to Passive."""
        self.ser.write("\x88")

    def spot(self):
        """Start a Roomba sport cleaning cycle.  Sets mode to Passive."""
        self.ser.write("\x86")

    def power(self):
        """Stops the robot.  Sets mode to Off. """
        """Following iRobot's naming convention, but this should really be named stop!"""
        self.ser.write("\x85")

    # Scheduling commands not currently implemented
    
    
    def drive(self, velocity, radius):
        """Drive in a circle with a specified radius, at a specified velocity"""
        """Positive radius = turning left (while moving forward).  Negative radius = turning right"""
        """Must be in Safe or Full mode"""
        if velocity < -500 or velocity > 500:
            raise ValueError("velocity must be between -500 and 500")
        if (radius < -2000 or radius > 2000) and radius != 0x7fff:
            raise ValueError("radius must be between -2000 and 2000")
        cmd = struct.pack(">Bhh", 0x89, velocity, radius)
        self.ser.write(cmd)

    def straight(self, velocity):
        """Drive in a straight line at a given velocity"""
        """Must be in Safe or Full mode"""
        self.drive(velocity, 0x7fff)

    def clockwise(self, velocity):
        """Turn in place clockwise at a given velocity"""
        """Must be in Safe or Full mode"""
        self.drive(velocity, -1)

    def counterclockwise(self, velocity):
        """Turn in place counterclockwise at a given velocity"""
        """Must be in Safe or Full mode"""
        self.drive(velocity, 1)

    def drive_direct(self, right, left):
        """Drive by directly controlling the speed of the left and right wheels"""
        """Must be in Safe or Full mode"""
        if left < -500 or left > 500 or right < -500 or right > 500:
            raise ValueError("Wheel velocities must be between -500 and 500")
        cmd = struct.pack(">Bhh", 0x145, right, left)
        self.ser.write(cmd)

    # Drive PWM seems no more useful than Drive Direct, so it is unimplemented
    
    def _update_leds(self):
        """Update the state of the robot's LEDs.  Mostly used internally"""
        """Must be in Safe or Full mode"""
        cmd = struct.pack(">BBBB", 0x8b, self.led_bits.int, self._power_color, self._power_intensity)
        self.ser.write(cmd)

    def power_color(self, color):
        """Set the color of the power led. 0 = green, 255 = red, intermediate values range through yellow and orange"""
        """Must be in Safe or Full mode"""
        if color < 0 or color > 255:
            raise ValueError("Color must be between 0 and 255")
        self._power_color = color
        self._update_leds()

    def power_intensity(self, intensity):
        """Set the intensity of the power led. 0 = off, 255 = full brightness"""
        """Must be in Safe or Full mode"""
        if intensity < 0 or intensity > 255:
            raise ValueError("Intensity must be between 0 and 255")
        self._power_intensity = intensity
        self._update_leds()

    def debris_led(self, power):
        """Set the debris LED on or off"""
        """Must be in Safe or Full mode"""
        self.led_bits[7] = power # Bit order is reversed from the docs, thanks to bitstring
        self._update_leds()

    def spot_led(self, power):
        """Set the spot LED on or off"""
        """Must be in Safe or Full mode"""
        self.led_bits[6] = power # Bit order is reversed from the docs, thanks to bitstring
        self._update_leds()

    def dock_led(self, power):
        """Set the dock LED on or off"""
        """Must be in Safe or Full mode"""
        self.led_bits[5] = power # Bit order is reversed from the docs, thanks to bitstring
        self._update_leds()

    def check_led(self, power):
        """Set the check robot LED on or off"""
        """Must be in Safe or Full mode"""
        self.led_bits[4] = power # Bit order is reversed from the docs, thanks to bitstring
        self._update_leds()

    # Scheduling and digit LEDs currently unimplemented

    # Song API currently unimplemented

    def sensor(self, id_):
        """Request a single sensor packet from the robot. See the iRobot documentation for the definition of the sensor packets and groups"""
        if not id_ in PACKET_LENGTHS:
            raise ValueError("Unknown packet: %d" % id_)
        cmd = struct.pack(">BB", 0x8e, id_)
        self.ser.write(cmd)
        # Read back packet data
        data = self.ser.read(PACKET_LENGTHS[id_])
        if id_ in COMPOSITE_PACKETS:
            for subid in COMPOSITE_PACKETS[id_]:
                self.sensors._parse_sensor_packet(subid, data)
                data = data[PACKET_LENGTHS[subid]:]
        else:
            self.sensors._parse_sensor_packet(id_, data)
        return self.sensors

    def query_list(self, ids):
        """Query a list of sensor ids from the robot"""
        for id_ in ids:
            if not id_ in PACKET_LENGTHS:
                raise ValueError("Unknown packet: %d" % id_)
        cmd = struct.pack(">BB%dB" % len(ids), 0x95, len(ids), *ids)
        self.ser.write(cmd)
        for id_ in ids:
            data = self.ser.read(PACKET_LENGTHS[id_])
            if id_ in COMPOSITE_PACKETS:
                for subid in COMPOSITE_PACKETS[id_]:
                    self.sensors._parse_sensor_packet(subid, data)
                    data = data[PACKET_LENGTHS[subid]:]
            else:
                self.sensors._parse_sensor_packet(id_, data)
        return self.sensors
            

    def sensor_stream(self, ids, callback = None):
        """Request a continuous stream of sensor readings, updating self.sensors.  If supplied, a callback is called with self.sensors every time a new reading is received."""
        for id_ in ids:
            if not id_ in PACKET_LENGTHS:
                raise ValueError("Unknown packet: %d" % id_)
        self._sensor_stream_thread = threading.Thread(target = self._parse_sensor_stream, args=(callback,))
        self._sensor_stream_thread.daemon = True
        self._sensor_stream_thread.start()
        cmd = struct.pack(">BB%dB" % len(ids), 0x94, len(ids), *ids)
        self.ser.write(cmd)


    def _parse_sensor_stream(self, callback):
        """Thread to read from the serial port and update the sensor object, calling the callback when a new sensor packet is called (if supplied)"""
        self.ser.flushInput()
        while True:
            header = ord(self.ser.read(1))
            while (header != 19):
                # We're misaligned
                print "Sensor data misaligned, skipping a byte: %d" % header
                header = ord(self.ser.read(1))
            nbytes = ord(self.ser.read(1))
            data = self.ser.read(nbytes)
            while len(data) > 0:
                id_ = ord(data[0])                
                data = data[1:]
                self.sensors._parse_sensor_packet(id_, data)
                data = data[PACKET_LENGTHS[id_]:]
            checksum = self.ser.read(1)
            if callback != None:
                callback(self.sensors)

    class Sensors:
        """Class holding parsed data collected from the robot's sensors"""
        
        def __init__(self):
            """All data starts off unknown: None"""
            # Packet 7
            self.wheel_drop_left = None
            self.wheel_drop_right = None
            self.bump_left = None
            self.bump_right = None
            
            # Packet 8
            self.wall = None

            # Packet 9
            self.cliff_left = None
            
            # Packet 10
            self.cliff_front_left = None

            # Packet 11
            self.cliff_front_right = None
            
            # Packet 12
            self.cliff_right = None

            # Packet 13
            self.virtual_wall = None

            # Packet 14
            self.left_wheel_overcurrent = None
            self.right_wheel_overcurrent = None
            self.main_brush_overcurrent = None
            self.side_brush_overcurrent = None
            
            # Packet 15
            self.dirt_detect = None

            # Packet 16 is unused

            # Packet 17
            self.infrared_character_omni = None

            # Packet 52
            self.infrared_character_left = None

            # Packet 53
            self.infrared_character_right = None

            # Packet 18
            self.clock_button = None
            self.schedule_button = None
            self.day_button = None
            self.hour_button = None
            self.minute_button = None
            self.dock_button = None
            self.spot_button = None
            self.clean_button = None

            # Packet 19
            # Distance travelled in mm since distance last requested
            self.distance = None
            
            # Packet 20
            # Angle turned since angle last requested
            self.angle = None
            
            # Packet 21
            self.charging_state = None

            # Packet 22
            self.battery_voltage = None
            
            # Packet 23
            self.battery_current = None
            
            # Packet 24
            self.battery_temperature = None
            
            # Packet 25
            self.battery_charge = None
            
            # Packet 26
            self.battery_capacity = None
            
            # Packet 27
            self.wall_signal = None
            
            # Packet 28
            self.cliff_left_signal = None
            
            # Packet 29
            self.cliff_front_left_signal = None
            
            # Packet 30
            self.cliff_front_right_signal = None
            
            # Packet 31
            self.cliff_right_signal = None
            
            # Packet 32 - 33 are unused

            # Packet 34
            self.home_base_avilable = None
            self.internal_charger_available = None

            # Packet 35
            self.oi_mode = None

            # Packet 36
            self.song_number = None
            
            # Packet 37
            self.song_playing = None
            
            # Packet 38
            self.stream_packets = None
            
            # Packet 39
            self.requested_velocity = None

            # Packet 40
            self.requested_radius = None

            # Packet 41
            self.requested_right_velocity = None
            
            # Packet 42
            self.requested_left_velocity = None

            # Packet 43
            self.left_encoder_count = None

            # Packet 44
            self.right_encoder_count = None

            # Packet 45
            self.light_bumper_right = None
            self.light_bumper_front_right = None
            self.light_bumper_center_right = None
            self.light_bumper_center_left = None
            self.light_bumper_front_left = None
            self.light_bumper_left = None
            
            # Packet 46
            self.light_bumper_left_signal = None
            
            # Packet 47
            self.light_bumper_front_left_signal = None
            
            # Packet 48
            self.light_bumper_center_left_signal = None
            
            # Packet 49
            self.light_bumper_center_right_signal = None
            
            # Packet 50
            self.light_bumper_front_right_signal = None

            # Packet 51
            self.light_bumper_right_signal = None

            # Packet 54
            self.left_motor_current = None

            # Packet 55
            self.right_motor_current = None
            
            # Packet 56
            self.main_brush_motor_current = None

            # Packet 57
            self.side_brush_motor_current = None
            
            # Packet 58
            self.stasis = None
            
        @staticmethod
        def _parse_ubyte(packet):
            return struct.unpack("B", packet[0])[0]

        @staticmethod
        def _parse_byte(packet):
            return struct.unpack("b", packet[0])[0]
        
        @staticmethod
        def _parse_ushort(packet):
            return struct.unpack(">H", packet[0:2])[0]
        
        @staticmethod
        def _parse_short(packet):
            return struct.unpack(">h", packet[0:2])[0]

        def _parse_sensor_packet(self, id_, packet):
            """Parse a signle sensor packet sent by the robot"""
            """Returns the number of bytes consumed"""
            if id_ == 7:
                data = self._parse_ubyte(packet)
                self.wheel_drop_left = ((data & 0x08) != 0)
                self.wheel_drop_right = ((data & 0x04) != 0)
                self.bump_left = ((data & 0x02) != 0)
                self.bump_right = ((data & 0x01) != 0)
            elif id_ == 8:
                data = self._parse_ubyte(packet)
                self.wall = (data == 1)
            elif id_ == 9:
                data = self._parse_ubyte(packet)
                self.cliff_left = (data == 1)
            elif id_ == 10:
                data = self._parse_ubyte(packet)
                self.cliff_front_left = (data == 1)
            elif id_ == 11:
                data = self._parse_ubyte(packet)
                self.cliff_front_right = (data == 1)
            elif id_ == 12:
                data = self._parse_ubyte(packet)
                self.cliff_right = (data == 1)
            elif id_ == 13:
                data = self._parse_ubyte(packet)
                self.virtual_wall = (data == 1)
            elif id_ == 14:
                data = self._parse_ubyte(packet)
                self.left_wheel_overcurrent = ((data & 0x10) != 0)
                self.right_wheel_overcurrent = ((data & 0x08) != 0)
                self.main_brush_overcurrent = ((data & 0x04) != 0)
                self.side_brush_overcurrent = ((data & 0x01) != 0)
            elif id_ == 15:
                self.dirt_detect = self._parse_ubyte(packet)
            elif id_ == 16:
                pass
                # Unused value sent anyway
            elif id_ == 17:
                self.infrared_character_omni = self._parse_ubyte(packet)
            elif id_ == 52:
                self.infrared_character_left = self._parse_ubyte(packet)
            elif id_ == 53:
                self.infrared_charcter_right = self._parse_ubyte(packet)
            elif id_ == 18:
                data = self._parse_ubyte(packet)
                self.clock_button = ((data & 0x80) != 0)
                self.schedule_button = ((data & 0x40) != 0)
                self.day_button = ((data & 0x20) != 0)
                self.hour_button = ((data & 0x10) != 0)
                self.minute_button = ((data & 0x08) != 0)
                self.dock_button = ((data & 0x04) != 0)
                self.spot_button = ((data & 0x02) != 0)
                self.clean_button = ((data & 0x01) != 0)
            elif id_ == 19:
                self.distance = self._parse_short(packet)
            elif id_ == 20:
                self.angle = self._parse_short(packet)
            elif id_ == 21:
                data = self._parse_ubyte(packet)
                self.charging_state = ("Not charging",
                                       "Reconditioning charging",
                                       "Full charging",
                                       "Trickle charging",
                                       "Waiting",
                                       "Charging fault condition")[data]
            elif id_ == 22:
                self.battery_voltage = self._parse_ushort(packet)
            elif id_ == 23:
                self.battery_current = self._parse_short(packet)
            elif id_ == 24:
                self.battery_temperature = self._parse_byte(packet)
            elif id_ == 25:
                self.battery_charge = self._parse_ushort(packet)
            elif id_ == 26:
                self.battery_capacity = self._parse_ushort(packet)
            elif id_ == 27:
                self.wall_signal = self._parse_ushort(packet)
            elif id_ == 28:
                self.cliff_left_signal = self._parse_ushort(packet)
            elif id_ == 29:
                self.cliff_front_left_signal = self._parse_ushort(packet)
            elif id_ == 30:
                self.cliff_front_right_signal = self._parse_ushort(packet)
            elif id_ == 31:
                self.cliff_right_signal = self._parse_ushort(packet)
            elif id_ == 32 or id_ == 33:
                pass
            elif id_ == 34:
                data = self._parse_ubyte(packet)
                self.home_base_available = ((data & 0x02) != 0)
                self.internal_charger_available = ((data & 0x01) != 0)
            elif id_ == 35:
                data = self._parse_ubyte(packet)
                self.oi_mode = ("Off", "Passive", "Safe", "Full")[data]
            elif id_ == 36:
                self.song_number = self._parse_ubyte(packet)
            elif id_ == 37:
                self.song_playing = (self._parse_ubyte(packet) != 0)
            elif id_ == 38:
                self.stream_packets = self._parse_ubyte(packet)
            elif id_ == 39:
                self.requested_velocity = self._parse_short(packet)
            elif id_ == 40:
                self.requested_radius = self._parse_short(packet)
            elif id_ == 41:
                self.requested_right_velocity = self._parse_short(packet)
                return 3
            elif id_ == 42:
                self.requested_left_velocity = self._parse_short(packet)
            elif id_ == 43:
                self.left_encoder_count = self._parse_ushort(packet)
            elif id_ == 44:
                self.right_encoder_count = self._parse_ushort(packet)
            elif id_ == 45:
                data = self._parse_ubyte(packet)
                self.light_bumper_right = ((data & 0x20) != 0)
                self.light_bumper_front_right = ((data & 0x10) != 0)
                self.light_bumper_center_right = ((data & 0x08) != 0)
                self.light_bumper_center_left = ((data & 0x04) != 0)
                self.light_bumper_front_left = ((data & 0x02) != 0)
                self.light_bumper_left = ((data & 0x01) != 0)
            elif id_ == 46:
                self.light_bumper_left_signal = self._parse_ushort(packet)
            elif id_ == 47:
                self.light_bumper_front_left_signal = self._parse_ushort(packet)
            elif id_ == 48:
                self.light_bumper_center_left_signal = self._parse_ushort(packet)
            elif id_ == 49:
                self.light_bumper_center_right_signal = self._parse_ushort(packet)
            elif id_ == 50:
                self.light_bumper_front_right_signal = self._parse_ushort(packet)
            elif id_ == 51:
                self.light_bumper_right_signal = self._parse_ushort(packet)
            elif id_ == 54:
                self.left_motor_current =  self._parse_short(packet)
            elif id_ == 55:
                self.right_motor_current = self._parse_short(packet)
            elif id_ == 56:
                self.main_brush_motor_current = self._parse_short(packet)
            elif id_ == 57:
                self.side_brush_motor_current = self._parse_short(packet)
            elif id_ == 58:
                self.stasis = (self._parse_ubyte(packet) != 0)


# Length of sensor packets
PACKET_LENGTHS = {0: 26,
                  1: 10,
                  2: 6,
                  3: 10,
                  4: 14,
                  5: 12,
                  6: 52,
                  7: 1,
                  8: 1,
                  9: 1,
                  10: 1,
                  11: 1,
                  12: 1,
                  13: 1,
                  14: 1,
                  15: 1,
                  16: 1,
                  17: 1,
                  52: 1,
                  53: 1,
                  18: 1,
                  19: 2,
                  20: 2,
                  21: 1,
                  22: 2,
                  23: 2,
                  24: 1,
                  25: 2,
                  26: 2,
                  27: 2,
                  28: 2,
                  29: 2,
                  30: 2,
                  31: 2,
                  32: 2,
                  33: 1,
                  34: 1,
                  35: 1,
                  36: 1,
                  37: 1,
                  38: 1,
                  39: 2,
                  40: 2,
                  41: 2,
                  42: 2,
                  43: 2,
                  44: 2,
                  45: 1,
                  46: 2,
                  47: 2,
                  48: 2,
                  49: 2,
                  50: 2,
                  51: 2,
                  54: 2,
                  55: 2,
                  56: 2,
                  57: 2,
                  58: 1,
                  100: 80,
                  101: 28,
                  106: 12,
                  107: 9}

COMPOSITE_PACKETS  = {0: range(7,27),
                      1: range(7, 17),
                      2: range(17, 21),
                      3: range(21, 27),
                      4: range(27, 35),
                      5: range(35, 43),
                      6: range(7,43),
                      100: range(7, 59),
                      101: range(43, 59),
                      106: range(46, 52),
                      107: range(54, 59)}
