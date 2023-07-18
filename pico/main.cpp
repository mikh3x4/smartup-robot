
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

MotorHardware<InteruptEncoder> motor_1; //Waiting for interupt encoders, probably can wait
MotorHardware<Encoder> motor_2;
MotorHardware<Encoder> motor_3;
MotorHardware<Encoder> motor_4;

ServosHardware servos;


void init(){
    stdio_init_all();

    ASSERTM(not cyw43_arch_init(), "failed to initialise\n");
    cyw43_arch_enable_sta_mode();

    printf("Connecting to %s Wi-Fi...\n", WIFI_SSID);
    ASSERTM(not cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000), "failed to connect to wifi\n");
    printf("Connected.\n");
}

void core1_entry() {
    printf("hello from core 1\n");
    motor_1.encoder.init_core_sepecific();
    while(1)
    {
        motor_1.dynamics();
        motor_2.dynamics();
        motor_3.dynamics();
        motor_4.dynamics();
        //distance debug
        printf("status: %d %d %d\n",motor_2.target_distance,motor_2.last_count,motor_2.LP_rate);
        //PID debug
        //printf("status: %d %d %d %d\n",motor_2.wanted_rate,motor_2.LP_rate,motor_2.integral_state,motor_2.last_count);
        sleep_ms(10);
    }
}


int main() {
    init();

    main_data.init();

    rgb_led.init(LED_RED, LED_GREEN, LED_BLUE);

    ADC.init();
    //ADC needs to be initialised before motors
    motor_1.init(MOTOR_1A, MOTOR_1B, ENCODER_1A, ENCODER_1B);
    motor_2.init(MOTOR_2A, MOTOR_2B, ENCODER_2A, ENCODER_2B);
    motor_3.init(MOTOR_3A, MOTOR_3B, ENCODER_3A, ENCODER_3B);
    motor_4.init(MOTOR_4A, MOTOR_4B, ENCODER_4A, ENCODER_4B);
    enable_PWM();

    servos.init();

    multicore_launch_core1(core1_entry);

    // motor_2.driving_mode=MOTOR_MODE::DISTANCE;
    // uint8_t i;
    // while(1)
    // {
    //     i=(i+1)%4;
    //     switch(i)
    //     {
    //     case 0:
    //         motor_2.drive_distance(20000,1023);
    //     break;
    //     case 1:
    //         motor_2.drive_distance(-20000,1023);
    //     break;
    //     case 2:
    //         motor_2.drive_distance(20000,255);
    //     break;
    //     case 3:
    //         motor_2.drive_distance(-20000,255);
    //     break;
    //     }
    //     motor_1.drive_power(300);
    //     motor_3.drive_power(300);
    //     motor_4.drive_power(300);
    //     //printf("encoder status: %d %d %d %d\n",motor_1.encoder.get_count(),motor_2.encoder.get_count(),motor_3.encoder.get_count(),motor_4.encoder.get_count());
    //     sleep_ms(10000);
    // }

    while (1) {
        main_data.telemetry.clear_debug();
        if( absolute_time_diff_us(main_data.active_command->recv_time, get_absolute_time()) > 500000) {
            printf("Stale command! ESTOP\n");
            DEBUG_PRINT("Stale Cmd! ESTOP\n");
            main_data.active_command->estop();
        }

        main_data.telemetry.v_bat = ADC.get_vbat();
        // main_data.telemetry.temp = adc.get_core_temp();

        main_data.telemetry.encoders_position[0] = motor_1.encoder.get_count();
        main_data.telemetry.encoders_position[1] = motor_2.encoder.get_count();
        main_data.telemetry.encoders_position[2] = motor_3.encoder.get_count();
        main_data.telemetry.encoders_position[3] = motor_4.encoder.get_count();

        // DEBUG_PRINT("test print\n");
        // DEBUG_PRINT("hello %f\n", adc.get_vbat());

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










