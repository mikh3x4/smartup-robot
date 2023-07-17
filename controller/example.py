
from robot import Robot, map
from gamepad import Gamepad
from time import sleep

gamepad = Gamepad()
# robot = Robot('192.168.1.10')
robot = Robot('172.20.10.13')

while 1:
    sleep(0.05)

    try:
        commands = gamepad.get()
    except ConnectionError:
        robot.estop()
        continue

    # print(commands)

    robot.start()

    if commands["r1"]:
        front = commands["ry"]
        side = commands["rx"]
    else:
        front = 0
        side = 0

    if commands["button_1"]:
        robot.set_servo(1, 1500)
    else:
        robot.set_servo(1, 1800)

    right = map(-front + side, -1, 1, -1023, 1023)
    left = map(-front - side, -1, 1, -1023, 1023)

    print(left, right)

    robot.set_motor_power(2, right)
    robot.set_motor_power(3, left)

    robot.set_led(0,255,0, 0)

    print(robot.get())
    print(robot.debug())


