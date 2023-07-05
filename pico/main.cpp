/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// #include "pico/stdlib.h"
//
// int main() {
// #ifndef PICO_DEFAULT_LED_PIN
// #warning blink example requires a board with a regular LED
// #else
//     const uint LED_PIN = PICO_DEFAULT_LED_PIN;
//     gpio_init(LED_PIN);
//     gpio_set_dir(LED_PIN, GPIO_OUT);
//     while (true) {
//         gpio_put(LED_PIN, 1);
//         sleep_ms(250);
//         gpio_put(LED_PIN, 0);
//         sleep_ms(250);
//     }
// #endif
// }


/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "pico/multicore.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/pio.h"

#include "error.hpp"

#include "command.hpp"
#include "telemetry.hpp"
#include "networking.hpp"

#include "encoder.hpp"
#include "led.hpp"
#include "motor.hpp"

#include "wifi_pass.h"
#include "pins.h"



void core1_entry() {
        printf("hello from core 1\n");
}



Networking main_data;

RBGLed rgb_led;

MotorHardware motor_1;
MotorHardware motor_2;
MotorHardware motor_3;
// MotorHardware motor_4;

void init(){
    stdio_init_all();

    ASSERTM(not cyw43_arch_init(), "failed to initialise\n");
    cyw43_arch_enable_sta_mode();

    printf("Connecting to %s Wi-Fi...\n", WIFI_SSID);
    ASSERTM(not cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000), "failed to connect to wifi\n");
    printf("Connected.\n");
}



int main() {
    init();

    main_data.init();

    rgb_led.init(LED_RED, LED_GREEN, LED_BLUE);

    motor_1.init(MOTOR_1A, MOTOR_1B, 0, ENCODER_1A, ENCODER_1B);
    motor_2.init(MOTOR_2A, MOTOR_2B, 0, ENCODER_2A, ENCODER_2B);
    motor_3.init(MOTOR_3A, MOTOR_3B, 0, ENCODER_3A, ENCODER_3B);
    // motor_4.init(MOTOR_4A, MOTOR_4B, 0, ENCODER_4A, ENCODER_4B);

    // multicore_launch_core1(core1_entry);

    adc_init();
    adc_gpio_init(26);
    adc_set_temp_sensor_enabled(true);
    adc_select_input(0);


    while (1) {
        if( absolute_time_diff_us(main_data.active_command->recv_time, get_absolute_time()) > 500000) {
            printf("Stale command! ESTOP\n");
            main_data.active_command->estop();
        }
        printf("encoder count: %d\n", motor_1.encoder.get_count());

        const float conversion_factor = 3.3f / (1 << 12);

        adc_select_input(0);
        main_data.telemetry.v_bat = adc_read() * conversion_factor;

        adc_select_input(4);
        float ADC_voltage = adc_read() * conversion_factor;
        main_data.telemetry.temp = 27 - (ADC_voltage - 0.706)/0.001721;

        //random test copy
        main_data.telemetry.encoders_position[0] = main_data.active_command->servos[0].angle;

        main_data.send_udp();
        sleep_ms(1000);
    }

}










