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

def loop():
    sleep(0.05)

    try:
        commands = gamepad.get()
    except ConnectionError:
        robot.estop()
        return

    robot.start()
    print(robot.get())

    if commands["r1"]:
        robot.set_motor_power(2, 1024)
        robot.set_servo(1, 1500)
    else:
        robot.set_motor_power(2, 0)
        robot.set_servo(1, 1800)

    robot.set_led(0,255,0, 0)

while 1:
    try:
        while 1:
            loop()
    finally:
        robot.estop()
