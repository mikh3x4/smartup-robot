
from robot import Robot, map
from gamepad import Gamepad

from time import sleep


g = Gamepad()

# r = Robot('192.168.5.50', password="secret")
r = Robot('192.168.1.10', password="secret")

while 1:
    sleep(0.05)

    commands = g.get()

    if commands is None:
        r.estop()
        continue
    
    r.start()

    # print(commands)

    if commands["r1"]:
        front = commands["ry"]
        side = commands["rx"]
    else:
        front = 0
        side = 0

    right = map(-front + side, -1, 1, -1023, 1023)
    left = map(-front - side, -1, 1, -1023, 1023)

    print(left, right)

    r.set_motor_power(2, right)
    r.set_motor_power(3, left)

    r.set_led(255,0,0, 100)

    try:
        print(r.udp.get())
        print(r.udp.get()["debug"])
    except:
        pass

