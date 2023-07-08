
cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

set(PICO_BOARD pico_w)

project(my_project)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project


# add_executable(hello_world
#     hello_world.c
# )
#
# # Add pico_stdlib library which aggregates commonly used features
# target_link_libraries(hello_world pico_stdlib)
#
# # create map/bin/hex/uf2 file in addition to ELF.
# pico_add_extra_outputs(hello_world)

####################

add_executable(my_proj
        main.cpp
        networking.cpp
        )

# pull in common dependencies
# target_link_libraries(my_proj pico_stdlib)

# enable usb output, disable uart output
pico_enable_stdio_usb(my_proj 1)
pico_enable_stdio_uart(my_proj 0)

pico_generate_pio_header(my_proj ${CMAKE_CURRENT_LIST_DIR}/encoder.pio)
pico_generate_pio_header(my_proj ${CMAKE_CURRENT_LIST_DIR}/led_pwm.pio)

# set(CMAKE_BUILD_TYPE Debug)
target_compile_definitions(my_proj PRIVATE PARAM_ASSERTIONS_ENABLE_ALL=1)

target_include_directories(my_proj PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        )

target_link_libraries(my_proj
        pico_cyw43_arch_lwip_threadsafe_background
        pico_stdlib
        pico_multicore
        hardware_adc
        hardware_pio
        hardware_pwm
        hardware_spi
        hardware_dma
        )

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(my_proj)


