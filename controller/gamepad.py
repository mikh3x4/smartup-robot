

import hid
import time

class Gamepad():
    def __init__(self):
        """
        Looks for gamepads and returns a gamepad object if its finds one
        """
        # vendor_id= 1356
        # product_id= 2508

        # vendor_id= 2064
        # product_id= 1

        self.gamepads = {
                (2064, 1): parse_esperanza_report,
                (1356, 2508): parse_ps4_report,
                }

        self.parse = None

        looking = True
        while looking:
            print("Looking for gamepad")
            for device in hid.enumerate():
                if((device["vendor_id"], device["product_id"]) in self.gamepads.keys()):
                    self.parse = self.gamepads[ (device["vendor_id"], device["product_id"]) ] 
                    vendor_id = device["vendor_id"]
                    product_id = device["product_id"]
                    looking = False
                    break

                # if device["vendor_id"] == vendor_id and device["product_id"] == product_id:
                #     looking = False
                #     break
            else:
                time.sleep(0.2)

        self.g = hid.device()
        self.g.open(vendor_id, product_id)
        self.g.set_nonblocking(True)

        print("Found gamepad")

        self.report = None
        self.last_update = time.monotonic()
    
    def get(self):
        """
        Returns a dictionary of possible buttons on the gamepad
        If the gamepad disconnects or 300ms have passed without an last_update
        if raises a ConnectionError
        """
        try:
            while 1:
                report = self.g.read(64)
                if not report:
                    break
                else:
                    self.report = report
                    self.last_update = time.monotonic()
        except Exception as e:
            print(e)
            raise ConnectionError

        if self.report is None:
            raise ConnectionError

        if self.last_update + 0.3 < time.monotonic():
            raise ConnectionError

        return self.parse(self.report)


    def send_commands(self):
        report = bytearray(79)
        report[0] = 0x05  # R eport ID for setting rumble and light bar
        report[1] = 0xFF  # Timeout value (0xFF means no timeout)
        report[2] = 0x00  # BLANK
        report[3] = 0x00  # BLANK
        report[4] = 0x00  # Small Rumble
        report[5] = 0x00  # Rumble
        report[6] = 0x00  # RED
        report[6] = 0x00  # GREEN
        report[6] = 0x00  # BLUE

        self.g.write(report)


def parse_ps4_report(report):
    # normalize a byte to -1 to 1 scale
    def normalize_stick(byte):
        return (byte - 127.5) / 127.5

    # normalize a byte to 0 to 1 scale
    def normalize_trigger(byte):
        return byte / 255.0

    # look at     # https://github.com/chrippa/ds4drv/blob/master/ds4drv/device.py

    dpad_states = ["up", "up_right", "right", "down_right", "down", "down_left", "left", "up_left", "none"]

    out = {
        "dpad_up": dpad_states[report[5] & 0xF] == "up" or dpad_states[report[5] & 0xF] == "up_right" or dpad_states[report[5] & 0xF] == "up_left",
        "dpad_down": dpad_states[report[5] & 0xF] == "down" or dpad_states[report[5] & 0xF] == "down_right" or dpad_states[report[5] & 0xF] == "down_left",
        "dpad_left": dpad_states[report[5] & 0xF] == "left" or dpad_states[report[5] & 0xF] == "up_left" or dpad_states[report[5] & 0xF] == "down_left",
        "dpad_right": dpad_states[report[5] & 0xF] == "right" or dpad_states[report[5] & 0xF] == "up_right" or dpad_states[report[5] & 0xF] == "down_right",
        "square": bool(report[5] & (1 << 4)),
        "cross": bool(report[5] & (1 << 5)),
        "circle": bool(report[5] & (1 << 6)),
        "triangle": bool(report[5] & (1 << 7)),
        "l1": bool(report[6] & 1),
        "r1": bool(report[6] & (1 << 1)),
        "l2": bool(report[6] & (1 << 2)),
        "r2": bool(report[6] & (1 << 3)),
        "share": bool(report[6] & (1 << 4)),
        "options": bool(report[6] & (1 << 5)),
        "l3": bool(report[6] & (1 << 6)),
        "r3": bool(report[6] & (1 << 7)),
        "ps": bool(report[7] & 1),
        "touchpad": bool(report[7] & (1 << 1)),
        "lx": normalize_stick(report[1]),
        "ly": -normalize_stick(report[2]),
        "rx": normalize_stick(report[3]),
        "ry": -normalize_stick(report[4]),
        "l2_trigger": normalize_trigger(report[8]),
        "r2_trigger": normalize_trigger(report[9])
        }

    if len(report) < 64:
        return out

    # parse 16-bit signed little-endian values for acceleration and rotation
    gyro_x = int.from_bytes(report[13:15], byteorder='little', signed=True) / 16.4# in g's
    gyro_y = int.from_bytes(report[15:17], byteorder='little', signed=True) / 16.4# in g's
    gyro_z = int.from_bytes(report[17:19], byteorder='little', signed=True) / 16.4# in g's
    accel_x = int.from_bytes(report[19:21], byteorder='little', signed=True) / 8192.0  # in deg/s
    accel_y = int.from_bytes(report[21:23], byteorder='little', signed=True) / 8192.0  # in deg/s
    accel_z = int.from_bytes(report[23:25], byteorder='little', signed=True) / 8192.0  # in deg/s

    # get touchpad finger position data
    touchpad_finger1_event = report[35] & 0x80 == 0  # touch event start (finger 1 down)
    touchpad_finger1_id = report[35] & 0x7F  # unique identifier for the first finger touch event
    touchpad_finger1_x = report[36] | ((report[37] & 0x0F) << 8)  # x position of finger 1 touch event
    #fixed from     # https://github.com/chrippa/ds4drv/blob/master/ds4drv/device.py
    touchpad_finger1_y = (report[38] << 4) | ((report[37] & 0xf0) >> 4)  # y position of finger 1 touch event

    out.update({"accel_x": accel_x,
        "accel_y": accel_y,
        "accel_z": accel_z,
        "gyro_x": gyro_x,
        "gyro_y": gyro_y,
        "gyro_z": gyro_z,
        "touchpad_finger1_event": touchpad_finger1_event,
        "touchpad_finger1_id": touchpad_finger1_id,
        "touchpad_finger1_x": touchpad_finger1_x,
        "touchpad_finger1_y": touchpad_finger1_y,
    })

    return out




def parse_esperanza_report(report):
    # normalize a byte to -1 to 1 scale

    def normalize_stick(byte):
        return (byte - 127.5) / 127.5

    # normalize a byte to 0 to 1 scale
    def normalize_trigger(byte):
        return byte / 255.0

    # look at     # https://github.com/chrippa/ds4drv/blob/master/ds4drv/device.py

    # dpad_states = ["up", "up_right", "right", "down_right", "down", "down_left", "left", "up_left", "none"]

    out = {
        # "dpad_up": dpad_states[report[5] & 0xF] == "up" or dpad_states[report[5] & 0xF] == "up_right" or dpad_states[report[5] & 0xF] == "up_left",
        # "dpad_down": dpad_states[report[5] & 0xF] == "down" or dpad_states[report[5] & 0xF] == "down_right" or dpad_states[report[5] & 0xF] == "down_left",
        # "dpad_left": dpad_states[report[5] & 0xF] == "left" or dpad_states[report[5] & 0xF] == "up_left" or dpad_states[report[5] & 0xF] == "down_left",
        # "dpad_right": dpad_states[report[5] & 0xF] == "right" or dpad_states[report[5] & 0xF] == "up_right" or dpad_states[report[5] & 0xF] == "down_right",

        "button_1": bool(report[5] & (31 ^ 15)),
        "button_2": bool(report[5] & (47 ^ 15)),
        "button_3": bool(report[5] & (79 ^ 15)),
        "button_4": bool(report[5] & (143 ^ 15)),

        "l1": bool(report[6] & 1),
        "r1": bool(report[6] & 2),
        "l2": bool(report[6] & 4),
        "r2": bool(report[6] & 8),
        "select": bool(report[6] & 16),
        "start": bool(report[6] & 32),

        "lx": normalize_stick(report[3]),
        "ly": -normalize_stick(report[4]),
        "rx": normalize_stick(report[1]),
        "ry": -normalize_stick(report[2]),
        }

    return out
