

import socket
import json
import threading
import time

def map(value, in_min, in_max, out_min, out_max):
    # Perform linear interpolation
    output = (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min

    if output > out_max:
        return out_max
    if output < out_min:
        return out_min

    return output

class UDP:
    PORT = 8850
    MAX_SIZE = 1024

    def __init__(self, ip):
        self.ip = ip
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)

        self.sock.settimeout(0.0)
        # self.sock.connect((ip, self.PORT))

        self.outgoing_addres = (ip, self.PORT)

        self.outgoing = None
        self.incoming = None

        self.thread_running = False
        self.thread_paused = False

    def send_message(self, message):
        self.sock.sendto(json.dumps(message).encode(), self.outgoing_addres)

    def get_most_recent(self):
        data = None
        try:
            while True:
                data, address = self.sock.recvfrom(self.MAX_SIZE)
        except socket.error as e:
            # print("socket error", e)
            pass

        if data is None:
            return None

        return json.loads( data.decode(), strict=False)


    def communicate(self):
        while self.outgoing == None:
            pass

        while self.thread_running:
            try:
                time.sleep(0.05)
                if self.thread_paused:
                    continue

                self.send_message(self.outgoing)
                incoming = self.get_most_recent()
                if incoming != None:
                    self.incoming = incoming
            except Exception as e:
                print(e)

    def set(self, message):
        self.outgoing = message

    def get(self):
        return self.incoming


class Robot:
    def __init__(self, ip="172.27.18.1", password="secret"):
        """
        Creates a new robot object
        args:
            - ip: The ip address of the robot as a string
        """

        self.udp = UDP(ip)
        self.msg = {"p": password,
                    "s": [None, None, None, None],
                    "m": [["off"], ["off"], ["off"], ["off"]],
                    "led": [0, 255, 0, 0]}

        self.com_tread = threading.Thread(target=self.udp.communicate, daemon=True)
        self.udp.thread_running = True
        self.com_tread.start()

        self.udp.set(self.msg)

    def __del__(self):
        self.udp.thread_running = False

    def estop(self):
        """
        Emergency Stop. 
        Cuts power to all motors and servos. Puts robot in safe state
        """
        self.udp.thread_paused = True

    def start(self):
        """
        Restarts the motors and servos after and estop
        """
        self.udp.thread_paused = False

    def set_led(self, red, green, blue, blink=0):
        """
        Sets a new LED color.
        arg:
            - red: int in range 0 - 255 
            - green: int in range 0 - 255
            - blue: int in range 0 - 255
            - blink: period of blinking in milliseconds. If zero no blinking
        """

        assert 0 <= red <= 255
        assert 0 <= green <= 255
        assert 0 <= blue <= 255
        assert blink >= 0
        self.msg["led"] = [int(red),int(green),int(blue),int(blink)]
        self.udp.set(self.msg)


    def set_motor_power(self, index, power):
        """
        Drives a motor with a fixed power
        args:
            - index: int in range 1-4. Which motor to drive?
            - power: int in range -1024 to 1024. What power to drive it with?
        """
        index -= 1
        assert 0 <= index <= 3
        assert -1025 <= power <= 1025
        self.msg["m"][index] = ["pwr", int(power)]
        self.udp.set(self.msg)

    def disable_motor(self, index):
        """
        Cuts power to motor
        args:
            - index: int in range 1-4. Which motor to stop?
        """
        index -= 1
        assert 0 <= index <= 3
        self.msg["m"][index] = ["off"]
        self.udp.set(self.msg)

    def set_servo(self, index, width):
        """
        Tells a servo to go to a given pulse width
        args:
            - index: int in range 1-4. Which servo to move?
            - angle: int in range 1000 - 2000. Want angle do we want it at?
        """

        index -= 1
        assert 0 <= index <= 3
        assert 500 <= width <= 2500
        self.msg["s"][index] = int(width)
        self.udp.set(self.msg)

    def get(self):
        """
        Returns the state of the robots sensors.
        returns:
            - dictionary of sensor readings.
            Example
             {"motor_position": [0,0, 100, -100],
               "battery_voltage": 9.43}

        """
        message = self.udp.get()
        if message is None:
            return None

        out = {"motor_position": message['pos'],
               "battery_voltage": message["vbat"],
               "motor_position_done": message["done"],
               "core_temperature": message["temp"],
               }

        return out

    def debug(self):
        """
        Displays debug messages from the c code
        """
        message = self.udp.get()
        if message is None:
            return None

        return message["debug"]

    def set_motor_power_distance(self, index, power, encoder_ticks):
        """
        Used janky algorythm to drive to a given encoder position
        args:
            - index: int in range 1-4. Which motor to drive?
            - power: int in range 0 to 1024 . With what max power to drive to destination
            - encoder_ticks: what encoder position do we want to drive to?
        """
        # raise NotImplementedError
        index -= 1
        assert 0 <= index <= 3
        assert 0 <= power <= 1024
        assert type(encoder_ticks) is int
        self.msg["m"][index] = ["dst", power, encoder_ticks]
        self.udp.set(self.msg)

    def set_motor_speed(self, index, speed):
        """
        Uses PID to drive motor at fixed speed
        args:
            - index: int in range 1-4. Which motor to drive?
            - speed: int in range -3000 to 3000 . What speed to drive it at? (Arbitrary units for now)
        """
        # raise NotImplementedError
        index -=1
        assert 0 <= index <= 3
        self.msg["m"][index] = ["spd", speed]
        self.udp.set(self.msg)


    def disable_servo(self, index):
        index -= 1
        assert 0 <= index <= 3
        self.msg["s"][index] = None
        self.udp.set(self.msg)

    def set_motor_pid(self, index, kp, ki, kd):
        raise NotImplementedError
        index -= 1
        assert 0 <= index <= 3

        setting = self.get_settings_version()
        self.msg["PID"] = [index, kp, ki, kd]
        self.udp.set(self.msg)

        # wait for new settings version number
        while (self.get_settings_version() == setting):
            pass

        self.msg.pop("PID")

    def get_settings_version(self):
        raise NotImplementedError
        return self.udp.get()["set_ver"]


if __name__ == "__main__":
    import time
    a = Robot('192.168.5.60')

    # a.set_servo(0,45)

    time.sleep(1)
    print("got", a.get())
    time.sleep(1)
    print("got", a.get())
    a.set_servo(0,90)
    time.sleep(1)
    print("got", a.get())
    time.sleep(1)

