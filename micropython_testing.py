def test():
    print("hello")

    from machine import Pin
    import machine
    # led = Pin("LED", Pin.OUT)
    #
    # led.toggle()
    # led.toggle()
    #
    SSID = "Hamiltopia"
    PASS = "1001dalmatians"

    import network
    import time
    import socket

    import ujson as json

    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    wlan.connect(SSID, PASS)
    wlan.isconnected()
    wlan.ifconfig()

    # import urequests
    # astronauts = urequests.get("http://api.open-notify.org/astros.json").json()
    # astronauts

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    sock.settimeout(0.0)
    sock.bind(("0.0.0.0", 8850))

    #the pico's udp recv buffer is one message long??

    adc = machine.ADC(26)

    def return_json(a = [0]):
        a[0] += 1
        return {"imu": "test_data", "count": a, "vbat": adc.read_u16() }

    

    address = None
    message_count = 0
    while True:
        while True:
            try:
                last_data, address = sock.recvfrom(1024)
                message = json.loads(last_data.decode())
                print("got data", message)
                message_count += 1
            except OSError as e:
                # print(e)
                break
        print("here")
        if address != None:
            # sock.connect(address)
            sock.sendto( json.dumps(return_json()).encode() ,address)
            print("sent")
        time.sleep(0.1)



