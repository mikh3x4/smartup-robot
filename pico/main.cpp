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

#include "lwip/pbuf.h"
#include "lwip/udp.h"

#define UDP_PORT 8850
#define BEACON_MSG_LEN_MAX 512
#define BEACON_TARGET "192.168.4.247"
#define BEACON_INTERVAL_MS 1000


#include "jsnm.h"
#include "command.hpp"
#include "error.hpp"
#include "parser.hpp"
#include "encoder.hpp"

#include "wifi_pass.h"



void core1_entry() {
        printf("hello from core 1\n");
}




#define DEBUG_STRING_LEN 128

class Sensors {
    public:
        long encoders_position[4] = {0, 1, 10, 100};
        long encoders_speed[4] = {0, 5, 50, -500};
        float v_bat = 3.14;
        float temp = 20.0; //probably not needed but added as test
        bool motor_done[4] = {0, 0, 0, 1};
        long settings_version = 0;
        char debug[DEBUG_STRING_LEN] = "testing";

    size_t encode_json(char *js, size_t len){
        int writen = snprintf(js, len,
                 "{ \"vbat\": %.2f, "
                 "\"pos\": [%ld, %ld, %ld, %ld], "
                 "\"spd\": [%ld, %ld, %ld, %ld], "
                 "\"done\": [%d, %d, %d, %d], "
                 "\"debug\": \"%s\", "
                 "\"temp\": %.2f, "
                 "\"set_ver\": %ld }",
                 v_bat,
                 encoders_position[0], encoders_position[1], encoders_position[2], encoders_position[3],
                 encoders_speed[0], encoders_speed[1], encoders_speed[2], encoders_speed[3],
                 motor_done[0], motor_done[1], motor_done[2], motor_done[3],
                 debug,
                 temp,
                 settings_version);

        ASSERT( writen > 0);
        ASSERT( writen < len);
        return writen;
    }
};


enum ROBOT_STATE {RUNNING, ESTOP, LOW_BATTERY, INIT, MULIPLE_CONTROLLERS};


void udp_recv_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);

class MainData {
public:
    enum ROBOT_STATE robot_data = INIT;
    struct pbuf *current_incomming;

    ParseJSON json_parser;

    // Double buffer the command so one is writen to while the other is used
    Command command_1;
    Command command_2;

    // then they are swapped
    // THIS IS VERY THREAD SAFTLY SKETCH
    // IT RELIES ON POINTER WRITES BEING ATTOMIC ON THE RP2040
    Command *active_command;
    Command *scratch_command;

    Sensors sensors;

    struct udp_pcb* udp_pcb;

    void init_udp_receiver() {
        struct udp_pcb* pcb = udp_new();
        ASSERT(pcb != NULL);

        if (pcb == NULL) {
            printf("Failed to create new UDP PCB.\n");
            return;
        }

        err_t er = udp_bind(pcb, IP_ADDR_ANY, UDP_PORT);
        ASSERT(er == ERR_OK);

        udp_recv(pcb, udp_recv_callback, this);
    }

    void init(){
        this->udp_pcb = udp_new();
        this->active_command = &this->command_1;
        this->scratch_command = &this->command_2;

        ipaddr_aton(BEACON_TARGET, &(this->active_command->telemetry_address));
        this->active_command->telemetry_port = 8851;

        init_udp_receiver();
    }

    void send_udp() {
        struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, BEACON_MSG_LEN_MAX+1, PBUF_RAM);
        char *buffer = (char *)p->payload;

        size_t real_size = this->sensors.encode_json(buffer, BEACON_MSG_LEN_MAX);

        pbuf_realloc(p, real_size);

        err_t er = udp_sendto(this->udp_pcb, p, &(active_command->telemetry_address), active_command->telemetry_port);

        printf("Sent packet: %.*s\n", p->len, buffer);
        printf("sent to %d\n", active_command->telemetry_port);

        pbuf_free(p);
        ASSERT(er == ERR_OK);
    }

};


void udp_recv_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    if (p == NULL) return;

    MainData *main_data_ptr = (MainData*)arg;

    memcpy(&(main_data_ptr->scratch_command->telemetry_address), addr, sizeof(ip_addr_t));
    main_data_ptr->scratch_command->telemetry_port = port;
    main_data_ptr->scratch_command->recv_time = get_absolute_time();

    char *rec = (char *)p->payload;
    bool worked = main_data_ptr->json_parser.parse_message(rec, p->len, main_data_ptr->scratch_command);

    printf("Received packet: %.*s\n", p->len ,rec);
    pbuf_free(p);

    if (worked){
        printf("Parsed ok\n");

        Command *temp = main_data_ptr->active_command;
        main_data_ptr->active_command = main_data_ptr->scratch_command; // This line needs to be atomic
        main_data_ptr->scratch_command = temp;
    }
}


MainData main_data;

Encoder encoder0;


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

    encoder0.init(0, 2);

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
        printf("encoder count: %d\n", encoder0.get_count());

        const float conversion_factor = 3.3f / (1 << 12);

        adc_select_input(0);
        main_data.sensors.v_bat = adc_read() * conversion_factor;

        adc_select_input(4);
        float ADC_voltage = adc_read() * conversion_factor;
        main_data.sensors.temp = 27 - (ADC_voltage - 0.706)/0.001721;

        //random test copy
        main_data.sensors.encoders_position[0] = main_data.active_command->servos[0].angle;

        main_data.send_udp();
        sleep_ms(1000);
    }

}










