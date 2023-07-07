
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
#include "servos.hpp"
#include "v_bat.hpp"

#include "wifi_pass.h"
#include "pins.h"



void core1_entry() {
        printf("hello from core 1\n");
}



Networking main_data;

RBGLed rgb_led;

// MotorHardware motor_1;
MotorHardware motor_2;
MotorHardware motor_3;
MotorHardware motor_4;

ServosHardware servos;

ADC adc;

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

    adc.init();

    //motor_1.init(MOTOR_1A, MOTOR_1B, ENCODER_1A, ENCODER_1B);
    motor_2.init(MOTOR_2A, MOTOR_2B, ENCODER_2A, ENCODER_2B);
    motor_3.init(MOTOR_3A, MOTOR_3B, ENCODER_3A, ENCODER_3B);
    motor_4.init(MOTOR_4A, MOTOR_4B, ENCODER_4A, ENCODER_4B);
    enable_PWM();


    servos.init();

    uint8_t counter=0;
    while(1)
    {
        for(int i=0;i<4;i++)
        {
            servos.set_power(i,true);
            if(i<3)
                servos.set_angle(i,1000+3*counter);
            else
                servos.set_angle(i,2000-3*counter);
        }
        int8_t tmp=counter;
        motor_3.drive_power(tmp*7);
        motor_4.drive_power(-tmp*7);
        rgb_led.set_color(counter<<7,counter<<9,0xFFFF-(counter<<7));
        sleep_ms(20);
        counter++;
    }

    // multicore_launch_core1(core1_entry);

    // while (1) {
    //     if( absolute_time_diff_us(main_data.active_command->recv_time, get_absolute_time()) > 500000) {
    //         printf("Stale command! ESTOP\n");
    //         main_data.active_command->estop();
    //     }

    //     main_data.telemetry.v_bat = adc.get_vbat();
    //     main_data.telemetry.temp = adc.get_core_temp();

    //     main_data.telemetry.encoders_position[0] = motor_1.encoder.get_count();
    //     main_data.telemetry.encoders_position[1] = motor_2.encoder.get_count();
    //     main_data.telemetry.encoders_position[2] = motor_3.encoder.get_count();


        motor_1.exec_command(main_data.active_command->motors[0]);
        motor_2.exec_command(main_data.active_command->motors[1]);

        //random test copy
        main_data.telemetry.encoders_position[3] = main_data.active_command->servos[0].angle;

    //     main_data.send_udp();

    //     sleep_ms(100);

        rgb_led.set_color(main_data.active_command->led.red,
                          main_data.active_command->led.green,
                          main_data.active_command->led.blue,
                          main_data.active_command->led.blink);

    // }
}










