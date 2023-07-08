
from robot import Robot
from ps4 import PS4Controller as Gamepad

from time import sleep


def map(value, in_min, in_max, out_min, out_max):
    # Perform linear interpolation
    return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min


g = Gamepad()

r = Robot('192.168.5.60', password="secret")

while 1:
    sleep(0.05)
    commands = g.get()
    # print(commands)

    right = map(commands["ry"], -1, 1, -255, 255)
    left = map(commands["ly"], -1, 1, -255, 255)

    print(left, right)

    # r.set_motor_power(0, right)
    # r.set_motor_power(1, -left)



    r.set_led( map(commands["ry"], -1, 1, 0, 255), map(commands["ly"], -1, 1, 0, 255), 0)

    try:
        print(r.udp.get()["debug"])
    except:
        pass

