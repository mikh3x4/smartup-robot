
# export EDITOR=nano
# alias backup='git add * && git commit'
# alias update='git -C /home/smartup/smartup-robot pull & cp /home/smartup/smartup-robot/controller/example.py /home/smartup/Desktop/SmartUp-code/example.py'
# export PATH=~/apps/thonny/bin:$PATH

echo 'SUBSYSTEM=="usb", ATTRS{idVendor}=="0810", MODE="0666", GROUP="plugdev"' | sudo tee /etc/udev/rules/99-my-device.rules
pip3 install hidapi
pip3 install -e .

