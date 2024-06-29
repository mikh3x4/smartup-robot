"""
Przklad kontrolowania robota na obozie Adamed Smartup

Link do dokumentacji jest na: https://bit.ly/smartup-robot

UWAGA: Nie modyfikujcie tego pilku, on moze byc nadpisany przez nowe wersje example.py
Zrobcie swoj plik do modyfikacji!
"""

from robot import Robot, map
from gamepad import Gamepad
from time import sleep

# UWAGA: Sprawdzcie czy gamepad jest w trybie ANALOG
gamepad = Gamepad()
robot = Robot()

while 1:
    sleep(0.05)

    try:
        commands = gamepad.get()
    except ConnectionError:
        robot.estop()
        continue

    print(commands)

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

    right = map(-front + side, -1, 1, -1023, 1023)
    left = map(-front - side, -1, 1, -1023, 1023)

    print(left, right)

    robot.set_motor_power(2, right)
    robot.set_motor_power(3, left)

    robot.set_led(0,255,0, 0)

    print(robot.get())


