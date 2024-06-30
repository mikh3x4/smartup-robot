

# put into .bashrc
# export EDITOR=nano
# alias backup='pushd ~/Desktop/SmartUp-code && git add * && git commit && popd'
# alias update='git -C /home/smartup/smartup-robot pull && cp /home/smartup/smartup-robot/controller/example.py /home/smartup/Desktop/SmartUp-code/example.py'
# export PATH=~/py_venv/bin:$PATH


echo 'SUBSYSTEM=="usb", ATTRS{idVendor}=="0810", MODE="0666", GROUP="plugdev"' | sudo tee /etc/udev/rules.d/99-gamepad.rules

pushd ~
python3 -m venv py_venv --system-site-packages
popd

~/py_venv/bin/pip3 install hidapi
~/py_venv/bin/pip3 install -e .

