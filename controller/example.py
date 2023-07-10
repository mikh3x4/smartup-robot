
from robot import Robot
from ps4 import Gamepad

from time import sleep


def map(value, in_min, in_max, out_min, out_max):
    # Perform linear interpolation
    return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min


g = Gamepad()

r = Robot('192.168.5.61', password="secret")
while 1:
    sleep(0.05)

    commands = g.get()

    if commands is None:
        r.estop()
        continue
    
    r.start()

    # print(commands)

    right = map(commands["ry"], -1, 1, -1023, 1023)
    left = map(commands["ly"], -1, 1, -1023, 1023)

    print(left, right)

    r.set_motor_power(1, right)
    r.set_motor_power(2, -left)

    r.set_led( map(commands["ry"], -1, 1, 0, 255), map(commands["ly"], -1, 1, 0, 255), 0)

    try:
        print(r.udp.get()["debug"])
    except:
        pass

