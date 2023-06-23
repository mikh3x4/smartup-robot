# print("hello")

# from machine import Pin
# led = Pin("LED", Pin.OUT)
#
# led.toggle()
# led.toggle()
#
SSID = "Hamiltopia"
PASS = "1001dalmatians"

import network
import time

wlan = network.WLAN(network.STA_IF)
wlan.active(True)
wlan.connect(SSID, PASS)
wlan.isconnected()
wlan.ifconfig()

#
# import urequests
# astronauts = urequests.get("http://api.open-notify.org/astros.json").json()
# astronauts

import socket
import time

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.settimeout(0.0)
sock.bind(("", 8850))

#the pico's udp recv buffer is one message long??

address = None

message_count = 0
while True:
    while True:
        try:
            last_data, address = sock.recvfrom(1024)
            print("got data")
            message_count += 1
        except OSError as e:
            print(e)
            break
    print("here")
    if address != None:
        sock.connect(address)
        sock.send(b"reply_"+str(message_count).encode())
        print("sent")
    time.sleep(0.1)



