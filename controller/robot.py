

import socket
import json

class UDP:
    PORT = 8850
    MAX_SIZE = 1024
    def __init__(self, ip):
        self.ip = ip
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.settimeout(0.0)
        self.sock.connect((ip, self.PORT))

        self.last_data = None
        self.last_address = None

    def send(self, message):
        asset self.last_address != None

        self.sock.connect(self.last_address)
        self.sock.send(message)

    def get(self):
        try:
            while True:
                self.last_data, self.last_address = self.sock.recvfrom(self.MAX_SIZE)
        except socket.error:
            pass

        return self.last_data

class Robot:

    def __init__(self, ip, pass):
        self.udp = UDP(ip)
        self.msg = {"s1": 0, "pass": pass}

    def read_voltage(self):
        pass


