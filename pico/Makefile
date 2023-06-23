

.PHONY: upload build setup_cmake serial

project_name := my_proj

upload: build/$(project_name).uf2
	picotool load -f build/$(project_name).uf2

serial:
	tio /dev/$$(ls /dev | grep tty.usbmodem)

build:
	cd build; make

cmake_setup:
	mkdir build; cd build; cmake ..

