
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


Networking main_data;

RBGLed rgb_led;

MotorHardware<Encoder> motor_1;
MotorHardware<Encoder> motor_2;
MotorHardware<Encoder> motor_3;
MotorHardware<Encoder> motor_4;

ServosHardware servos;


void init(){
    stdio_init_all();

    ASSERTM(not cyw43_arch_init(), "failed to initialise\n");

    if(1){
        cyw43_arch_enable_ap_mode("Smartup 1", "testing123", CYW43_AUTH_WPA2_AES_PSK);
        printf("created WIFI");
    }else{//used in the past with a centeral access point
        cyw43_arch_enable_sta_mode();

        printf("Connecting to %s Wi-Fi...\n", WIFI_SSID);
        ASSERTM(not cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000), "failed to connect to wifi\n");
        printf("Connected.\n");
    }
}

void core1_entry() {
    printf("hello from core 1\n");
    while(1)
    {
        motor_1.dynamics();
        motor_2.dynamics();
        motor_3.dynamics();
        motor_4.dynamics();
        sleep_ms(10);
    }
}


bool overtemp = 0;

int main() {
    init();

    sleep_ms(500);
    main_data.config_wifi();
    main_data.init();

    rgb_led.init(LED_RED, LED_GREEN, LED_BLUE);

    ADC.init();
    //ADC needs to be initialised before motors

    //Motor order flipped to match silk. Pin/Net labels match schematic
    motor_4.init(MOTOR_1A, MOTOR_1B, ENCODER_1A, ENCODER_1B);
    motor_3.init(MOTOR_2A, MOTOR_2B, ENCODER_2A, ENCODER_2B);
    motor_2.init(MOTOR_3A, MOTOR_3B, ENCODER_3A, ENCODER_3B);
    motor_1.init(MOTOR_4A, MOTOR_4B, ENCODER_4A, ENCODER_4B);
    enable_PWM();
    motor_1.calibrate();
    motor_2.calibrate();
    motor_3.calibrate();
    motor_4.calibrate();

    servos.init();

    multicore_launch_core1(core1_entry);


    while (0) {//testing loop
            // motor_4.drive_power(1023);
            rgb_led.set_color(0, 255, 0, 0);
            servos.set_power(0,1);
            servos.set_pulse_width(0, 1500);
            printf("ON\n");
            sleep_ms(3000);

            // motor_4.drive_power(0);
            servos.set_power(0,0);
            rgb_led.set_color(0, 0, 255, 0);
            printf("OFF\n");
            sleep_ms(3000);
    }

    while (1) {
        main_data.telemetry.clear_debug();
        if( absolute_time_diff_us(main_data.active_command->recv_time, get_absolute_time()) > 900000) {
            printf("Stale command! ESTOP\n");
            DEBUG_PRINT("Stale Cmd! ESTOP\n");
            main_data.active_command->estop();

        }
            printf("ADC %f, %f\n", ADC.get_vbat(), ADC.get_core_temp());
            printf("Computer IP: %s\n", ip4addr_ntoa( &main_data.active_command->telemetry_address ));

        // if( (main_data.telemetry.temp > 40.0) or overtemp){
        //     overtemp = 1;
        //
        //     printf("Temp %f, %f\n", main_data.telemetry.temp, ADC.get_core_temp());
        //     cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        //     main_data.active_command->estop();
        //
        //     main_data.active_command->led.red = 0;
        //     main_data.active_command->led.green = 0;
        //     main_data.active_command->led.blue = 0;
        //     main_data.active_command->led.blink = 0;
        // }

        main_data.telemetry.v_bat = ADC.get_vbat();
        main_data.telemetry.temp = ADC.get_core_temp();

        main_data.telemetry.encoders_position[0] = motor_1.encoder.get_count();
        main_data.telemetry.encoders_position[1] = motor_2.encoder.get_count();
        main_data.telemetry.encoders_position[2] = motor_3.encoder.get_count();
        main_data.telemetry.encoders_position[3] = motor_4.encoder.get_count();

        motor_1.exec_command(main_data.active_command->motors[0]);
        motor_2.exec_command(main_data.active_command->motors[1]);
        motor_3.exec_command(main_data.active_command->motors[2]);
        motor_4.exec_command(main_data.active_command->motors[3]);

        // SWAPPED ORDER OF SERVOS TO MATCH BOARD
        servos.exec_command(0, main_data.active_command->servos[3]);
        servos.exec_command(1, main_data.active_command->servos[2]);
        servos.exec_command(2, main_data.active_command->servos[1]);
        servos.exec_command(3, main_data.active_command->servos[0]);

        rgb_led.set_color(main_data.active_command->led.red,
                          main_data.active_command->led.green,
                          main_data.active_command->led.blue,
                          main_data.active_command->led.blink);

        main_data.send_udp();
        sleep_ms(10);

    }
}










