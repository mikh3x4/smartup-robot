

.PHONY: upload build setup_cmake monitor clean

project_name := my_proj

upload: build/$(project_name).uf2
	picotool load -f build/$(project_name).uf2

monitor:
	ls /dev | grep -q tty.usbmodem && tio /dev/$$(ls /dev | grep tty.usbmodem)

build:
	cd build; make

cmake_setup:
	mkdir build; cd build; cmake ..

clean:
	rm -rf ./build/

