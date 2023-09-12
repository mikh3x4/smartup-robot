


import sys
import glob
import serial

# Configure your trigger string
trigger_str = "Rec"
estop = "ES"

def find_usbmodem_ports():
    """Returns a list of tty.usbmodem* ports."""
    return glob.glob('/dev/tty.usbmodem*')

def main():
    ports = find_usbmodem_ports()
    if len(ports) != 1:
        print(f'Error: Expected 1 serial port, but found {len(ports)}.')
        sys.exit(1)

    port = ports[0]
    with serial.Serial(port, baudrate=115200, timeout=1) as ser:
        buffer = []
        while True:
            line = ser.readline().decode('utf-8')  # read a '\n' terminated line

            # print(line)
            # continue
            buffer.append(line)
            if trigger_str in line or estop in line:
                # When trigger string is found, print out all the buffered lines
                print("".join(buffer))
                # Clear console for better visualization of stable lines
                print("\033c", end="")
                # Clear the buffer for the next round
                buffer.clear()

if __name__ == "__main__":
    main()
