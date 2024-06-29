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
