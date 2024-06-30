# Embeded code on pico w

Look at the targets in the Makefile

--

To setup infras. Rerun after making changes in CMakeLists.txt
`make clean && make cmake_setup`  

--

To put code onto board. Check wifi_pass.h to make sure you're setting the correct board name/serial number  
`make build && make upload`

--

To view serial output (ctrl+t and then q to quit)  
`make monitor`

--


tested with pico-sdk commit f396d05f8252d4670d4ea05c8b7ac938ef0cd381

to install the sdk rough steps:

```

cd ~
git clone git@github.com:raspberrypi/pico-sdk.git
cd pico-sdk
git checkout f396d05f8252d4670d4ea05c8b7ac938ef0cd381
git submodule update --init
echo 'export PICO_SDK_PATH=/home/smartup/pico-sdk' >> ~/.bashrc

```
