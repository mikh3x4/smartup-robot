
echo 'SUBSYSTEM=="usb", ATTRS{idVendor}=="0810", MODE="0666", GROUP="plugdev"' | sudo tee /etc/udev/rules/99-my-device.rules
pip install hidapi
pip install -e .

